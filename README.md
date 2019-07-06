NetDisk
=======
By Stephen Heumann

NetDisk is an Apple IIGS utility that allows you to mount disks images hosted on an HTTP server (a public web server or one on your local network).  The mounted disk images behave as if they were normal (but read-only) disks on your IIGS: you can open files and run programs directly off of them, or copy the files to your local disks using the Finder or other standard utilities.

The NetDisk package also comes with the Archive.org Disk Browser, a Finder extension that lets you conveniently search for disk images on the Internet Archive (archive.org) and mount them using NetDisk.


System Requirements
-------------------
* An Apple IIGS with System 6.0.1 or later
* Marinetti 3.0b11 or later
* A Marinetti-compatible network interface (such as an Ethernet card)
* 3 MB or more of memory recommended if using Archive.org Disk Browser


Installation
------------
To install NetDisk and the Archive.org Disk Browser:

* Place the `NetDiskInit` file in the `*:System:System.Setup` folder (where `*` indicates your boot disk).
* Place the `NetDisk` file in the `*:System:CDevs` folder.
* Place the `DiskBrowser` file in the `*:System:FinderExtras` folder. (If your IIgs uses a multi-user network booting setup, place it in the `FinderExtras` folder within your user folder instead.)

Reboot the system to complete the installation.


Compatibility
-------------
NetDisk can access disk image files on most servers that allow unencrypted HTTP access.  The server must also support HTTP range requests.  Most modern servers support these, but some may not, in which case an error message will be shown.

NetDisk supports raw (aka ProDOS-order and various other names), 2MG, DOS-order, and DiskCopy 4.2 disk images.  The images may use any filesystem for which you have an FST installed, except that DOS 3.3 filesystems are not supported due to a limitation in that FST.  You can use images of any size (subject to the limitations of the relevant FST), from floppy disks to large hard drive partitions or CD-ROMS.


Using NetDisk
-------------
To mount a disk image, simply open the __NetDisk__ control panel, enter the http:// URL for the image to be mounted, and click __Mount Disk Image__.  If you do not get an error message, the disk will now be mounted and accessible.  If you are in the Finder, it will appear on the desktop.

You can explicitly select the disk image format using the __Format__ pop-up menu.  In most cases this can be left at the default "Auto-Detect" setting to let NetDisk detect the format of the image, but you can explicitly specify a format if the auto-detection does not work properly.

The __Use Disk Cache__ check box controls whether the standard GS/OS disk caching mechanism should be used with the disk image.  This box should normally be left checked, since it improves performance.  You should un-check it if the disk image on the server might change while you have it mounted (but note that such changes can cause problems in some cases even if caching is disabled).

Disk images mounted by NetDisk are read-only. They can be accessed while running any 16-bit GS/OS application, but not in ProDOS 8 mode.  (This means that in many cases 16-bit applications can be run directly off the disk image, but 8-bit programs will have to be copied to a local drive and run from there.)

When you are done with a disk image, you can unmount it as you would any other disk (e.g. by dragging it to the trash in the Finder).  Up to 16 disk images may be mounted at once.


Using the Archive.org Disk Browser
----------------------------------
The Archive.org Disk Browser is a Finder extension that lets you search for disk images on the Internet Archive (archive.org) and mount then using NetDisk.  To use it, select __Archive.org Disk Browser...__ in the Finder's __Extras__ menu.

In the disk browser window, you can enter what to search for, choose if you want to find disks for the Apple IIGS or any Apple II, and then click __Find Disks__ to find any matching disks.  

When the search is done, a list of matching disks will be displayed.  You can select a disk and click __Mount Disk__ (or double-click the disk name) to mount it using NetDisk.


Troubleshooting Notes
---------------------
* In some system configurations, there can be a compatibility issue between GUPP 1.07 and NetDisk, which manifests itself as hanging when starting or quitting applications.  If you experience this, I suggest either disabling GUPP or downgrading to GUPP 1.06.  You may also be able to work around this by changing the order of inits or disabling other inits that patch tools.
