# Building DynAdjust from source code

The following build instructions are only needed if you would like to build DynAdjust yourself, rather than use [pre-built binaries](https://github.com/icsm-au/DynAdjust/releases), or make changes to the source code and [contribute to the DynAdjust repository](./CONTRIBUTING.md).

## Contents

- [General requirements and prerequisites](#general-requirements-and-prerequisites)
- [Windows only](#windows-only)
  - [1. Install Windows prerequisites](#1-install-windows-prerequisites)
    - [1.1 Install Microsoft Visual Studio 2022 Community Edition](#11-install-microsoft-visual-studio-2022-community-edition)
    - [1.2 Install Boost C++ headers and libraries](#12-install-boost-c-headers-and-libraries)
    - [1.3 Install CodeSynthesis XSD and Apache xerces-c headers and libraries](#13-install-codesynthesis-xsd-and-apache-xerces-c-headers-and-libraries)
    - [1.4 Install Intel oneAPI Math Kernel Library (MKL)](#14-install-intel-oneapi-math-kernel-library-mkl)
  - [2. Building Windows binaries in Visual Studio](#2-building-windows-binaries-in-visual-studio)
- [Linux and Mac](#linux-and-mac)
  - [1. Install prerequisites](#1-install-prerequisites)
  - [2. Build the source code](#2-build-the-source-code)
    - [2.1. Clone a fresh copy, build and install](#21-clone-a-fresh-copy-build-and-install)
    - [2.2. Build an existing copy](#22-build-an-existing-copy)
  - [3. Test your changes and add test scripts](#3-test-your-changes-and-add-test-scripts)
  - [4. Build Help](#4-build-help)


## General requirements and prerequisites

To build DynAdjust, the following prerequisites will be needed:

- A C++14 compiler (e.g. gcc, Microsoft Visual Studio or Apple LLVM (clang))
- Boost C++ headers and libraries
- Apache Xerces C++ headers and libraries
- Codesynthesis XSD headers and libraries
- Intel Math Kernel Library (MKL) headers and libraries

The way in which these prerequisites are installed will depend upon your operating system and will be discussed in the following sections.

## Windows only

Building DynAdjust on Windows requires two steps: (1) installation of prerequisite tools and (2) compilation via Visual Studio C++.

### 1. Install Windows prerequisites

#### 1.1 Install Microsoft Visual Studio 2022 Community Edition

Microsoft’s Visual Studio 2022 Community Edition is available from
https://visualstudio.microsoft.com/vs/community/

C++ is required for compiling all DynAdjust binaries.  MFC is required only for building `GeoidInt.exe` - Geoid Interpolation software with a (dialog-based) graphical user interface.

#### 1.2 Install Boost C++ headers and libraries

The supported versions of boost are 1.58.0 – 1.78.0. The headers and library sources are available from https://www.boost.org/users/download/

The boost libraries needed by DynAdjust include `filesystem`, `system`, `program_options`, `thread`, `date_time`, `math`, `timer`, `atomic` and `chrono`. These will need to be built from the boost C++ sources using Visual Studio 2022, and installed to a suitable folder on your machine.

Follow the instructions on the [Boost Website](https://www.boost.org/doc/libs/1_78_0/more/getting_started/windows.html#prepare-to-use-a-boost-library-binary) to build the boost libraries from source.  For example, the steps are: 

1. Download and extract boost to a suitable folder
2. Run `Bootstrap.bat` to build `b2`
3. Build the boost binaries using `b2`

For steps 2 and 3, run the following (assuming boost has been downloaded and unzipped):

``` bat
rem Start building boost
echo 
echo Building bootstrap.bat
echo

rem inside the root directory where boost was unzipped, change to tools\build\
cd .\tools\build

rem build b2 using VS 2022
call bootstrap.bat vc143

rem Directory to boost root
set boost_dir=boost_1_78_0

rem Store compiled libraries in directories corresponding to 64-bit and 32-bit.
set stage_64=C:\Data\boost\%boost_dir%\lib\x64
set stage_32=C:\Data\boost\%boost_dir%\lib\Win32
set headers=C:\Data\boost\%boost_dir%\include

rem Number of cores to use when building boost
set cores=%NUMBER_OF_PROCESSORS%

rem Visual Studio 2022
set msvcver=msvc-14.3

rem change to the root directory, copy b2 to root
cd ..\..
copy .\tools\build\b2.exe .\

rem Static libraries (64 bit)
echo Building %boost_dir% (64-bit) with %cores% cores using toolset %msvcver%.
echo Destination directory is %stage_64%
b2 -j%cores% toolset=%msvcver% address-model=64 architecture=x86 link=static,shared threading=multi runtime-link=shared --build-type=minimal stage --stagedir=%stage_64%

rem move contents of %stage_64%\lib to %stage_64%
move %stage_64%\lib\* %stage_64%\
del %stage_64%\lib

rem Static libraries (32 bit)
echo Building %boost_dir% (32-bit) with %cores% cores using toolset %msvcver%.
echo Destination directory is %stage_32%
b2 -j%cores% toolset=%msvcver% address-model=32 architecture=x86 link=static,shared threading=multi runtime-link=shared --build-type=minimal stage --stagedir=%stage_32%

rem move contents of %stage_32%\lib to %stage_32%
move %stage_32%\lib\* %stage_32%\
rmdir /S /Q %stage_32%\lib

rem make include folder (C:\Data\boost\%boost_dir%\include) and move headers (boost folder)
md %headers%
move .\boost %headers% 
```

The DynAdjust repository includes a Visual Studio property sheet (`dynadjust.props`), which allows you to set the folder paths to the boost header files and libraries on your machine. The boost header and library folder paths are saved within `dynadjust.props` as _User Macros_, named **BoostIncludeDir** and **BoostLibDir**, and are referenced throughout the solution’s project properties. Saving thes paths in a global property sheet
provides a convenient way to reference custom boost C++ file paths across the entire solution without having to change individual property page for each project.

By default, the boost paths are set as follows. Change these to match the location of the boost header files and libraries on your machine, making sure that `\lib\` contains two folders named `x64` and `Win32` if you need to build 64-bit and 32-bit binaries respectively.

- **BoostIncludeDir:**  `C:\Data\boost\boost_1_78_0\include\`
- **BoostLibDir:** `C:\Data\boost\boost_1_78_0\lib\$(Platform)`

#### 1.3 Install CodeSynthesis XSD and Apache xerces-c headers and libraries

DynAdjust requires CodeSynthesis XSD (version 4.0) headers and Apache xerces-c headers and libraries. The x86 and x64 Windows dependencies are available as a bundle via:
https://www.codesynthesis.com/products/xsd/download.xhtml

If the default installation path (`C:\Program Files (x86)\CodeSynthesis XSD 4.0`) is used during setup, the XSD and xerces-c paths will be correctly referenced via the Visual Studio property sheet `dynadjust.props`. As with the boost paths, the header and library folder paths for XSD and xerces-c are saved using _User Macros_, named **XsdIncludeDir**, **XsdLibDir_x64**, and **XsdLibDir_Win32**:

- **XsdIncludeDir**: `C:\Program Files (x86)\CodeSynthesis XSD 4.0\include`
- **XsdLibDir_x64**: `C:\Program Files (x86)\CodeSynthesis XSD 4.0\lib64\vc-12.0`
- **XsdLibDir_Win32**: `C:\Program Files (x86)\CodeSynthesis XSD 4.0\lib\vc-12.0`

If an alternative installation path is chosen, change the User Macros accordingly.

#### 1.4 Install Intel oneAPI Math Kernel Library (MKL)

DynAdjust requires Intel’s oneAPI MKL and TBB libraries. A free version of oneAPI is available from:
https://www.intel.com/content/www/us/en/developer/tools/oneapi/onemkl.html

With Visual Studio 2022 already installed, the Intel oneAPI installer will automatically enable integration into the Visual Studio 2022 IDE. This means that the oneAPI MKL and TBB libraries and headers will be automatically referenced upon compiling DynAdjust without modification.

> **Note:** The entire oneAPI toolkit is quite large – choose MKL installation only for a minimum build set up.

### 2. Building Windows binaries in Visual Studio

DynAdjust is comprised of several executables and dependent dynamic link libraries (DLL), each of which is managed and configured as a VC++ project.  All projects are contained within a single solution file `dynadjust_x_xx_xx.sln`.  

The VC++ project for each executable is named using the convention `dna<program-name>wrapper`, except for the main program
`dynadjust`. Upon compilation, these projects will create executables named `<program-name>.exe`.
Each executable named `<program-name>.exe` is dependent on a DLL named `dna<program-name>.dll`. The DLLs must and will be compiled first before compiling the executables.

The executable projects and their dependent DLLs are listed below:
- dnaadjustwrapper
  - dnaadjust
- dnageoidwrapper
  - dnageoid
- dnaimportwrapper
  - dnaimport
- dnaplotwrapper
  - dnaplot
- dnareftranwrapper
  - dnareftran
- dnasegmentwrapper
  - dnasegment
- dynadjust (no project dependencies, but requires all preceding projects to be built for normal execution behaviour)
- GeoidInt
  - dnageoid

For each VC++ project, four build configurations have been created:
1. Debug Win32
2. Release Win32
3. Debug x64
4. Release x64

The project properties pages for each executable and DLL project make use of User Macros that simplify the creation of settings for the four configurations.

Given that many functions are shared throughout the suite of executables and DLLs, the DynAdjust solution makes extensive use of precompiled headers to simplify and speed up compile time.

## Linux and Mac

Building DynAdjust on Linux or Mac requires two steps: (1) installation of prerequisite tools and (2) execution of a gcc build script using cmake.

### 1. Install prerequisites

The specific Linux/Mac prerequisites for building DynaAdjust include the following software:

- gcc-c++ (with std C++ 14 support), with a compatible version of make
- cmake (minimum v3.13)
- Intel oneAPI Math Kernel Library (MKL)
- boost-devel (minimum v1.58.0. v1.78.0 preferred)
- xerces-c (3.1.4)
- xsd (4.0)
- git (if cloning copies from the GitHub repository)
- bzip2 (required for building xerces-c from source)
- wget (for accessing Intel gpg keys, xsd, xerces-c)

> **Note:** If all prerequisites are installed, they do not need to be re-installed. In this instance, skip to step [2. Build the source code](#2-build-the-source-code).

These prerequisites can be installed manually, or via a convenient installation script:

1. Download the shell script [`install_dynadjust_prerequisites.sh`](https://github.com/icsm-au/DynAdjust/raw/master/resources/install_dynadjust_prerequisites.sh) (from the [resources](https://github.com/icsm-au/DynAdjust/tree/master/resources) folder) to a suitable directory. It does not matter where this script is executed from.
2. Open the terminal and go to the directory where this shell script was saved.
3. Execute this script as follows:

    ``` bash
    $ ./install_dynadjust_prerequisites.sh
    ```

> **Note:** To execute the prerequisites shell script, run the script either as:
> ``` bash
> $ bash ./install_dynadjust_prerequisites.sh
> ```
> or change the execute permission on the script and run as:
> ``` bash
> $ chmod +x ./install_dynadjust_prerequisites.sh
> $ ./install_dynadjust_prerequisites.sh
> ```

  Executing the script without any options will cause the script to run in interactive mode, allowing you to choose several options for how the prerequisites (boost, xerces-c, xsd, oneAPI mkl) are installed. 
  
  Your system-specific package manager will be used to install boost and oneAPI mkl.
  
  Options are provided for installing xerces-c and xsd prerequisites via your system-specific package manager or downloading and building from source.
  
  In the event one or more prerequistites are already installed, you will have the option to skip installation.
  
  The script will attempt to identify your Linux distribution or Mac flavour, and will select the system-specific package manager accordingly. If your distribution is not supported, you can attempt to select your distribution using the `--distro` option. In this case, you will be required to select the base distro (e.g. Mint is based on Ubuntu). If this fails to work, please submit an issue at: https://github.com/icsm-au/DynAdjust/issues including the script's message and your distribution.
  
  To view the alternative options for `install_dynadjust_prerequisites.sh`, run the script with help option:

  ``` bash
  $ ./install_dynadjust_prerequisites.sh -h
  ```

  This will display the script's help message:

  ``` bash
  usage: install_dynadjust_prerequisites.sh [options]
  
  options:
    -d [ --distro ] arg    The linux distribution. Recognised distros include:
                             - CentOS Linux
                             - Debian
                             - Fedora
                             - openSUSE
                             - Red Hat Enterprise Linux
                             - Ubuntu
                           If not provided, I will try to get the distro from /etc/elease.
    -m [ --mode ] arg      Mode of installing prerequisites:
                             0: interactive (default)
                             1: package manager
                             2: build from source
                             3: skip
    -h [ --help ]          Prints this help message
  
  example: install_dynadjust_prerequisites.sh -d Ubuntu -m 1
  ```

### 2. Build the source code

[![cmake workflow](https://img.shields.io/github/workflow/status/icsm-au/dynadjust/Build%20release?label=cmake%20workflow)](https://github.com/icsm-au/DynAdjust/actions/workflows/cmake_release.yml)

There are two general approaches to building and installing DynAdjust:

  1. Cloning a fresh copy of DynAdjust from GitHub, and building
  2. Building DynAdjust from an existing copy of the source code, either a previous version or a local version you have modified.

#### 2.1. Clone a fresh copy, build and install

To build and install DynAdjust to `/opt/dynadjust/gcc` using the latest source code from the main branch on GitHub:

1. Create a suitable directory where DynAdjust is to be built.
2. Open the terminal and go to this directory.
4. Download the shell script [`make_dynadjust_gcc.sh`](https://github.com/icsm-au/DynAdjust/raw/master/resources/make_dynadjust_gcc.sh) into this directory.
3. Execute the `make_dynadjust_gcc.sh` shell script.

For example, execute the following at the terminal (in the home directory):

  ``` bash
  $ mkdir dynadjust_latest
  $ cd ./dynadjust_latest
  $ wget https://github.com/icsm-au/DynAdjust/blob/master/resources/make_dynadjust_gcc.sh
  $ chmod +x ./make_dynadjust_gcc.sh
  $ ./make_dynadjust_gcc.sh
  ```

After a successful build, binaries will be located in the `./dynadjust/build-gcc/...` directories. Superuser privileges will be required to install the binaries to `/opt/dynadjust/gcc/x_x_x/`, in which case you will be prompted for the superuser's password. Symbolic links to the binaries (installed to `/opt/dynadjust/`) will be created in your `~/bin` directory, enabling you to execute DynAdjust from anywhere on your system.

If you do not want to install DynAdjust to `/opt/dynadjust/gcc/x_x_x/`, run the script with the "no install" option (either `-n` or `--no-install`):

  ``` bash
  $ ./make_dynadjust_gcc.sh --no-install
  ```
  
#### 2.2. Build an existing copy

[![github releases](https://img.shields.io/github/v/release/icsm-au/DynAdjust)](https://github.com/icsm-au/DynAdjust/releases)

To build and install DynAdjust from an existing copy of the source code, either from a previous version on GitHub or from a local copy that you have modified:

1. Create a suitable directory where DynAdjust is to be built.
2. Open the terminal and go to this directory.
3. Optionally, download the version you would like to build to this directory. All versions of the DynAdjust source code are available as `.tar` files.
4. Extract the `.tar` file in the current directory.
5. Change to the directory containing the source code (see note below).
6. Execute the `make_dynadjust_gcc.sh` shell script (located in the `./resources/` subdirectory) from the current directory with the "no clone" option (either `-c` or `--no-clone`).

> **Note:** The directory containing the source code will contain the subdirectories `dynadjust`, `resources`, `sampleData`, `ubuntu`, `.github` and `.vscode`.

For example, execute the following at the terminal (in the home directory) to get, build and install version 1.2.4:

  ``` bash
  $ mkdir dynadjust_local_copy
  $ cd ./dynadjust_local_copy/
  $ wget https://github.com/icsm-au/DynAdjust/archive/refs/tags/v1.2.4.tar.gz -O DynAdjust-1.2.4.tar.gz
  $ tar xzvf DynAdjust-1.2.4.tar.gz
  $ cd ./DynAdjust-1.2.4/
  $ ./resources/make_dynadjust_gcc.sh --no-clone
  ```

As with the previous build approach, executing `./resources/make_dynadjust_gcc.sh` without `--no-install` will build and install binaries to `/opt/dynadjust/gcc/x_x_x/`.

### 3. Test your changes and add test scripts

[![github issues](https://img.shields.io/github/issues/icsm-au/DynAdjust.svg)](https://github.com/icsm-au/DynAdjust/issues)
[![cmake tests](https://img.shields.io/github/workflow/status/icsm-au/dynadjust/Build,%20test%20and%20code%20coverage?label=cmake%20tests)](https://github.com/icsm-au/DynAdjust/actions/workflows/test_coverage.yml)
[![codacy badge](https://img.shields.io/codacy/grade/a3944cda0c72445f8a13b1f82b64f714)](https://app.codacy.com/gh/icsm-au/DynAdjust/dashboard)
[![coveralls status](https://img.shields.io/coveralls/github/icsm-au/DynAdjust)](https://coveralls.io/github/icsm-au/DynAdjust)
[![codecov status](https://img.shields.io/codecov/c/github/icsm-au/dynadjust)](https://codecov.io/gh/icsm-au/DynAdjust)

If you have made changes to the source code and would like to build and test your changes, simply return to the source code directory and run the following:

``` bash
  $ cd ./DynAdjust-1.2.4/
  $ ./resources/make_dynadjust_gcc.sh --no-clone --test --no-install
  ```

This will automatically build a debug variant (produced by the `--debug` option), and will execute a range of tests using data contained in the [`./sampleData/`](../sampleData) directory.

If you would like to debug one of the programs (e.g. to debug `dnaadjust` using `gdb`), run the following:

``` bash
  $ ./resources/make_dynadjust_gcc.sh --no-clone --debug --no-install --binary adjust
  ```

This will build debug variants of both the `libdnaadjust.so` library and `dnaadjust` executable.  

To assist with building and debugging the respective DynAdjust binaries using Microsoft Visual Studio Code, please visit the [`/.vscode/`](../.vscode/) folder for some example debug and launch configuration files.

If you have added a new feature and would like to test its functionality, please consider adding some test data to the `./sampleData/` directory and adding a test script to the list of tests in [`./CMakeLists.txt`](https://github.com/icsm-au/DynAdjust/blob/master/dynadjust/CMakeLists.txt#L167).

### 4. Build Help

To view the available options for building DynAdjust, execute `./make_dynadjust_gcc.sh` with the "help" option:

  ``` bash
  $ ./make_dynadjust_gcc.sh -h
  ```
  
This will display the script's help message:

  ``` bash
  usage: make_dynadjust_gcc.sh [options]
  
  options:
    -a [ --auto ]        Run automatically with no user interaction.
    -b [ --binary ] arg  Build a specific binary (e.g. "dnaimport" or "dnaadjustwrapper").
                         By default, "all" binaries are built.
    -c [ --no-clone ]    By default, the latest version will be cloned from GitHub
                         into the current directory, using:
                           git clone https://github.com/icsm-au/DynAdjust.git
                         Provide this option if building source from a local copy, e.g.:
                           $ wget https://github.com/icsm-au/DynAdjust/archive/refs/tags/v1.1.  tar.gz -O DynAdjust-1.1.0.tar.gz
                           $ tar xzvf DynAdjust-1.1.0.tar.gz
                           $ cd DynAdjust-1.1.0/
                           $ bash ./resources/make_dynadjust_gc.sh (this script)
    -d [ --debug ]       Compile debug version.
    -n [ --no-install ]  Do not install binaries.
    -t [ --test ]        Run cmake tests.
    -h [ --help ]        Prints this help message.
    
  examples:
    make_dynadjust_gcc.sh --auto --no-clone --test --no-install
  ```

