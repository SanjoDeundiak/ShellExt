<h1>Overview</h1>

This is Windows Shell Context Menu Extension COM component 
that allows users to get some log-file with important information:
date, size and a per­byte sum (a flavour of checksum, 4 bytes for
each file).

In this extension I have implemented thread pool using boost 
concurrency for finding check-sum. It is ready to work with huge 
amount of files.

Part of code related to COM is from Microsoft's example:
http://code.msdn.microsoft.com/windowsdesktop/CppShellExtContextMenuHandl-410a709a

But there were several minor bugs in this example, that I had fixed.

<h2>Installation</h2>

1. Build the project. Pay attention to use x64 or x32 settings depending on your system
2. To use this program you need to run cmd with admin privileges, go to directory with
your dll file. And type
#-regsvr32 CppShellExtContextMenuHandler.dll
3. In order to delete this program first do
#-regsvr32 /u CppShellExtContextMenuHandler.dll
to clear register.

<h3>Usage</h3>

Just select files that you need, click right button and choose appropriate menu item.
log.txt will be created in this directory with all information
