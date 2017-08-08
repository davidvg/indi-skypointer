SkyPointer INDI Driver
======================

This package provides a INDI driver for
[SkyPointer](https://github.com/juanmb/skypointer).

Requirements
------------

 * cmake
 * [INDI library](http://www.indilib.org/) >= v0.9.6

You need to install both indi and indi-devel to build this package.

Installation
------------

You must have CMake >= 2.4.7 in order to build this package.

    $ cmake .
    $ make
    $ sudo make install

How to Use
----------

You can use the Generic INDI Driver in any INDI-compatible client such as
KStars or Xephem.

To run the driver from the command line:

    $ indiserver -v indi_skypointer

You can then connect to the driver from any client, the default port is 7624.
If you're using KStars, the driver will be automatically listed in KStars'
Device Manager, no further configuration is necessary.

Author
------

* Juan Menendez <juanmb@gmail.com>
