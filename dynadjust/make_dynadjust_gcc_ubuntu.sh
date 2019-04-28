#! /bin/bash

################################################################################
#
# This script configures the environment and installs DynAdjust using:
#   gcc 7.2.1
#   boost 1.58
#   xerces-c 3.1.4
#
################################################################################

# Needed in order to find boost installation
export BOOST_ROOT=/opt/boost/gcc/1.58

if [ -d "./build-gcc" ]; then
    cd ./build-gcc
    rm -rf CMakeCache.txt CMakeFiles cmake_install.cmake dynadjust Makefile
    make distclean
else
    mkdir ./build-gcc
    cd ./build-gcc
fi

cp ../FindXercesC_ubunutu.cmake ./
cp ../FindMKL_ubuntu.cmake ./
cp ../FindXSD.cmake ./

REL_BUILD_TYPE="Release"
DBG_BUILD_TYPE="Debug"
THIS_BUILD_TYPE=$REL_BUILD_TYPE

# test argument for build type
if [ "$#" -lt 1 ]; 
then
	echo "+ No output specified.  Building $THIS_BUILD_TYPE by default...";

elif [ "$1" == "debug" -o "$1" == "Debug" ]; 
then
    THIS_BUILD_TYPE="Debug"

elif [ "$1" == "release" -o "$1" == "Release" ];
then
    THIS_BUILD_TYPE="Release"
fi

echo ""
echo "Building DynaNet ($THIS_BUILD_TYPE)..."
echo ""

gcc_version=$(gcc -v 2>&1 | tail -1 | awk '{print $1 " " $2 " " $3}')

echo $gcc_version
echo ""

# copy CMakeLists

if [ "$THIS_BUILD_TYPE" == "Debug" ]; 
then
	cp ../CMakeLists.debug.txt ../CMakeLists.txt
	cp ../dynadjust/dnaadjust/CMakeLists.debug.txt ../dynadjust/dnaadjust/CMakeLists.txt
	cp ../dynadjust/dnaadjustwrapper/CMakeLists.debug.txt ../dynadjust/dnaadjustwrapper/CMakeLists.txt
	cp ../dynadjust/dnageoid/CMakeLists.debug.txt ../dynadjust/dnageoid/CMakeLists.txt
	cp ../dynadjust/dnageoidwrapper/CMakeLists.debug.txt ../dynadjust/dnageoidwrapper/CMakeLists.txt
	cp ../dynadjust/dnaimport/CMakeLists.debug.txt ../dynadjust/dnaimport/CMakeLists.txt
	cp ../dynadjust/dnaimportwrapper/CMakeLists.debug.txt ../dynadjust/dnaimportwrapper/CMakeLists.txt
	cp ../dynadjust/dnaplot/CMakeLists.debug.txt ../dynadjust/dnaplot/CMakeLists.txt
	cp ../dynadjust/dnaplotwrapper/CMakeLists.debug.txt ../dynadjust/dnaplotwrapper/CMakeLists.txt
	cp ../dynadjust/dnareftran/CMakeLists.debug.txt ../dynadjust/dnareftran/CMakeLists.txt
	cp ../dynadjust/dnareftranwrapper/CMakeLists.debug.txt ../dynadjust/dnareftranwrapper/CMakeLists.txt
	cp ../dynadjust/dnasegment/CMakeLists.debug.txt ../dynadjust/dnasegment/CMakeLists.txt
	cp ../dynadjust/dnasegmentwrapper/CMakeLists.debug.txt ../dynadjust/dnasegmentwrapper/CMakeLists.txt
	cp ../dynadjust/dynadjust/CMakeLists.debug.txt ../dynadjust/dynadjust/CMakeLists.txt

