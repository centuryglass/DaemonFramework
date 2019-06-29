# KeyDaemon
  KeyDaemon is a Linux setuid helper program that can globally monitor keyboard events, so that the application it supports can detect keyboard input even if another window is focused. This is intended to simplify the process of adding global hotkeys/controls to any application, without relying on any specific desktop environment or even the X window server.

### Security
To keep this from indiscriminately leaking keyboard input data, the KeyDaemon operates with a strict set of restrictions. If any of the following conditions are not met, the application will terminate.

- KeyDaemon may only run from a specific installation path, set on compilation.

- Each KeyDaemon is linked to a specific parent application path on compilation. Only processes running that application may launch KeyDaemon.

- KeyDaemon's executable and its parent application executable must be located in secured directories, only editable by root.

- KeyDaemon must be given a limited set of valid keyboard codes on launch, containing no invalid input, and not exceeding the maximum tracked key count defined on compilation.

### TODO
- I still need to implement a means of secure inter-process communication to send key codes to the parent application. The current plan is to use a named Linux pipe file, secured by a custom application user group.

- The specific keyboard event file used is currently hard-coded in. I plan to update the KeyReader class to scan all keyboard event files, and add a build option to enable optional restrictions on the set of accessible event files.
