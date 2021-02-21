#include <WinApiWrapper.h>
#include <Oleacc.h>
#include <algorithm>
#include <Util.h>
#include <set>
#include <tlhelp32.h>
#include <iostream>
#include <thread>
#include <spinner.h>
#include <chrono>
#include <filesystem>

using namespace std::chrono_literals;

std::vector<HWND> windows = {};
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);

std::vector<process> process::enumAllProcess()
{
  std::vector<process> processes;
  HANDLE hndl = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS | TH32CS_SNAPMODULE, 0);
  if (hndl)
  {
    PROCESSENTRY32 processScanned = {sizeof(PROCESSENTRY32)};
    Process32First(hndl, &processScanned);
    do
    {
      if (!processScanned.th32ProcessID)
        continue;
      if (std::wstring(processScanned.szExeFile).empty() || std::wstring(processScanned.szExeFile) == L"System" | std::wstring(processScanned.szExeFile) == L"svchost.exe")
        continue;
      processes.push_back(process(processScanned.dwSize, std::wstring(processScanned.szExeFile)));
    } while (Process32Next(hndl, &processScanned));

    CloseHandle(hndl);
  }

  return processes;
}

BOOL EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
  windows.push_back(hwnd);
  return TRUE;
}

std::wstring utf8ToUtf16(std::string utf8Str)
{
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
  return conv.from_bytes(utf8Str);
}

std::string utf16ToUtf8(std::wstring utf16Str)
{
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
  return conv.to_bytes(utf16Str);
}

DWORD waitForProgramLaunchSync(const std::wstring toWatch)
{
  spinnercpp::spinner s(200ms, 65, "", "", "  Waiting for " + utf16ToUtf8(toWatch) + " to launch");
  s.start();
  while (true)
  {
    BOOST_FOREACH (process p, process::enumAllProcess())
    {
      if (p.name == toWatch)
        return p.pid;
    }
  }
  s.stop();
}

void waitForProgramLaunchAsync(const std::wstring toWatch, std::function<void(DWORD pid)> onLaunch)
{
  std::thread([toWatch, onLaunch]() {
    DWORD pid = waitForProgramLaunchSync(toWatch);
    onLaunch(pid);
  }).detach();
}

InjectDllReturnValue InjectDll(DWORD pid, std::string Path)
{
  if (!std::filesystem::exists(Path))
    return InjectDllReturnValue::InexistantDLL;
  // Some top variables aha..
  DWORD ProcessId = pid;
  DWORD ExitCode; // Defining the exit code for the thread

  long DLL_ = Path.length() + 1;

  HANDLE ProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessId); // Getting the process handle
  if (!ProcessHandle)
  {
    return InjectDllReturnValue::AcessWindowHandleError;
  }
  LPVOID MemAllocation = VirtualAllocEx(ProcessHandle, NULL, DLL_, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE); // Getting memory allocation
  if (!MemAllocation)
  {
    return InjectDllReturnValue::MemAlocationError;
  } // If didn't find the memory allocation it will return internal error, since that's what it is lool
    /*
	  Writing to memory
	  We use virtual protect to change the protection region
	*/
  DWORD WriteMemOldProc;
  VirtualProtect(WriteProcessMemory, 1, PAGE_EXECUTE_READWRITE, &WriteMemOldProc);
  int WritingSuccessful = WriteProcessMemory(ProcessHandle, MemAllocation, utf8ToUtf16(Path.c_str()).c_str(), DLL_, 0);
  VirtualProtect(WriteProcessMemory, 1, WriteMemOldProc, &WriteMemOldProc);
  /*
	  Creating a thread to the target handle
	*/
  LPTHREAD_START_ROUTINE Kernel32_Library = (LPTHREAD_START_ROUTINE)GetProcAddress(LoadLibraryW(L"kernel32"), "LoadLibraryW");               // Staring the routine
  HANDLE DLLHandler_ = CreateRemoteThread(ProcessHandle, NULL, NULL, (LPTHREAD_START_ROUTINE)Kernel32_Library, MemAllocation, NULL, NULL); // Creating the thread
  WaitForSingleObject(DLLHandler_, INFINITE);                                                                                              // Wating for the object
  VirtualFreeEx(ProcessHandle, MemAllocation, NULL, MEM_RELEASE);                                                                          // Free the dll
  GetExitCodeThread(DLLHandler_, &ExitCode);                                                                                               // Checking the exit code

  // Checking if the injection was successful
  CloseHandle(ProcessHandle); // Closing the process handle
  if (!ExitCode)
  {
    return InjectDllReturnValue::InjectionFail;
  }
  else
  {
    return InjectDllReturnValue::InjectionSucess;
  }
}