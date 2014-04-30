Compiling QWave from source
===========================

Pre-requisites
--------------

QWave depends on the following software packages. Make sure these are available
before you begin.

- libsndfile
- libsamplerate
- Qt4

If you want to compile the Python binding, the followings are needed as well.

 Python
 SIP
 PyQt4

On many linux distros, you will have to install the "devel" packages too,
since the header and library files are included in those "devel" packages.

On linux
--------

The following will work most of the time.

 cd src
 qmake
 make

If not, it's likely that the header files or library files of libsndfile and
libsamplerate are not found. Check src/lib.pro file, especially the
"unix:linux-g++" section. Look for lines with INCLUDEPATH and LIBS and fix them
if necessary.

Also, make sure that the right version of Qt is being used. Try "qmake -query".

Also note that having Qt3 headers and libraries in compiler's search path
causes wierd problems.

To compile a QWave Python module, try the following.

 mkdir python
 cd sip
 python config.py
 cd ../python
 make

If things go wrong, this time check the config.py file. Especially, check lines
with "makefile.extra_include_dirs" and "makefile.extra_lib_dirs".

