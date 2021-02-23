#pragma once
#include <vector>
#include <string>
#include <windows.h>
#include <filesystem>
#include <Psapi.h>
#include <Windows.h>
#include <WtsApi32.h>
#include <boost/foreach.hpp>
#include <codecvt>
#include <string>
#include <Psapi.h>
#include <functional>
#include <thread>

std::wstring getProcessExeName(DWORD pid);

std::wstring utf8ToUtf16(std::string utf8Str);

std::string utf16ToUtf8(std::wstring utf16Str);

struct process
{
	/**
	 * The exe name of the process
	 */
	std::wstring exe_name;
	DWORD pid;
	HANDLE process_handle()
	{
		HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, this->pid);
		return processHandle;
	}

	process(DWORD pid, std::wstring name)
	{
		this->pid = pid;
		this->exe_name = name;
	}

	static std::vector<process> enumAllProcess();
};

/**
 * Wait for a program to be opened syncronously
 * @param toWatch the exe name of the program to wait to launch
 * @return The PID of the program that opened
*/
DWORD waitForProgramLaunchSync(std::wstring toWatch);

/**
 * Wait for a program to be opened asyncronously
 * @param toWatch the exe name of the program to wait to launch
 * @param onLaunch an function to execute once the program find the program that launched
 * @return An thread to scheduele the program watcher
*/
void waitForProgramLaunchAsync(const std::wstring toWatch, std::function<void(DWORD pid)> onLaunch);

/**
 * Inject a DLL
 * @param pid The process to inject the dll
 * @param path The path of the dll
*/
void InjectDll(DWORD pid, std::string dll);