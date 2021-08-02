# Installation

The steps required to install DynAdjust on your system will depend upon your operating system, and whether or not you choose to build from source code or install pre-built binaries. In addition, installation may require the installation (or building) of one or more prerequisite applications that are external to DynAdjust but essential for its installation and use.

## Contents
  - [Installing from pre-built binaries](#installing-from-pre-built-binaries)
    - [Linux, Mac and Windows](#linux-mac-and-windows)
    - [Windows only](#windows-only)
  - [Building from source](#building-from-source)
    - [General requirements and prerequisites](#general-requirements-and-prerequisites)
    - [Linux and Mac](#linux-and-mac)
      - [1. Install prerequisites](#1-install-prerequisites)
      - [2. Build the source code](#2-build-the-source-code)
        - [2.1. Clone a fresh copy, build and install](#21-clone-a-fresh-copy-build-and-install)
        - [2.2. Build an existing copy](#22-build-an-existing-copy)
        - [2.3. Testing your changes and adding test scripts](#23-testing-your-changes-and-adding-test-scripts)
        - [2.4. Build Help](#24-build-help)
    - [Windows only](#windows-only-1)


## Installing from pre-built binaries

### Linux, Mac and Windows

The DynAdjust repository comes with a [Dockerfile](https://github.com/icsm-au/DynAdjust/blob/master/Dockerfile) which builds a DynAdjust docker image for the Linux environment each time changes are pushed to the main repository. This means if you have docker installed on your system, a DynAdjust image can be run on your system (whether Linux, Mac or Windows) within a virtual enviroment managed by docker.

To access the latest docker image, please visit the [DynAdjust repo on Docker Hub](https://hub.docker.com/r/icsm/dynadjust).

Alternatively, you can pull a DynAdjust docker image from your system via:

  ``` bash
  $ docker pull icsm/dynadjust
  ```

### Windows only

For each stable release, DynAdjust binaries are built using Microsoft Visual Studio 2017 and published on the [releases page](https://github.com/icsm-au/dynadjust/releases/latest).

To install DynAdjust on Windows:

1. Download the latest pre-built Windows (64-bit) binaries from the [releases page](https://github.com/icsm-au/dynadjust/releases/latest) to a dedicated folder (not the Desktop) on your system.  Add the location of this folder to your system's path environment variables so that DynAdjust can be executed from any location on your system.

2. Install the following (64-bit) libraries and dependencies:
   1. Apache Xerces-C++ XML Parser: <http://xerces.apache.org/xerces-c/download.cgi>
   2. Codesynthesis XSD XML Data Binding for C++: <https://www.codesynthesis.com/products/xsd/download.xhtml>
   3. Intel Math Kernel Library: <https://software.seek.intel.com/performance-libraries>
   4. Microsoft Visual C++ Redistributable for Visual Studio 2017: <https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads>, or <https://visualstudio.microsoft.com/downloads/> > _Other Tools and Frameworks_ > _Microsoft Visual C++ Redistributable for Visual Studio 2019_

## Building from source

The following build instructions are only needed if you would like to build DynAdjust yourself, rather than use pre-built binaries, or make changes to the source code and [contribute to the DynAdjust repository](../CONTRIBUTING.md).

### General requirements and prerequisites

- A C++14 compiler (e.g. gcc, Microsoft Visual Studio or Apple LLVM (clang))
- Boost C++ headers and libraries
- Apache Xerces C++ headers and libraries
- Codesynthesis XSD headers and libraries
- Intel Math Kernel Library (MKL) headers and libraries

### Linux and Mac

#### 1. Install prerequisites

To install the prerequisites:

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

  Executing the script without any options will cause the script to run in interactive mode, allowing you to choose several options for how the prerequisites (boost, xerces-c, xsd, mkl) are installed. 
  
  Your system-specific package manager will be used to install boost and mkl.
  
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

#### 2. Build the source code

There are two general approaches to building and installing DynAdjust:

  1. Cloning a fresh copy of DynAdjust from GitHub, and building
  2. Building DynAdjust from an existing copy of the source code, either a previous version or a local version you have modified.

##### 2.1. Clone a fresh copy, build and install

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
  
##### 2.2. Build an existing copy

To build and install DynAdjust from an existing copy of the source code, either from a previous version on GitHub or from a local copy that you have modified:

1. Create a suitable directory where DynAdjust is to be built.
2. Open the terminal and go to this directory.
3. Optionally, download the version you would like to build to this directory. All versions of the DynAdjust source code are available as `.tar` files.
4. Extract the `.tar` file in the current directory.
5. Change to the directory containing the source code (see note below).
6. Execute the `make_dynadjust_gcc.sh` shell script (located in the `./resources/` subdirectory) from the current directory with the "no clone" option (either `-c` or `--no-clone`).

> **Note:** The directory containing the source code will contain the subdirectories `dynadjust`, `resources`, `sampleData`, `ubuntu`, `.github` and `.vscode`.

For example, execute the following at the terminal (in the home directory) to get, build and install version 1.2.2:

  ``` bash
  $ mkdir dynadjust_local_copy
  $ cd ./dynadjust_local_copy/
  $ wget https://github.com/icsm-au/DynAdjust/archive/refs/tags/v1.2.2.tar.gz -O DynAdjust-1.2.2.tar.gz
  $ tar xzvf DynAdjust-1.2.2.tar.gz
  $ cd ./DynAdjust-1.2.2/
  $ ./resources/make_dynadjust_gcc.sh --no-clone
  ```

As with the previous build approach, executing `./resources/make_dynadjust_gcc.sh` without `--no-install` will build and install binaries to `/opt/dynadjust/gcc/x_x_x/`.

##### 2.3. Testing your changes and adding test scripts

If you have made changes to the source code and would like to build and test your changes, simply return to the source code directory and run the following:

``` bash
  $ cd ./DynAdjust-1.2.2/
  $ ./resources/make_dynadjust_gcc.sh --no-clone --test --no-install
  ```

This will automatically build a debug variant (produced by the `--debug` option), and will execute a range of tests using data contained in the [`./sampleData/`](../sampleData) directory.

If you would like to debug one of the programs (e.g. to debug `dnaadjust` using `gdb`), run the following:

``` bash
  $ ./resources/make_dynadjust_gcc.sh --no-clone --debug --no-install --binary adjust
  ```

This will build debug variants of both the `libdnaadjust.so` library and `dnaadjust` executable.  To assist with debugging the respective DynAdjust binaries, please refer to [`/.vscode/launch.json`](../.vscode/launch.json) for debug launch configuration settings for Microsoft Visual Studio Code.

If you have added a new feature and would like to test its functionality, please consider adding some test data to the `./sampleData/` directory and adding a test script to the list of tests in [`./CMakeLists.txt`](https://github.com/icsm-au/DynAdjust/blob/master/dynadjust/CMakeLists.txt#L167).

##### 2.4. Build Help

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




### Windows only

Please refer to the Windows compilation [instructions](https://github.com/icsm-au/DynAdjust/blob/master/resources/dynadjust-compilation-in-windows.pdf) for the steps to compile DynAdjust on Windows using Microsoft's freely available Visual Studio 2017 Community Edition.
