Since MAGNA++ uses pthreads, we need a POSIX compliant compiler with pthreads support for it to work.

Note, the MAGNA++ GUI uses FLTK (Fast Light Toolkit). So FLTK needs to be installed in order to compile it. FLTK is statically linked into MAGNA++, so there's no need to install anything to run the executable itself.

MAGNA++ also uses the Eigen matrix library, which needs to be installed before compiling.

In Unix and Mac OS X, you can used gcc and its variants. You will need to install FLTK and Eigen.

To compile in Unix, Mac, or Cygwin, run make clean and then
make all
and/or
make gui

The first command compiles the CLI code. The second compiles the GUI code.


For Windows, since Visual C++ doesn't support pthreads (and a bunch of other POSIX stuff that I used), I opted to use MinGW. 

Here is what I did specifically.

1. I went to http://win-builds.org/ and followed the instructions to install MinGW-w64 (either 32-bit or 64-bit)

2. I went to https://www.sourceware.org/pthreads-win32/, downloaded it and compiled it using the command "make clean GC-static"
The reason I am not using the native pthreads library that MinGW-w64 uses (which is called winpthreads) is that it seems to be unstable and MAGNA++ crashed a few times when I used winpthreads.
Look in the Makefile.mingw file to find out the location to put pthreads-win32 in.

3. I installed FLTK (http://www.fltk.org/index.php)

4. Installed Eigen (http://eigen.tuxfamily.org/index.php?title=Main_Page)

5.
To compile with MinGW-w64, run make clean and then

make -f Makefile.mingw all
and/or
make -f Makefile.mingw gui

This makefile, Makefile.mingw, just contains some flags for MingGW-w64 so that pthreads and FLTK can compile statically without problems (and so it uses pthreads-win32)
