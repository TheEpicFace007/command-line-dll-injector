# command-line-dll-injector
An command line utility made for injecting Dll

# Features
* Inject DLL
* Wait for a process to launch and inject the dll as soon as it launch
* Have a delay before injecting the dll

# Command line argument

|      **Argument**      | **Description**                                      | **Example**          |
|:----------------------:|------------------------------------------------------|----------------------|
|    `-h [ --help ]`     | Produce a help message                               |                      |
|   `-d [ --dll ] arg`   | Specify the dll to launch                            | `--dll cheat.dll`    |
| `-p [ --process ] arg` | Specify the program name to inject                   | `--process MGSV.exe` |
|    `-w [ --watch ]`    | Wait for the specified to be opened before injecting | `-w -p game.exe`     |
|     `--delay arg`      | Wait a amount of time before injecting(in seconds)   | `--delay 1.5`        |

**Example:** \
`injector.exe --dll my-dll.dll --process game.exe --delay 2.5 -w`