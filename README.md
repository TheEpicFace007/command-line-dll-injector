# command-line-dll-injector
An command line utility made for injecting Dll

# Features
* Inject DLL
* Inject dll as soon as a program launch
* Have a delay before injecting the dll

# Command line argument

|    **Argument**    | **Description**                                      | **Example**          |
|:------------------:|------------------------------------------------------|----------------------|
|  `-h [ --help ]`   | Produce a help message                               |                      |
| `-d [ --dll ] arg` | Specify the dll to launch                            | `--dll cheat.dll`    |
| `-e [ --exe ] arg` | Specify the program name to inject                   | `--process MGSV.exe` |
|  `-w [ --watch ]`  | Wait for the specified to be opened before injecting | `-w -p game.exe`     |
|   `--delay arg`    | Wait a amount of time before injecting(in seconds)   | `--delay 1.5`        |

**Example:** \
`.\command-line-dll-injector.exe -d ..\..\test-dll\test-dll-x64.dll --delay 6 -w --exe mspaint.exe`