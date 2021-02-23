#include <iostream>
#include <Windows.h>
#include <WinApiWrapper.h>
#include <dbg.h>
#include <boost/program_options.hpp>
#include <string>
#include <thread>
#include <chrono>
#include <BlackBone/Process/RPC/RemoteFunction.hpp>

using namespace std::chrono_literals;
namespace po = boost::program_options;
void handleInjectionMessage(InjectDllReturnValue injectionMessage);

    int main(int argc, char const *argv[])
{
  po::options_description desc("Allowed options");
  desc.add_options()
  ("help,h", "Produce help message of the program")
  ("dll,d", po::value<std::string>(),"The dll to inject")
  ("process,p", po::value<std::string>(), "The program exe name to inject the dll. Example: mspaint.exe")
  ("watch,w", "Inject the DLL as soon as the specified program is launched")
  ("delay", po::value<float>(), "Wait a certain aount of time before injecting in seconds")
  ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm, true);
  po::notify(vm);

  if (vm.count("help"))
  {
    std::cerr << desc << "\n";
    return 0;
  }

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


  return 0;
}

void handleInjectionMessage(InjectDllReturnValue injectionMessage)
{
  std::cout << (int)injectionMessage << "\n";
  switch (injectionMessage)
  {
  case InjectDllReturnValue::AcessWindowHandleError:
    std::cerr << "Failed to acess the process handle.\n";
    exit(1);
    break;
  case InjectDllReturnValue::InexistantDLL:
    std::cerr << "The DLL doesn't exist. Have you specified the right path of the dll?\n";
    exit(1);
    break;
  case InjectDllReturnValue::InjectionFail:
    std::cerr << "Injection fail.\n";
    exit(1);
  case InjectDllReturnValue::InjectionSucess:
    std::cout << "Sucessfully injected dll!\n";
    exit(0);
    break;
  case InjectDllReturnValue::MemAlocationError:
    std::cerr << "Failed alocating the memory\n";
    exit(1);
    break;
  }
}