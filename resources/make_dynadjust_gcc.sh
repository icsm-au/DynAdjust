#! /bin/bash

################################################################################
#
# This script clones the latest version, configures the environment, then
# builds and installs DynAdjust using:
#   gcc 10.1.1
#   boost 1.69.0
#   xerces-c 3.1.4
#
################################################################################

# set defaults
_script="make_dynadjust_gcc.sh"
_auto=0 # default option is to ask for user input
_debug=0 # default option is to build release variant
_clone=0 # default option is to clone afresh
_test=0 # default option is to skip cmake tests
_install=0 # default option is to install binaries

#################################################################################
# Common functions
#
# display example message
function example {
    echo -e "examples:"
	echo -e "  $_script --auto --do-not-clone --test --no-install"
}

# display usage message
function usage {
    echo -e "usage: $_script [options]\n"
}

# display help message (calls usage and example)
function help {
    echo ""
    usage
    echo -e "options:"
    echo -e "  -a [ --auto ]          Run automatically with no user interaction."
    echo -e "  -d [ --debug ]         Compile debug version."
    echo -e "  -c [ --do-not-clone ]  By default, the latest version will be cloned from GitHub."
    echo -e "                         Set this option if a clone does not need to be made."
    echo -e "  -t [ --test ]          Run cmake tests."
    echo -e "  -n [ --no-install ]    Do not install binaries."
    echo -e "  -h [ --help ]          Prints this help message.\n"
    example
    echo ""
}
#################################################################################

# get argument parameters
while [[ "$1" != "" ]];
do
   case $1 in
   -a  | --auto ) 	shift
   				  	_auto=1 # run automatically (or silently)
			    	;;
   -d  | --debug )  shift
   					_debug=1 # compile debug variant
			        ;;
   -c  | --do-not-clone )  shift
   					_clone=1 # do not clone from GitHub
			        ;;
   -t  | --test )   shift
   					_test=1 # run tests
			        ;;
   -n  | --no-install ) shift
   					_install=1 # do not install binaries
			        ;;
   -h   | --help )  help
                    exit
                    ;;
   *)                     
                    echo -e "\n$_script: illegal option $1"
                    help
					exit 1 # error
                    ;;
    esac
done

echo -e "\n==========================================================================="
echo -e "DynAdjust build configuration options..."

if [[ $_debug -eq 1 || $_test -eq 1 ]]; then
	echo -e " - debug variant."
else
	echo -e " - release variant."
fi

if [[ $_auto -eq 1 ]]; then
	echo -e " - build automatically (no user input)."
else
	echo -e " - run interactively (ask for user input)."
fi

if [[ $_clone -eq 1 ]]; then
	echo -e " - do not clone a fresh copy from GitHub."
else
	echo -e " - clone a fresh copy from GitHub."
fi

if [[ $_test -eq 1 ]]; then
	echo -e " - run tests."
fi

if [[ $_install -eq 1 ]]; then
	echo -e " - do not install."
else
	echo -e " - install binaries to /opt/dynadjust/gcc/x_x_x."
fi


# get current directory
_cwd="$PWD"
# set dynadjust clone dir
_clone_dir="$_cwd/DynAdjust"
# set dynadjust root dir
_root_dir="$_clone_dir/dynadjust"
# set dynadjust root dir
_test_dir="$_clone_dir/sampleData"
# set build dir
_build_dir="$_root_dir/build_gcc"
# set clone url
_clone_url="https://github.com/icsm-au/DynAdjust.git"

# usr bin directory
BIN_FOLDER="~/bin"
eval BIN_FOLDER_FULLPATH="$BIN_FOLDER"

# opt installation folder
OPT_DYNADJUST_PATH=/opt/dynadjust
OPT_DYNADJUST_GCC_PATH=/opt/dynadjust/gcc
DYNADJUST_INSTALL_PATH=/opt/dynadjust/gcc/1_0_3

# version info
_version="1.0.3"

echo -e "\n==========================================================================="
echo -e "Build and installation of DynAdjust $_version...\n"
if [[ $_clone -eq 0 ]]; then
	echo "Repository settings:"
	echo "  Git repo:      $_clone_url"
