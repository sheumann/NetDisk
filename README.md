NetDisk
=======
By Stephen Heumann

NetDisk is an Apple IIGS utility that allows you to mount disks images hosted on an HTTP server (a public web server or one on your local network).  The mounted disk images behave as if they were normal (but read-only) disks on your IIGS: you can open files and run programs directly off of them, or copy the files to your local disks using the Finder or other standard utilities.


System Requirements
-------------------
* An Apple IIGS with System 6.0.1 or later
* Marinetti
* A Marinetti-compatible network interface (such as an Ethernet card)


Installation
------------
To install NetDisk, place the `NetDiskInit` file in the `*:System:System.Setup` folder (where `*` indicates your boot disk), and place the `NetDisk` file in the `*:System:CDevs` folder.  Reboot the system to complete the installation.


Compatibility
-------------
NetDisk can access disk image files on most servers that allow unencrypted HTTP access.  The server must also support HTTP range requests.  Most modern servers support these, but some may not, in which case an error message will be shown.

NetDisk supports raw (aka ProDOS-order and various other names) or 2MG disk images.  DOS-order images and other formats are not supported.  The images may use any filesystem for which you have an FST installed, except that DOS 3.3 filesystems are not supported due to a limitation in that FST.  You can use images of any size (subject to the limitations of the relevant FST), from floppy disks to large hard drive partitions or CD-ROMS.


Usage
-----
To mount a disk image, simply open the __NetDisk__ control panel, enter the http:// URL for the image to be mounted, and click __Mount Disk Image__.  If you do not get an error message, the disk will now be mounted and accessible.  If you are in the Finder, it will appear on the desktop.

The __Use Disk Cache__ check box controls whether the standard GS/OS disk caching mechanism should be used with the disk image.  This box should normally be left checked, since it improves performance.  You should un-check it if the disk image on the server might change while you have it mounted (but note that such changes can cause problems in some cases even if caching is disabled).

Disk images mounted by NetDisk are read-only. They can be accessed while running any 16-bit GS/OS application, but not in ProDOS 8 mode.

When you are done with a disk image, you can unmount it as you would any other disk (e.g. by dragging it to the trash in the Finder).  Up to 16 disk images may be mounted at once.
