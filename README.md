# command-line-dll-injector
An command line utility made for injecting Dll

<iframe src='https://gfycat.com/ifr/OblongOldGrayling' frameborder='0' scrolling='no' allowfullscreen width='640' height='404'></iframe>

# Features
* Inject dll as soon as a program launch
* Have a delay before injecting the dll

* Easy to integrate into others programs

The program will return 1 if there's a error and send error to stderr and will send the non-error messages to stdout

* Respect the [CLI guidelines](https://clig.dev/)

The program fully respect  the CLI Guidelines which mean that the program will be easy to use both for the user and if the program is integrated into another program.

# Command line argument

|    **Argument**    | **Description**                                                                           | **Example**          |
|:------------------:|-------------------------------------------------------------------------------------------|----------------------|
|  `-h [ --help ]`   | Produce a help message                                                                    |                      |
| `-d [ --dll ] arg` | Specify the dll to launch                                                                 | `--dll cheat.dll`    |
| `-e [ --exe ] arg` | Specify the program name to inject                                                        | `--exe MGSV.exe` |
|  `-w [ --watch ]`  | Wait for the specified to be opened before injecting                                      | `-w -p game.exe`     |
|   `--delay arg`    | Wait a amount of time before injecting(in seconds)                                        | `--delay 1.5`        |
| `-u [ --unload ]`  | Unload the dll that is going to be injected if it's already present in the process memory |                      |

# Example:

* `.\command-line-dll-injector.exe -d ..\..\test-dll\test-dll-x64.dll --delay 6 -w --exe mspaint.exe`
* `.\command-line-dll-injector.exe -d ..\..\test-dll\test-dll-x64.dll --delay 6 -w --exe mspaint.exe --unload`