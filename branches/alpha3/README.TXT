-=[GREETS]=-

Welcome to the wonderful world of Minnow, a programming language designed to be easy to use and robust enough for today's multiprocessor hardware.

-=[GETTING THE LATEST]=-

All the latest news on Minnow is available from the Minnow website -- http://www.minnow-lang.org 

Make sure to check it from time to time (or add the RSS to your reader) to keep up to date.

-=[COMPILING]=-

To get started, make sure you have an updated copy of cmake for your platform.  Minnow has been tested with Ubuntu, OS X (cmake from macports), and Windows Vista (cmake from main cmake site).

On Linux:
  - cmake -DCMAKE_BUILD_TYPE=gdb .
  - make
  
On Mac OS X:
  - cmake -DCMAKE_BUILD_TYPE=gdb .
  - make

On Windows Vista:
  This readme assumes you have installed mingw and msys, as well as the additional bash package (if necessary).  Once installed:

  - bash
  - cmake -DCMAKE_BUILD_TYPE=gdb -G "MSYS Makefiles" .
  - make

If all goes well you'll have a libaquarium in the main directory as well as a ./bin/minnowc (or bin/minnowc.exe on Windows).  This is the minnow compiler.  To do its dirty work, it requires GCC, but assuming you built the above correctly, we can assume you have GCC also.  If you built aquarium and minnow with Visual Studio, the compiler will instead look for the "cl.exe" commandline compiler.

-=[USAGE]=-

Usage: minnowc <options> <filename(s)>
-o <filename>   : compile and output to a binary file
-O <level>      : set the optimization level
-I <source dir> : add a source include directory to your search path
-L <lib dir>    : add a library directory to the library search path
-C              : output the generated C file to the stdout
-v              : print version
-h              : this help

To compile a Minnow source file, make sure you're in the main source directory (where libaquarium is), and run:

  ./bin/minnowc <SOURCE FILE> -o <OUTPUT BINARY>

There are lots of examples to get you started in the ./samples directory.  Also be sure to check out the Minnow documentation wiki at: http://www.minnow-lang.org/docs

-=[BUGS]=-

If you discover a bug, let us know.  The project is hosted on Google Code at: http://code.google.com/p/minnow-language/ and you should be able to get noticed filing a bug in the [Issues] tab.

-=[SVN]=-

The latest source is available from subversion at the Google Code website: http://code.google.com/p/minnow-language/ under the [Source] tab.

-=[THANKS]=-

Big thanks to Jason Turner (http://www.emptycrate.com) for more coding discussions than anyone would think possible.  Special thanks to Joe Armstrong, Simon Peyton-Jones, Robin Milner, Bjarne Stroustrup -- I've long lost count of how many times they've inspired another push forward while trying to figure out how far up their shoulders are.
