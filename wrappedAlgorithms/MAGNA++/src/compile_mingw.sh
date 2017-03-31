# eventually use gcc -dumpmachine I guess (x86_64-w64-mingw32 vs i686-w64-mingw32)
HERE=`pwd`
cd ../../pthread-win32
make realclean GC-static
cd $HERE
make -f Makefile.mingw clean gui
make -f Makefile.mingw all
mv magna.exe ../execs/magnapp_cli_win64.exe
mv magnafluidui.exe ../execs/magnapp_gui_win64.exe
