# DaemonFramework
A generic framework for programs that support larger applications by performing tasks that require greater privileges.

Daemons created with the DaemonFramework run with extra privileges, either granted with setcap, or by configuring the daemon executable to run as a privileged user or group. DaemonFramework provides a system allowing your application to launch and communicate with these daemons over named pipes. More importantly, DaemonFramework provides a set of configurable security restrictions to ensure the daemon only runs in the expected context, for its intended purpose. These security options are documented in the [Daemon makefile](./MakeDaemon.mk).

This project is still in its early stages, and in most cases I'd advise against using it as it is now. All basic functionality is implemented and working, but testing and documentation are not yet complete, and additional security research is required.

In the future, this project will contain guides and templates for creating a daemon and controlling it within your application. Meanwhile, the test daemon defined in Tests/BasicDaemon and the test parent application defined in Tests/BasicParent can be used as examples.
