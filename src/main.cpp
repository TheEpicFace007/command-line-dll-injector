#ifdef DEBUG
#else
// #pragma comment(lib, "../BlackBone.lib")
#pragma comment(lib, "../dBlackBone.lib")
#endif

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

using namespace std::chrono_literals;
namespace po = boost::program_options;

int main(int argc, char const *argv[])
{

  blackbone::InitializeOnce();

  po::options_description desc("Allowed options");
  desc.add_options()
  ("help,h", "Produce help message of the program")
  ("dll,d", po::value<std::string>(), "The dll to inject")
  ("exe,e", po::value<std::string>(), "The program exe file name to inject the onto Example: mspaint.exe")
  ("process-name,p", po::value<std::string>(), "The program process name")("watch,w", "Inject the DLL as soon as the specified program is launched")
  ("delay", po::value<float>(), "Wait a certain aount of time before injecting in seconds");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm, true);
  po::notify(vm);

  if (vm.count("help"))
  {
    std::cout << desc << "\n";
    return 0;
  }

  if (vm.count("process-name") ^ vm.count("exe") == false)
  {
    std::cerr << "Error: " << "Invalid args." << std::endl;
    std::cerr << desc << std::endl;
    return 1;
  }

  // Set the delay
  float delay;
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
  }

  // Attacg the dll
  if (vm.count("process-name"))
  {
    std::cerr << "Feature not added yet" << std::endl; 
    return 1;
  }
  else if (vm.count("exe"))
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
    process.EnsureInit();
    HANDLE processHandle;
    HANDLE h = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

    blackbone::pe::PEImage dll_peimage;
    dll_peimage.Load(utf8ToUtf16(dll_path));

    // Check the file format
/*     if (IsWow64(h) && dll_peimage.mType() != blackbone::eModType::mt_mod64)
    {
      std::cerr << "Error: attempt to load a 32 bit dll on a 64 bit process\n";
      return 1;
    }
    else if (IsX86Process(h) && dll_peimage.mType() != blackbone::eModType::mt_mod32)
    {
      std::cerr << "Error: attempt to load a 64 bit dll on a 32 bit process\n";
      return 1;
    }
 */    
    process.Attach(pid);
    if (dll_peimage.pureIL())
    {
      bool didInjectILDll = process.modules().InjectPureIL(
        blackbone::ImageNet::GetImageRuntimeVer(dll_peimage.path().c_str()),
        dll_peimage.path(),
        std::wstring(),
        std::wstring(),
        0
      );

      if (!didInjectILDll)
      {
        std::cerr << "An error has happened while injecting an Il(dot net) dll\n";
        return 2;
      }
    }
    else
    {

    }
  }

  return 0;
}