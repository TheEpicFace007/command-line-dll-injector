#ifdef _DEBUG
// #pragma comment(lib, "../dBlackBone.lib")
#else
#endif
#pragma comment(lib, "../BlackBone.lib")

#include <iostream>
#include <Windows.h>
#include <WinApiWrapper.h>
#include <dbg.h>
#include <boost/program_options.hpp>
#include <string>
#include <thread>
#include <chrono>
#include <BlackBone/Process/RPC/RemoteFunction.hpp>
#include <BlackBone/Process/Process.h>
#include <BlackBone/Config.h>
#include <BlackBone/Process/Process.h>
#include <BlackBone/PE/PEImage.h>
#include <BlackBone/Misc/Utils.h>
#include <filesystem>
#include <BlackBone/PE/PEImage.h>
#include <BlackBone/PE/ImageNET.h>
#define NAKED __declspec( naked )


using namespace std::chrono_literals;
namespace po = boost::program_options;

bool nt_error(NTSTATUS status);

// #define WAIT_DEBUG_ATTACH

int main(int argc, char const *argv[])
{
#ifdef WAIT_DEBUG_ATTACH
  BOOL isDebuggerPresent;
  while (!isDebuggerPresent) { CheckRemoteDebuggerPresent(GetModuleHandle(NULL), &isDebuggerPresent); };
#endif
  SetConsoleTitleW(L"Command line dll injector");
  blackbone::InitializeOnce();
  po::options_description desc("Allowed options");
  desc.add_options()
  ("help,h", "Produce help message of the program")
  ("dll,d", po::value<std::string>(), "The dll to inject")
  ("exe,e", po::value<std::string>(), "The program exe file name to inject the onto Example: mspaint.exe")
  // ("process-name,p", po::value<std::string>(), "The program process name")
  ("watch,w", "Inject the DLL as soon as the specified program is launched")
  ("delay", po::value<float>(), "Wait a certain aount of time before injecting in seconds");
  // ("unload-dll,u", "Unload the dll that is going to be injected if it's already loaded");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm, true);
  po::notify(vm);

  if (vm.count("help"))
  {
    std::cout << desc << "\n";
    return 0;
  }

  if (!vm.count("exe"))
  {
    std::cerr << "Error: " << "Invalid args." << std::endl;
    std::cerr << desc << std::endl;
    return 1;
  }

  // Set the delay
  float delay = 0;
  if (vm.count("delay"))
  {
    try
    {
      delay = vm["delay"].as<float>();
    }
    catch (...)
    {
      std::cerr << "Failed to get the delay\n";
      exit(1);
    }
  }
  // Check if the dll is present
  std::string dll_path;
  if (vm.count("dll"))
  {
    dll_path = vm["dll"].as<std::string>();
    if (!std::filesystem::exists(dll_path))
    {
      std::cerr << "Error: Dll doesn't exist\n";
      return 1;
    }
    dll_path = std::filesystem::absolute(std::filesystem::directory_entry(dll_path).path()).generic_u8string();
  }

  // Attacg the dll
if (vm.count("exe"))
  {
    DWORD pid;
    std::string exe_name = vm["exe"].as<std::string>();
    if (vm.count("watch"))
    {
      pid = waitForProgramLaunchSync(utf8ToUtf16(exe_name));
    }
    else
    {
      if (blackbone::Process::EnumByName(utf8ToUtf16(exe_name)).empty())
      {
        std::cerr << "The process doesn't exist\n";
        return 1;
      }

      pid = blackbone::Process::EnumByName(utf8ToUtf16(exe_name))[0];
    }
    
    blackbone::Process process;
    HANDLE h = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    process.Attach(h);

    blackbone::pe::PEImage dll_peimage;
    dll_peimage.Load(utf8ToUtf16(dll_path));

    // Check the file format
    if (dll_peimage.mType() == blackbone::eModType::mt_default || dll_peimage.mType() == blackbone::eModType::mt_unknown);
    else if (process.core().isWow64() && dll_peimage.mType() != blackbone::eModType::mt_mod64)
    {
      std::cerr << "Error: attempt to load a 32 bit dll on a 64 bit process\n";
      return 1;
    }
    else if (!process.core().isWow64() && dll_peimage.mType() != blackbone::eModType::mt_mod32)
    {
      std::cerr << "Error: attempt to load a 64 bit dll on a 32 bit process\n";
      return 1;
    }

    // NT_ERROR
    Sleep(delay * 1000);

    // attach the dll
    if (vm.count("unload-dll"))
    {
      std::cout << "Unloading dll. . ." << std::endl;
      auto dll_ = process.modules().GetModule(utf8ToUtf16(vm["dll_"].as<std::string>()));
      if (dll_->type != blackbone::eModType::mt_unknown || dll_->size == 0x0)
      {
        if (nt_error(process.modules().Unload(dll_)))
          std::cerr << "There was a error unloading the dll on the process. Continuing execution of the program." << std::endl;
        else
        {
          std::cout << "Unloaded dll!" << std::endl;
        }
      }
      else
        std::cerr << "Couldn't unload dll as it's not present in memory." << std::endl;
    }

    if (dll_peimage.pureIL())
    {
      bool didInjectILDll;
      DWORD returnCode = 0;

      didInjectILDll = process.modules().InjectPureIL(
          blackbone::ImageNET().GetImageRuntimeVer(dll_peimage.path().c_str()),
          dll_peimage.path(),
          std::wstring(),
          std::wstring(),
          returnCode);

      if (!didInjectILDll)
      {
dll_injection_error:
        std::cerr << "An error has happened while injecting the dll\n";
        return 1;
      }
    }
    else
    {
      bool didInjectedDll;
      if (!process.modules().Inject(dll_peimage.path(), process.threads().getMostExecuted()).success())
      {
        std::cerr << "An error has happened when injecting the dll\n";
        return 1;
      }
    }
    std::cout << "Sucesfully injected the dll into the process!\n";
    return 0;
  }

  std::cout << desc << "\n";
  return 0;
}

bool nt_error(NTSTATUS status)
{
  return ((((ULONG)(status)) >> 30) == 3);
}