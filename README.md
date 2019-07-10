# DaemonFramework
  A generic framework for secure Linux setuid helper applications.

Warning: This project is not at all usable yet. I'm modifying my KeyDaemon project to create it and I have just barely started. I should have a usable version up within a day or two.

### Security
If any of the following conditions are not met, the application will terminate.

- A daemon may only run from a specific installation path, set on compilation.

- Each daemon is linked to a specific parent application path on compilation. Only processes running that application may launch the daemon.

- The daemon's executable and its parent application executable must be located in secured directories, only editable by root.
