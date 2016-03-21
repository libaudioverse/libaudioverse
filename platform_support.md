#Libaudioverse Build Instructions and Platform Support

This document outlines Libaudioverse's support for various platforms and how to build it on each.  If a platform isn't in this document, it's probably planned for the future (this project will run everywhere, one day).  Supported platforms are currently Windows (X86 only) and Linux (X64 and X86).

Instructions are intentionally as basic as possible.  If you know what you're doing, you don't need to follow them to the letter.  If something breaks, however, please follow this document closely before reporting it.  If you prefer something briefer (and more cryptic for new programmers) see `appveyor.yml` (Windows) or `.travis.yml` (Linux) in the root of this repository.

##Dependencies For All Platforms

You need the following on all platforms.  Installation instructions for each platform may be found below:

- Boost.  I test with 1.59.0 and 1.58.0.  Travis CI has 1.54.0 and compiles, but nothing runs the resulting binaries due to lack of the ability to listen to the output.
- Python 3.4 or later.
- CMake 2.8 or later.
- The python packages PyYAML, Pypandoc, Jinja2, pycparser, sphinx, wheel, numpy, and scipy.
- A working installation of pandoc (for some platforms; on Windows, pypandoc bundles it).
- A C++14 capable compiler.

In addition, building the manual requires asciidoctor and ruby.  The manual does not build by default, so lack of these packages should not be detrimental.

##Windows

Libaudioverse supports Windows Vista or later when built for X86.  X64 configurations are not supported, primarily due to difficulties with Libsndfile.  Your compiler must be VC++ 2015.

Furthermore, again due to difficulties with Libsndfile, you need to building on an X64 version of Windows even though you are going to get X86 binaries.  This limitation will also be lifted as soon as is feasible.

Python bindings may be installed with pip.  If all you need is the Python bindings, building Libaudioverse is not necessary.  If installation with pip fails, be sure to upgrade pip, setuptools, and wheel.

The rest of this section specifically outlines how to build Libaudioverse on Windows.

###Getting Dependencies

You need to do the following steps exactly once.  Most of this is general development setup for C++ and Python, and the list of dependencies at the top of this document should be sufficient if you know how to properly install them.

