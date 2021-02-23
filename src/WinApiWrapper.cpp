#include "WinApiWrapper.h"
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
#include <BlackBone/Process/Process.h>
#include <BlackBone/Process/ProcessCore.h>
#include <BlackBone/Process/ProcessModules.h>
//#include "dbg.h"

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
      if (std::wstring(processScanned.szExeFile).empty() || std::wstring(processScanned.szExeFile) == L"System" || std::wstring(processScanned.szExeFile) == L"svchost.exe")
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

DWORD waitForProgramLaunchSync(std::wstring toWatch)
{
  spinnercpp::spinner s(200ms, 65, "", "", "  Waiting for " + utf16ToUtf8(toWatch) + " to launch");
  s.start();
  auto start = std::chrono::steady_clock::now();


  while (true)
  {
    for (DWORD p : blackbone::Process::EnumByName(toWatch))
    {
      std::wstring pExe = getProcessExeName(p);
      if (pExe == toWatch)
      {
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        s.stop();
        std::cout << std::endl << "Found " << utf16ToUtf8(toWatch) << " in " << elapsed_seconds.count() << "s" << std::endl;
        return p;
      }
    }
  }
  s.stop();
  return NULL;
}

void waitForProgramLaunchAsync(const std::wstring toWatch, std::function<void(DWORD pid)> onLaunch)
{
  std::thread([toWatch, onLaunch]() {
    DWORD pid = waitForProgramLaunchSync(toWatch);
    onLaunch(pid);
  }).detach();
}

void InjectDll(DWORD pid, std::string dll)
{
  
}

std::wstring getProcessExeName(DWORD pid)
{
  HANDLE pHandle;
  constexpr int buffer_size = 1000;
  wchar_t *fileName = new WCHAR[buffer_size];
  HANDLE h = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
  
  GetModuleFileNameExW(h, NULL, fileName, buffer_size);
  std::filesystem::directory_entry file(fileName);
  delete[] fileName;
  std::wstring f = file.path().filename().generic_wstring();
  return f;
  // return utf8ToUtf16(std::filesystem::directory_entry(fileName))
}

BOOL IsWow64(HANDLE process)
{
  BOOL bIsWow64 = FALSE;

  typedef BOOL(WINAPI * LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);
  LPFN_ISWOW64PROCESS fnIsWow64Process;
  fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle(TEXT("kernel32")), "IsWow64Process");

  if (NULL != fnIsWow64Process)
  {
    if (!fnIsWow64Process(process, &bIsWow64))
    {
      //handle error
    }
  }
  return bIsWow64;
}

bool IsX86Process(HANDLE process)
{
  SYSTEM_INFO systemInfo = {0};
  GetNativeSystemInfo(&systemInfo);

  // x86 environment
  if (systemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
    return true;

  // Check if the process is an x86 process that is running on x64 environment.
  // IsWow64 returns true if the process is an x86 process
  return IsWow64(process);
}