elif [ "$THIS_BUILD_TYPE" == "Release" ];
then
	cp ../CMakeLists.release.txt ../CMakeLists.txt
	cp ../dynadjust/dnaadjust/CMakeLists.release.txt ../dynadjust/dnaadjust/CMakeLists.txt
	cp ../dynadjust/dnaadjustwrapper/CMakeLists.release.txt ../dynadjust/dnaadjustwrapper/CMakeLists.txt
	cp ../dynadjust/dnageoid/CMakeLists.release.txt ../dynadjust/dnageoid/CMakeLists.txt
	cp ../dynadjust/dnageoidwrapper/CMakeLists.release.txt ../dynadjust/dnageoidwrapper/CMakeLists.txt
	cp ../dynadjust/dnaimport/CMakeLists.release.txt ../dynadjust/dnaimport/CMakeLists.txt
	cp ../dynadjust/dnaimportwrapper/CMakeLists.release.txt ../dynadjust/dnaimportwrapper/CMakeLists.txt
	cp ../dynadjust/dnaplot/CMakeLists.release.txt ../dynadjust/dnaplot/CMakeLists.txt
	cp ../dynadjust/dnaplotwrapper/CMakeLists.release.txt ../dynadjust/dnaplotwrapper/CMakeLists.txt
	cp ../dynadjust/dnareftran/CMakeLists.release.txt ../dynadjust/dnareftran/CMakeLists.txt
	cp ../dynadjust/dnareftranwrapper/CMakeLists.release.txt ../dynadjust/dnareftranwrapper/CMakeLists.txt
	cp ../dynadjust/dnasegment/CMakeLists.release.txt ../dynadjust/dnasegment/CMakeLists.txt
	cp ../dynadjust/dnasegmentwrapper/CMakeLists.release.txt ../dynadjust/dnasegmentwrapper/CMakeLists.txt
	cp ../dynadjust/dynadjust/CMakeLists.release.txt ../dynadjust/dynadjust/CMakeLists.txt
fi

cmake ../

make -j2

#exit

DYNADJUST_INSTALL_PATH=/opt/dynadjust/gcc/1_0_0

if [ ! -d "/opt/dynadjust" ]; then
    sudo mkdir /opt/dynadjust
fi

if [ ! -d "/opt/dynadjust/gcc" ]; then
    sudo mkdir /opt/dynadjust/gcc
    sudo mkdir $DYNADJUST_INSTALL_PATH
fi

if [ ! -d $DYNADJUST_INSTALL_PATH ]; then
    sudo mkdir $DYNADJUST_INSTALL_PATH
fi

echo "Copying libraries and binaries to $DYNADJUST_INSTALL_PATH ..."

if [ -e "./dynadjust/dynadjust/dynadjust" ]; then
	sudo cp ./dynadjust/dynadjust/dynadjust $DYNADJUST_INSTALL_PATH/
	ln -sf $DYNADJUST_INSTALL_PATH/dynadjust ~/bin/dynadjust
	echo " - dynadjust"
fi

if [ -e "./dynadjust/dnaadjustwrapper/dnaadjust" ]; then
	sudo cp ./dynadjust/dnaadjust/libdnaadjust.so $DYNADJUST_INSTALL_PATH/
	sudo cp ./dynadjust/dnaadjustwrapper/dnaadjust $DYNADJUST_INSTALL_PATH/
	ln -sf $DYNADJUST_INSTALL_PATH/dnaadjust ~/bin/dnaadjust
	ln -sf $DYNADJUST_INSTALL_PATH/libdnaadjust.so  ~/bin/libdnaadjust.so 
	echo " - dnaadjust, libdnaadjust.so"
fi

if [ -e "./dynadjust/dnaimportwrapper/dnaimport" ]; then
	sudo cp ./dynadjust/dnaimport/libdnaimport.so $DYNADJUST_INSTALL_PATH/
	sudo cp ./dynadjust/dnaimportwrapper/dnaimport $DYNADJUST_INSTALL_PATH/
	ln -sf $DYNADJUST_INSTALL_PATH/dnaimport ~/bin/dnaimport
	ln -sf $DYNADJUST_INSTALL_PATH/libdnaimport.so  ~/bin/libdnaimport.so
	echo " - dnaimport, libdnaimport.so"
fi

