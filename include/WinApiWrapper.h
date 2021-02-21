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

std::wstring utf8ToUtf16(std::string utf8Str);

std::string utf16ToUtf8(std::wstring utf16Str);

struct process
{
	/**
	 * The exe name of the process
	 */
	std::wstring name;
	DWORD pid;

	process(DWORD pid, std::wstring name)
	{
		this->pid = pid;
		this->name = name;
	}

	static std::vector<process> enumAllProcess();
};

/**
 * Wait for a program to be opened syncronously
 * @param toWatch the exe name of the program to wait to launch
 * @return The PID of the program that opened
*/
DWORD waitForProgramLaunchSync(const std::wstring toWatch);

/**
 * Wait for a program to be opened asyncronously
 * @param toWatch the exe name of the program to wait to launch
 * @param onLaunch an function to execute once the program find the program that launched
 * @return An thread to scheduele the program watcher
*/
void waitForProgramLaunchAsync(const std::wstring toWatch, std::function<void(DWORD pid)> onLaunch);

enum class InjectDllReturnValue
{
	AcessWindowHandleError,
	MemAlocationError,
	InjectionFail,
	InjectionSucess,
	InexistantDLL
};

/**
 * Inject a DLL
 * @param pid The process to inject the dll
 * @param path The path of the dll
 * @return Return false if there was a error while injecting
*/
InjectDllReturnValue InjectDll(DWORD pid, std::string Path);