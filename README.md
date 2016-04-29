MEKA
====

- Homepage: http://www.smspower.org/meka
- Forum: http://www.smspower.org/forums/f7-MEKA

You can download the latest beta windows binaries from this forum thread:
http://www.smspower.org/forums/13019-Meka080WithNewSoundEngineTESTERSWANTED?start=300#86895

MEKA is a multi-machine emulator. The following machines are supported by MEKA:

- Sega Game 1000 (SG-1000)
- Sega Computer 3000 (SC-3000)
- Sega Super Control Station (SF-7000)
- Sega Mark III (+ FM Unit)
- Sega Master System (SMS)
- Sega Game Gear (GG)
- ColecoVision (COLECO)
- Othello Multivision (OMV)

Along with a wide range of peripherals and exotic games support. 

MEKA should compile for MS-Windows, GNU/Linux and OSX (older versions support MS-DOS).

MEKA was started in 1998 and first released on April 3rd 1999. It was my first major C project, so the codebase is old, ugly and messy! It is still being updated sometimes.

In spite of its old age and clunky technology, MEKA is possibly the most exhaustive Sega 8-bit emulator in term of coverage of obscure games, peripherals. And also provide competent debugging and reverse engineering tools. It is still maintained for those purpose but doesn't has much use for the average player. 
The OLD SVN repository is still available for reference at svn://svn.smspower.org/meka/

MY MODIFICATIONS
====

- 3D Side-by-Side mode was implemented.  This enables native 3D support found in modern 3D TVs.

- Added a new "Status" menu option to the 3-D menu.  Options are now Disabled/Enabled/Auto.
The Auto option will selectively enable 3D mode in games that support the feature.
The Enabled option will force 3D to enabled.  This is useful for "Line of Fire", which has a hidden 3-D mode.

- Made various changes to allow compilation in Xcode

- Made Mouse cursor hide in Fullscreen mode (annotated with // TOMMOUSE //)
This was done in anticipation of bluetooth Wiimote support for use as a light gun (not yet implemented)
I may instead opt to get WJoy (or similar) working in El Capitan --In particular, the IR Mouse feature.
This would allow a Wiimote to be used as a light gun on any emulator that has mouse-based light gun support.

- Contains a workaround for what I assume to be a bug in Allegro's fullscreen support.
Instead of using fullscreen mode, it's now using a "windowed" fullscreen mode.
If the fullscreen mode is enabled in the meka.cfg, the resolution parameter is ignored, and the desktop resolution is used instead.
The parameter is respected when running in windowed mode.

FUTURE PLANS
====
- Compile/build on Windows and Linux in addition to OS X.
- Support for Wiimote as IR light gun.
- Get these changes incorporated into the official repo.  If someone wants to do that, feel free to do so (and let me know)!
I honestly wasn't sure where/how to go about doing that, so I made a new repo instead.
- Some refactoring is probably in order.  For instance, there's a good chance I broke COM port 3D.  Didn't have the hardware to test.

NOTE
====
I noticed a few rom files which I omitted from version control  These may be needed for the build:
I wasn't sure if these were "official" or "reverse engineered", so I just left them out.
- meka/data/rom_coleco.rom
- meka/data/rom_sf7000.rom
- meka/data/rom_sms.rom
- meka/data/rom_smsj.rom

These files should go into Meka.app/Contents/Resources/Data/

The compiled OS X binary is at meka/Dist/Meka.app


Gallery
-------

![Debugger Screenshot](http://www.smspower.org/forums/files/meka_080_wip_debugger_823.png)

![Debugger Screenshot](http://www.smspower.org/meka/gallery/meka072-wip-sagaia.png)