if [ -e "./dynadjust/dnareftranwrapper/dnareftran" ]; then
	sudo cp ./dynadjust/dnareftran/libdnareftran.so $DYNADJUST_INSTALL_PATH/
	sudo cp ./dynadjust/dnareftranwrapper/dnareftran $DYNADJUST_INSTALL_PATH/
	ln -sf $DYNADJUST_INSTALL_PATH/dnareftran ~/bin/dnareftran
	ln -sf $DYNADJUST_INSTALL_PATH/libdnareftran.so  ~/bin/libdnareftran.so
	echo " - dnareftran, libdnareftran.so"
fi

if [ -e "./dynadjust/dnageoidwrapper/dnageoid" ]; then
	sudo cp ./dynadjust/dnageoid/libdnageoid.so $DYNADJUST_INSTALL_PATH/
	sudo cp ./dynadjust/dnageoidwrapper/dnageoid $DYNADJUST_INSTALL_PATH/
	ln -sf $DYNADJUST_INSTALL_PATH/dnageoid ~/bin/dnageoid
	ln -sf $DYNADJUST_INSTALL_PATH/libdnageoid.so  ~/bin/libdnageoid.so
	echo " - dnageoid, libdnageoid.so"
fi

if [ -e "./dynadjust/dnasegmentwrapper/dnasegment" ]; then
	sudo cp ./dynadjust/dnasegment/libdnasegment.so $DYNADJUST_INSTALL_PATH/
	sudo cp ./dynadjust/dnasegmentwrapper/dnasegment $DYNADJUST_INSTALL_PATH/
	ln -sf $DYNADJUST_INSTALL_PATH/dnasegment ~/bin/dnasegment
	ln -sf $DYNADJUST_INSTALL_PATH/libdnasegment.so  ~/bin/libdnasegment.so 
	echo " - dnasegment, libdnasegment.so"
fi

if [ -e "./dynadjust/dnaplotwrapper/dnaplot" ]; then
	sudo cp ./dynadjust/dnaplot/libdnaplot.so $DYNADJUST_INSTALL_PATH/
	sudo cp ./dynadjust/dnaplotwrapper/dnaplot $DYNADJUST_INSTALL_PATH/
	ln -sf $DYNADJUST_INSTALL_PATH/dnaplot ~/bin/dnaplot
	ln -sf $DYNADJUST_INSTALL_PATH/libdnaplot.so  ~/bin/libdnaplot.so 
	echo " - dnaplot, libdnaplot.so"
fi

echo "Creating symbolic links to libraries and binaries in $DYNADJUST_INSTALL_PATH ..."
sudo ln -sf $DYNADJUST_INSTALL_PATH/libdnaimport.so /opt/dynadjust/libdnaimport.so
sudo ln -sf $DYNADJUST_INSTALL_PATH/libdnareftran.so /opt/dynadjust/libdnareftran.so
sudo ln -sf $DYNADJUST_INSTALL_PATH/libdnageoid.so /opt/dynadjust/libdnageoid.so
sudo ln -sf $DYNADJUST_INSTALL_PATH/libdnasegment.so /opt/dynadjust/libdnasegment.so 
sudo ln -sf $DYNADJUST_INSTALL_PATH/libdnaadjust.so /opt/dynadjust/libdnaadjust.so 
sudo ln -sf $DYNADJUST_INSTALL_PATH/libdnaplot.so /opt/dynadjust/libdnaplot.so 

sudo ln -sf $DYNADJUST_INSTALL_PATH/dnaimport /opt/dynadjust/dnaimport
sudo ln -sf $DYNADJUST_INSTALL_PATH/dnareftran /opt/dynadjust/dnareftran
sudo ln -sf $DYNADJUST_INSTALL_PATH/dnageoid /opt/dynadjust/dnageoid
sudo ln -sf $DYNADJUST_INSTALL_PATH/dnasegment /opt/dynadjust/dnasegment
sudo ln -sf $DYNADJUST_INSTALL_PATH/dnaadjust /opt/dynadjust/dnaadjust
sudo ln -sf $DYNADJUST_INSTALL_PATH/dnaplot /opt/dynadjust/dnaplot
sudo ln -sf $DYNADJUST_INSTALL_PATH/dynadjust /opt/dynadjust/dynadjust


