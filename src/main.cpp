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
    std::wstring processName = utf8ToUtf16(vm["process-name"].as<std::string>());
    std::cerr << "Feature not added yet"    
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
    std::cout << "Got PID: " << pid << "\n";
  }

  return 0;
}