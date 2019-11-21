File Shredder

File shredder runs under the Windows operating system (XP and later) to delete files on a hard disk drive in a manner that is 
intended to make any recovery of the data impossible.

To install File Shredder copy File Shredder.exe to your desktop, or other folder, on a computer using a Windows operating 
system.

To shred files select them and then drag and drop them onto the icon. The first time you run the program it will present you 
with the licence agreement, once this is accepted you will be prompted to confirm that you wish to delete the files and, if 
you confirm this, it will begin to shred the files. Alternatively you can double click on the program icon to open it. Here 
too, on first use, the licensing agreement will be presented and, once the terms and conditions have been accepted you can 
select how the program runs through the Options menu on the menu bar. You can choose between one of three 'Modes':

   'Silent' meaning the program begins to shred files that are dragged and dropped onto the icon immediately without even 
      asking for confirmation that you wish to delete files; 
   'Request confirmation' where you will be asked to confirm that you wish to delete the files.
   'Verbose,' the default mode, where a window will inform you of the progress the program is making.

In addition you can choose one of two 'Methods':
 
  'Hide files before deletion,' where the files will be transferred to a temporary folder before they are deleted, here if
      you log off during the run or the computer crashes any files that remain will be deleted when you next use your account.
   'Delete selected files directly,' the default method, where files are removed one by one from wherever they happen to be.

Your selections, along with the coordinates of the various windows will be recorded in the Windows registry ready for next 
time. 

The program overwrites the data in each file with: its inverse, all zeros, all ones, followed by two passes of random nubers 
seeded individually. Each pass of data is written using commands found in the Windows API, rather than using those native to 
C++ so that all file buffers are be flushed to disk before the next pass is initiated. When fstream was used buffering by 
Windows was a problem.

Alan Entwistle, Entyware

This program was developed using Bloodshed Software Dev C++ version 4.9.9.2 and random numbers are generated using a Mersenne
Twister © 2003 written by Richard J. Wagner, building on work by Makoto Matsumoto, Takuji Nishimura, and Shawn Cokus.

