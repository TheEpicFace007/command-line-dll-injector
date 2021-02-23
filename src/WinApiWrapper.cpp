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

DWORD waitForProgramLaunchSync(const std::wstring toWatch)
{
  spinnercpp::spinner s(200ms, 65, "", "", "  Waiting for " + utf16ToUtf8(toWatch) + " to launch");
  s.start();
  while (true)
  {
    BOOST_FOREACH (process p, process::enumAllProcess())
    {
      if (p.name == toWatch)
      {
        s.stop();
        std::cout << "Found the program!";
        return p.pid;
      }
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

void InjectDll(DWORD pid, std::string Path)
{
}