fi
echo "Build settings:"
echo "  Current dir:   $_cwd"
if [[ $_clone -eq 0 ]]; then
	echo "  Clone dir:     $_clone_dir"
fi
echo "  Build dir:     $_build_dir"

if [[ $_install -eq 0 ]]; then
	echo "Installation settings:"
	echo "  Install dir:   $DYNADJUST_INSTALL_PATH"
	echo "  User bin dir:  $BIN_FOLDER_FULLPATH"
fi

if [[ $_test -eq 1 ]]; then
	echo "Test settings:"
	echo "  Test dir:      $_test_dir"
fi

#
# determine whether user needs prompting
case ${_auto} in
    0) # perform interactive build
        echo " "
		read -r -p "Is this ok [Y/n]: " response;;
    *) # build without asking
        response="y";;
esac

if [[ "$response" =~ ^([nN][oO]|[nN])$ ]]
then    
    exit
else
    echo " "
fi

# INSTALL DYNADJUST
# 1. create install dirs:
if [[ $_install -eq 0 ]]; then
	if [[ ! -d "$BIN_FOLDER_FULLPATH" ]]; then
		echo " "
		echo "Making $BIN_FOLDER_FULLPATH"
		mkdir "$BIN_FOLDER_FULLPATH"
	fi
fi

# 2. clone from GitHub:
if [[ $_clone -eq 0 ]]; then
	echo " "
	echo "Cloning DynAdjust..."
	git clone "$_clone_url" || echo -e "Okay, let's assume we already have a previously cloned version.\n"
fi

if [[ -d "$_build_dir" ]]; then
    echo "Cleaning out directory $_build_dir"
    cd "$_build_dir"
    rm -rf CMakeCache.txt CMakeFiles cmake_install.cmake dynadjust Makefile
    cd "$_cwd"
else
    echo "Creating new directory $_build_dir"
    mkdir "$_build_dir"
fi

cd "$_build_dir"

# 3. copy files:
echo "Copying Find*.cmake files to build directory..."
cp ../FindXercesC.cmake ./
cp ../FindMKL.cmake ./
cp ../FindXSD.cmake ./

REL_BUILD_TYPE="Release"
DBG_BUILD_TYPE="Debug"
THIS_BUILD_TYPE=$REL_BUILD_TYPE

# 4. build:
# Force build type to Debug for --debug or --test options
if [[ $_debug -eq 1 || $_test -eq 1 ]]; then
	# debug
	THIS_BUILD_TYPE=$DBG_BUILD_TYPE
fi

echo " "
echo "Building DynaNet ($THIS_BUILD_TYPE)..."
echo " "

gcc_version=$(gcc -v 2>&1 | tail -1 | awk '{print $1 " " $2 " " $3}')

echo $gcc_version
echo " "

#
# determine whether to prepare cmake files with testing or not
case ${_test} in
    0) # skip tests
        echo -e "cmake -DBUILD_TESTING=OFF -DCMAKE_BUILD_TYPE=$THIS_BUILD_TYPE ..\n"
		cmake -DBUILD_TESTING="OFF" -DCMAKE_BUILD_TYPE="$THIS_BUILD_TYPE" .. || exit 1;;
    *) # run cmake tests with code coverage
        echo -e "cmake -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=$THIS_BUILD_TYPE ..\n"
		cmake -DBUILD_TESTING="ON" -DCMAKE_BUILD_TYPE="$THIS_BUILD_TYPE" .. || exit 1;;
esac

echo -e "\n==========================================================================="
echo -e "Building DynAdjust $_version...\n"

make -j $(nproc) || exit 1

echo " "

case ${_test} in
    1) # run cmake tests
		echo -e "==========================================================================="
		echo -e "Testing DynAdjust $_version...\n"
        make CTEST_OUTPUT_ON_FAILURE=1 test;;
esac

#
# determine if user needs prompting
case ${_auto} in
    0) # install binaries
        echo " "
		read -r -p "Install DynAdjust to $OPT_DYNADJUST_PATH [Y/n]: " optresponse;;
    *) # proceed without asking
        optresponse="y"
		# set install option
		if [[ $_install -eq 1 ]]; then
			optresponse="n"
		fi
		;;
esac