First, install [Visual Studio 2015](https://www.visualstudio.com/en-us/products/visual-studio-community-vs.aspx) or later.
As of Visual Studio 2015, the C++ compiler is no longer included by default.  You need to do a custom installation and make sure to check it and all components related to it.
Be prepared for this installer to spend 2 or 3 hours and to use multiple gigabytes of hard drive space as well as a great deal of bandwidth: Visual Studio is a very large package.

Grab the [newest version of Python 3](https://www.python.org/downloads/).  Both 3.4 and 3.5 are known to work.  Install it, go to a command prompt, and make sure the command `py -3` works.  If this launches a 64-bit Python, you will have problems with the rest of this process (This can't be fixed, unfortunately).  If this command doesn't work, something has gone wrong with the installation.

You may need to upgrade setuptools, wheel, and pip.  Old versions of these packages have bugs which will cause the build to fail when it tries to generate the Python wheel.  Unless you have a reason not to do so, execute the following command: `py -3 -m pip install --upgrade pip setuptools wheel`.

Next, grab 32-bit versions of Numpy and Scipy.  The easiest way to get them is via [Christoph Gohlke's site](http://www.lfd.uci.edu/~gohlke/pythonlibs/).  You need to make sure that the versions you obtain match the version of Python you have.  Install them with `py -3 -m pip <path_to_wheel>`.

To finish Python setup, execute `py -3 -m pip install pycparser jinja2 pyyaml pypandoc sphinx`.

Note: We use `py -3 -m pip` because, on many machines, `pip` points to Python 2.

Next, obtain and run the [Libsndfile Installer.](http://www.mega-nerd.com/libsndfile/).  Make sure this is the one for 32-bit Windows.

Then, obtain and run the [CMake Installer](https://cmake.org/download/).  While 2.8 technically works, 3.3 or later is strongly suggested due to various fixed bugs between then and now.  You need CMake to be added to your path; the command `cmake --version` should work at a command prompt.  For some reason, restarting the machine can sometimes make it show up when it fails to do so immediately after install.  If this still doesn't fix it, you may need to add it manually.

Next, you need to find the Visual Studio development command prompt for 32-bit C++.  This is renamed and moved in every release of Visual studio and also varies between Windows versions.  If you think you will be building Libaudioverse frequently, I suggest putting it on the desktop so you can find it again easily.

For Windows 7 and Visual Studio 2015, you can find it in start>all programs>Visual Studio 2015>Visual Studio Tools.  You know you have the right one if the output of the command `cl` looks like the following (note the X86.  X64 is an indicator that you are using the wrong command prompt):

```
Microsoft (R) C/C++ Optimizing Compiler Version 19.00.23506 for x86
Copyright (C) Microsoft Corporation.  All rights reserved.

usage: cl [ option... ] filename... [ /link linkoption... ]
```

The rest of the commands in this section must be run from this command prompt.

The last required step is to get and build boost.  To do so, you need [the Boost sourceccode](http://www.boost.org/).  Download it and extract it to a directory without spaces.

Open your developer command prompt to this directory, and run the following two commands.  The second will take a great deal of time to run.  You need the output from the final process, so don't clear the command prompt:

```
bootstrap.bat
b2 variant=debug,release link=static threading=single,multi address-model=32 toolset=msvc-14.0 runtime-link=shared,static
```

Note that `b2` accepts an option of the form `-j n` to specify the number of threads to use.  You may optionally add this if you have a machine with lots of cores, but it is not shown in the above because `n` should match the number of cores on your machine.

If everything was successful, you were told to add some things to environment variables.  You have two choices.  The first is to go to control panel and add them permanently (not recommended).  What I suggest is to create a `cppshell.bat` file and put it somewhere on the path.  You can then add whatever directories need to be added there as opposed to adding them permanently.  Simply run `cppshell` from a Visual Studio developer command prompt before using things that require Boost.  As an example, here is mine (be sure to edit the paths for your setup):

```
@echo off
echo Adding environment variables...
set include=c:\boost_1_59_0;%include%
set lib=c:\boost_1_59_0\stage\lib;%lib%
```

The rest of this section is optional.

If you wish to be able to build a local copy of the manual, you must also obtain [Ruby](https://www.ruby-lang.org/en/documentation/installation/).  Once you have done so, run the command `gem install asciidoctor` from a command prompt.

If you are going to be building Libaudioverse frequently, you may want a faster build method than NMake, which only uses one core.  The one I use is [Jom](https://wiki.qt.io/Jom).  Download it and add it to your path.

###Building

If you got this far, you're done and don't need to run the above stuff ever again.  Also you're set up for C++ development on Windows.

Clone the Libaudioverse repository or otherwise obtain the Libaudioverse source code (git is not mentioned above; you don't need it because you can just download a zip from GitHub).  Open your developer command prompt to the directory to which you extracted the source code.  Possibly run your `cppshell.bat`.  Then run the following commands:

```
mkdir build
cd build
cmake -G "NMake Makefiles" ..
nmake
```

If you wish to use jom, then replace the last two commands with these:

```
cmake -G "NMake Makefiles JOM" ..
jom
```

Use of msbuild will  work but is not recommended as it gets into additional complexities to do with manually changing the build configurations.  For NMake and Jom, the Libaudioverse CMake scripts force a release build by default.  To test your build, try the following two commands:

```
examples\automation
utils\time_convolution default
```

The first should give audio output and the second should give a reasonable number of sources (certainly at least 100. If it says 30 or something comparably sad then this process is broken and you need to edit CMakeCache.txt to change the build type.  Also you should report it).

If you set up to build the manual, you may do so with either `nmake libaudioverse_docs` or `jom libaudioverse_docs` depending on your generator.

You now have all supported language bindings in a subdirectory `bindings` and the manual in subdirectory `documentation` as `libaudioverse_manual.html`.

##Linux

Linux supports both X86 and X64.  The compiler requirement is gcc 4.9 or later.

At the moment, the only supported audio output is ALSA.  This causes somewhat higher latency for Pulse users.  A Pulse backend will happen in the future.

You cannot currently reasonably run Libaudioverse on ARM.  It does not have Neon optimizations.  Unlike Windows, this configuration may compile, but it will be slow.  ARM platforms are not officially supported or tested and you are on your own if you opt to try it.

###Getting Dependencies

Some distros change the name of some packages, but it roughly looks like this:

```
apt-get update
sudo apt-get install gcc g++ make cmake python3 libboost-all-dev libsndfile-dev libasound2-dev pandoc python3,numpy python3-scipy python3-pip
sudo python3 --m pip install pycparser jinja2 wheel sphinx pyyaml pypandoc
```

There are two important points.  Python 3.4 or later must be available as `python3` and you must have gcc and g++ 4.9 or later.  This latter point is important for Ubuntu LTS, which uses a gcc that is too old.

The only package that seems to be named differently on different distros is libsndfile.  On debian, it's `libsndfile-dev`, but Ubuntu has it as `libsndfile1-dev`.


###Building

Roughly:

```
git clone http://github.com/camlorn/libaudioverse
cd libaudioverse
mkdir build
cd build
cmake ..
make
```

And testing:

```
./examples/automation
./utils/time_convolution default
```

As with Windows, the first should give audio output and the second should give at least 100 sources as an estimate (at least on Desktops).