if [[ "$optresponse" =~ ^([nN][oO]|[nN])$ ]]
then    
    echo " "
else

    _lib_ext="so"

	# 1 GET OS, DISTRO AND TOOLS
	if [[ "$OSTYPE" == "darwin"* ]]; then
		# Mac OSX
		_lib_ext="dylib"
	fi

	if [[ ! -d $OPT_DYNADJUST_PATH ]]; then
		sudo mkdir $OPT_DYNADJUST_PATH
	fi

	if [[ ! -d $OPT_DYNADJUST_GCC_PATH ]]; then
		sudo mkdir $OPT_DYNADJUST_GCC_PATH
	fi

	if [[ ! -d $DYNADJUST_INSTALL_PATH ]]; then
		sudo mkdir $DYNADJUST_INSTALL_PATH
	fi

	echo "Copying libraries and binaries to $DYNADJUST_INSTALL_PATH ..."

	if [[ -e "./dynadjust/dynadjust/dynadjust" ]]; then
		sudo cp ./dynadjust/dynadjust/dynadjust "$DYNADJUST_INSTALL_PATH/"
		ln -sf "$DYNADJUST_INSTALL_PATH/dynadjust" "$BIN_FOLDER_FULLPATH/dynadjust"
		echo " - dynadjust"
	fi

	if [[ -e "./dynadjust/dnaadjustwrapper/dnaadjust" ]]; then
		sudo cp ./dynadjust/dnaadjust/libdnaadjust.$_lib_ext "$DYNADJUST_INSTALL_PATH/"
		sudo cp ./dynadjust/dnaadjustwrapper/dnaadjust "$DYNADJUST_INSTALL_PATH/"
		ln -sf "$DYNADJUST_INSTALL_PATH/dnaadjust" "$BIN_FOLDER_FULLPATH/dnaadjust"
		ln -sf "$DYNADJUST_INSTALL_PATH/libdnaadjust.$_lib_ext"  "$BIN_FOLDER_FULLPATH/libdnaadjust.$_lib_ext"
		echo " - dnaadjust, libdnaadjust.$_lib_ext"
	fi

	if [[ -e "./dynadjust/dnaimportwrapper/dnaimport" ]]; then
		sudo cp ./dynadjust/dnaimport/libdnaimport.$_lib_ext "$DYNADJUST_INSTALL_PATH/"
		sudo cp ./dynadjust/dnaimportwrapper/dnaimport "$DYNADJUST_INSTALL_PATH/"
		ln -sf "$DYNADJUST_INSTALL_PATH/dnaimport" "$BIN_FOLDER_FULLPATH/dnaimport"
		ln -sf "$DYNADJUST_INSTALL_PATH/libdnaimport.$_lib_ext"  "$BIN_FOLDER_FULLPATH/libdnaimport.$_lib_ext"
		echo " - dnaimport, libdnaimport.$_lib_ext"
	fi

	if [[ -e "./dynadjust/dnareftranwrapper/dnareftran" ]]; then
		sudo cp ./dynadjust/dnareftran/libdnareftran.$_lib_ext "$DYNADJUST_INSTALL_PATH/"
		sudo cp ./dynadjust/dnareftranwrapper/dnareftran "$DYNADJUST_INSTALL_PATH/"
		ln -sf "$DYNADJUST_INSTALL_PATH/dnareftran" "$BIN_FOLDER_FULLPATH/dnareftran"
		ln -sf "$DYNADJUST_INSTALL_PATH/libdnareftran.$_lib_ext" "$BIN_FOLDER_FULLPATH/libdnareftran.$_lib_ext"
		echo " - dnareftran, libdnareftran.$_lib_ext"
	fi

	if [[ -e "./dynadjust/dnageoidwrapper/dnageoid" ]]; then
		sudo cp ./dynadjust/dnageoid/libdnageoid.$_lib_ext "$DYNADJUST_INSTALL_PATH/"
		sudo cp ./dynadjust/dnageoidwrapper/dnageoid "$DYNADJUST_INSTALL_PATH/"
		ln -sf "$DYNADJUST_INSTALL_PATH/dnageoid" "$BIN_FOLDER_FULLPATH/dnageoid"
		ln -sf "$DYNADJUST_INSTALL_PATH/libdnageoid.$_lib_ext"  "$BIN_FOLDER_FULLPATH/libdnageoid.$_lib_ext"
		echo " - dnageoid, libdnageoid.$_lib_ext"
	fi

	if [[ -e "./dynadjust/dnasegmentwrapper/dnasegment" ]]; then
		sudo cp ./dynadjust/dnasegment/libdnasegment.$_lib_ext "$DYNADJUST_INSTALL_PATH/"
		sudo cp ./dynadjust/dnasegmentwrapper/dnasegment "$DYNADJUST_INSTALL_PATH/"
		ln -sf "$DYNADJUST_INSTALL_PATH/dnasegment" "$BIN_FOLDER_FULLPATH/dnasegment"
		ln -sf "$DYNADJUST_INSTALL_PATH/libdnasegment.$_lib_ext"  "$BIN_FOLDER_FULLPATH/libdnasegment.$_lib_ext"
		echo " - dnasegment, libdnasegment.$_lib_ext"
	fi

	if [[ -e "./dynadjust/dnaplotwrapper/dnaplot" ]]; then
		sudo cp ./dynadjust/dnaplot/libdnaplot.$_lib_ext "$DYNADJUST_INSTALL_PATH/"
		sudo cp ./dynadjust/dnaplotwrapper/dnaplot "$DYNADJUST_INSTALL_PATH/"
		ln -sf "$DYNADJUST_INSTALL_PATH/dnaplot" "$BIN_FOLDER_FULLPATH/dnaplot"
		ln -sf "$DYNADJUST_INSTALL_PATH/libdnaplot.$_lib_ext" "$BIN_FOLDER_FULLPATH/libdnaplot.$_lib_ext"
		echo " - dnaplot, libdnaplot.$_lib_ext"
	fi

	echo "Creating symbolic links to libraries and binaries in $DYNADJUST_INSTALL_PATH ..."
	sudo ln -sf "$DYNADJUST_INSTALL_PATH/libdnaimport.$_lib_ext" /opt/dynadjust/libdnaimport.$_lib_ext
	sudo ln -sf "$DYNADJUST_INSTALL_PATH/libdnareftran.$_lib_ext" /opt/dynadjust/libdnareftran.$_lib_ext
	sudo ln -sf "$DYNADJUST_INSTALL_PATH/libdnageoid.$_lib_ext" /opt/dynadjust/libdnageoid.$_lib_ext
	sudo ln -sf "$DYNADJUST_INSTALL_PATH/libdnasegment.$_lib_ext" /opt/dynadjust/libdnasegment.$_lib_ext 
	sudo ln -sf "$DYNADJUST_INSTALL_PATH/libdnaadjust.$_lib_ext" /opt/dynadjust/libdnaadjust.$_lib_ext 
	sudo ln -sf "$DYNADJUST_INSTALL_PATH/libdnaplot.$_lib_ext" /opt/dynadjust/libdnaplot.$_lib_ext 

	sudo ln -sf "$DYNADJUST_INSTALL_PATH/dnaimport" /opt/dynadjust/dnaimport
	sudo ln -sf "$DYNADJUST_INSTALL_PATH/dnareftran" /opt/dynadjust/dnareftran
	sudo ln -sf "$DYNADJUST_INSTALL_PATH/dnageoid" /opt/dynadjust/dnageoid
	sudo ln -sf "$DYNADJUST_INSTALL_PATH/dnasegment" /opt/dynadjust/dnasegment
	sudo ln -sf "$DYNADJUST_INSTALL_PATH/dnaadjust" /opt/dynadjust/dnaadjust
	sudo ln -sf "$DYNADJUST_INSTALL_PATH/dnaplot" /opt/dynadjust/dnaplot
	sudo ln -sf "$DYNADJUST_INSTALL_PATH/dynadjust" /opt/dynadjust/dynadjust

fi

echo "Done."
echo " "

if [[ $_install -eq 0 ]]; then
	echo "Don't forget to add the bin directory to path in ~/.bash_profile"
	echo "For example:"
	echo "    EXPORT PATH=$PATH:$HOME/bin"
	echo " "
fi
