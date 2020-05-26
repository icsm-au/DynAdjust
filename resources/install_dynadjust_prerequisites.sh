#! /bin/bash

#################################################################################
#
# This script downloads, (optionally) builds and installs DynAdjust prerequisites
#   
#################################################################################


# Capture system variables and set defaults

if [ -e "/etc/os-release" ]; then
    # NAME="CentOS Linux"
    # NAME="Red Hat Enterprise Linux Server"
    # NAME=Fedora
    # NAME="openSUSE Leap"
    # NAME=openSUSE
    # NAME="Ubuntu"
    # NAME="Debian"
    _distro=$(awk -F= '/^NAME/{print $2}' /etc/os-release | tr -d \")
else
    _distro=$(awk -F= '/^NAME/{print $2}' /usr/lib/os-release | tr -d \")
fi

_system=$(uname -sroi)

_repo_intel_yum="https://yum.repos.intel.com/mkl/setup/intel-mkl.repo"
_repo_intel_apt="https://apt.repos.intel.com/mkl"
_gpg_intel_yum="https://yum.repos.intel.com/intel-gpg-keys/GPG-PUB-KEY-INTEL-SW-PRODUCTS-2019.PUB"
_gpg_intel_apt="https://apt.repos.intel.com/intel-gpg-keys/GPG-PUB-KEY-INTEL-SW-PRODUCTS-2019.PUB"
_gpg_intel_keyfile="GPG-PUB-KEY-INTEL-SW-PRODUCTS-2019.PUB"
_toolset_apt="apt"
_toolset_yum="yum"
_toolset_dnf="dnf"
_toolset_zyp="zypper"
_format_deb="deb"
_format_rpm="rpm"

# 1 GET OS, DISTRO AND TOOLS
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    # Linux
    if [[ "$_distro" == "CentOS"* || "$_distro" == "Red Hat"* ]]; then
        # NAME="CentOS Linux"
        # NAME="Red Hat Enterprise Linux Server"
        _repo_intel="${_repo_intel_yum}"
        _gpg_intel="${_gpg_intel_yum}"
        _toolset="${_toolset_yum}"
        _format="${_format_rpm}"
    elif [[ "$_distro" == "Fedora"* ]]; then
        # NAME=Fedora
        _repo_intel="${_repo_intel_yum}"
        _gpg_intel="${_gpg_intel_yum}"
        _toolset="${_toolset_dnf}"
        _format="${_format_rpm}"
    elif [[ "$_distro" == *"SUSE"* ]]; then
        # NAME="openSUSE Leap"
        # NAME=openSUSE
        _repo_intel="${_repo_intel_yum}"
        _gpg_intel="${_gpg_intel_yum}"
        _toolset="${_toolset_zyp}"
        _format="${_format_rpm}"
    elif [[ "$_distro" == "Ubuntu" || "$_distro" == "Debian"* ]]; then
        # NAME="Ubuntu"
        # NAME="Debian GNU/Linux"
        _repo_intel="${_repo_intel_apt}"
        _gpg_intel="${_gpg_intel_apt}"
        _toolset="${_toolset_apt}"
        _format="${_format_deb}"
    else
        # Unknown.
        echo " "
        echo "I don't know where to find ${OSTYPE} or Intel MKL repos for ${_distro} and am going to quit."
        echo " "
        exit
    fi
elif [[ "$OSTYPE" == "darwin"* ]]; then
    # Mac OSX
    echo " "
    echo "I don't know how to handle ${OSTYPE} and am going to quit."
    echo " "
    exit
else
    # Unknown.
    echo " "
    echo "I don't know how to handle ${OSTYPE} and am going to quit."
    echo " "
    exit
fi

# Debian	.deb	apt, apt-cache, apt-get, dpkg
# Ubuntu	.deb	apt, apt-cache, apt-get, dpkg
# CentOS	.rpm	yum
# Fedora	.rpm	dnf
# SuSE      .rpm    zypper

echo " "
echo "Installation of dynadjust prerequisites"
echo "==========================================================================="
echo "Environment:"
echo " distro:          ${_distro}"
echo " system:          ${_system}"
echo " format:          ${_format}"
echo " package manager: ${_toolset}"
echo "Installing:"
echo " boost            https://boost.org (via ${_toolset_yum} repos)"
echo " intel mkl        ${_repo_intel}"
echo " xerces-c         http://archive.apache.org"
echo " xsd              https://www.codesynthesis.com"
echo "==========================================================================="
echo " "
read -r -p "Is this ok [Y/n]: " response

if [[ "$response" =~ ^([nN][oO]|[nN])$ ]]
then    
    exit
else
    echo " "
fi

# get current directory
_cwd="$PWD"

echo "Prerequisites Log" > "${_cwd}/prequisites.log" 2>&1

#
# get standard yum repos for building c++ projects
echo "Checking for the following packages (install if missing):"

# Install basic packages from package manager
if [[ "${_format}" == "rpm" ]]; then
    echo " bzip2, p7zip, wget, cmake, make, g++, git and boost + boost-devel..."
    echo " "
    sudo ${_toolset} install bzip2 p7zip wget cmake make g++ git boost boost-devel 
elif [[ "${_format}" == "deb" ]]; then
    echo " bzip2, p7zip, wget, cmake, make, gcc-c++, git and libboost-all-dev..."
    echo " "
    sudo ${_toolset}-get install bzip2 p7zip wget cmake make g++ git libboost-all-dev
else
    echo " "
    echo "I don't know how to handle ${OSTYPE} or ${_distro} and am going to quit."
    echo " "
    exit
fi

#
# install intel mkl
echo " "
read -r -p "Download and install Intel MKL [Y/n]: " mklresponse
if [[ "$mklresponse" =~ ^([nN][oO]|[nN])$ ]]
then    
    echo "Skipping Intel MKL installation."
else
    # Install MKL for rpm based distros (Fedora, CentOS, Red Hat, SUSE, OpenSUSE)
    if [[ "${_format}" == "rpm" ]]; then
        sudo ${_toolset} config-manager --add-repo ${_repo_intel}
        sudo ${_format} --import ${_gpg_intel}
        sudo ${_toolset} install intel-mkl
    # Install MKL for deb based distros (Ubuntu, Debian)
    elif [[ "${_format}" == "deb" ]]; then
        wget ${_gpg_intel}
        sudo ${_toolset}-key add ${_gpg_intel_keyfile}
        sudo sh -c 'echo deb https://apt.repos.intel.com/mkl all main > /etc/apt/sources.list.d/intel-mkl.list'
        sudo ${_toolset}-get install intel-mkl
    else
        echo " "
        echo "I don't know how to handle ${OSTYPE} or ${_distro} and am going to quit."
        echo " "
        exit
    fi

    if [ -e ${_gpg_intel_keyfile} ]; then
        rm -f ${_gpg_intel_keyfile}
    fi
fi

DOWNLOADS_FOLDER="~/downloads"
DOWNLOADS_FOLDER_FULLPATH="`eval echo ${DOWNLOADS_FOLDER//>}`"

#
# INSTALL XERCES-C (3.1.4)
# NOTE - consider dnf install xerces-c-devel
#
echo " "
echo "Installation of Apache Xerces-C++ XML Parser (xerces-c)"
PS3='Select which method to use to install xerces-c: '
select opt in "Package manager (${_toolset})" "Build xerces-c from source and install to /opt" "Skip installation"
do
    case $opt in
        "Package manager (${_toolset})")
            break
            ;;
        "Build xerces-c from source and install to /opt")
            break
            ;;
        "Skip installation")
            break
            ;;
        *) echo "invalid option $REPLY";;
    esac
done

# Package manager
if [ ${REPLY} == 1 ]; then
    echo " "
    echo "Installing xerces-c via ${_toolset}..."

    # Install xerces-c for rpm based distros (Fedora, CentOS, Red Hat, SUSE, OpenSUSE)
    if [[ "${_format}" == "rpm" ]]; then
        sudo ${_toolset} install xerces-c-devel
    # Install xerces-c for deb based distros (Ubuntu, Debian)
    elif [[ "${_format}" == "deb" ]]; then
        sudo ${_toolset}-get install libxerces-c-dev
    else
        echo " "
        echo "I don't know how to handle ${OSTYPE} or ${_distro} and am going to quit."
        echo " "
        exit
    fi

    echo " "

# Build from source
elif [ ${REPLY} == 2 ]; then
    echo " "
    echo "Installing xerces-c from source..."

    # 1. create install dir:
    if [ ! -d "/opt/xerces-c" ]; then
        sudo mkdir /opt/xerces-c/
    fi 
    
    if [ ! -d "/opt/xerces-c/3.1.4" ]; then
        sudo mkdir /opt/xerces-c/3.1.4
    fi    

    # 2. download:
    echo "Downloading xerces-c 3.1.4..."
    echo " "
    cd $DOWNLOADS_FOLDER_FULLPATH >> "${_cwd}/prequisites.log" 2>&1

    wget http://archive.apache.org/dist/xerces/c/3/sources/xerces-c-3.1.4.tar.gz

    # 3. extract: 
    tar xvzf xerces-c-3.1.4.tar.gz >> "${_cwd}/prequisites.log" 2>&1

    # 4. compile:
    echo "Building xerces-c (be patient)..."
    cd ./xerces-c-3.1.4
    ./configure --prefix=/opt/xerces-c/3.1.4 >> "${_cwd}/prequisites.log" 2>&1
    make -j >> "${_cwd}/prequisites.log" 2>&1
    
    echo "Installing xerces-c to /opt/xerces-c/3.1.4..."
    sudo make install >> "${_cwd}/prequisites.log" 2>&1
    echo "Done."
    
    # Return to running directory
    cd $_cwd

    # 5. cleanup
    XERCES_TMP_FOLDER="${DOWNLOADS_FOLDER_FULLPATH}/xerces-c-3.1.4/"
    XERCES_TMP_FILE="${DOWNLOADS_FOLDER_FULLPATH}/xerces-c-3.1.4.tar.gz"
    
    echo "Cleaning up:"
    echo " ${XERCES_TMP_FOLDER}"
    echo " ${XERCES_TMP_FILE}"
    
    if [ -d $XERCES_TMP_FOLDER ]; then
        rm -Rf $XERCES_TMP_FOLDER
    fi

    if [ -e $XERCES_TMP_FILE ]; then
        rm -f $XERCES_TMP_FILE
    fi
    
else
    echo " "
    echo "Skipping xerces-c installation."
fi


#
# INSTALL XSD
# NOTE - consider dnf install xsd
#

echo " "
echo "Installation of Codesynthesis XSD: XML Data Binding for C++ (xsd)"
PS3='Select which method to use to install xsd: '
select opt in "Package manager (${_toolset})" "Download and install xsd to /opt" "Skip installation"
do
    case $opt in
        "Package manager (${_toolset})")
            break
            ;;
        "Download and install xsd to /opt")
            break
            ;;
        "Skip installation")
            break
            ;;
        *) echo "invalid option $REPLY";;
    esac
done

# Package manager
if [ ${REPLY} == 1 ]; then
    echo " "
    echo "Installing xsd via ${_toolset}..."
    echo "WARNING: DynAdjust has been developed with xsd version 4.0.0."
    echo "         Later versions may cause compilation to fail."
    echo " "

    # Install xsd for rpm based distros (Fedora, CentOS, Red Hat, SUSE, OpenSUSE)
    if [[ "${_format}" == "rpm" ]]; then
        sudo ${_toolset} install xsd
    # Install xsd for deb based distros (Ubuntu, Debian)
    elif [[ "${_format}" == "deb" ]]; then
        sudo ${_toolset}-get install xsdcxx
    else
        echo " "
        echo "I don't know how to handle ${OSTYPE} or ${_distro} and am going to quit."
        echo " "
        exit
    fi

    echo " "

# Install from source
elif [ ${REPLY} == 2 ]; then
    echo " "
    echo "Installing xsd from source..."

    # 1. create install dir:
    if [ ! -d "/opt/xsd" ]; then
        sudo mkdir /opt/xsd
    fi

    if [ ! -d $DOWNLOADS_FOLDER_FULLPATH ]; then
        mkdir $DOWNLOADS_FOLDER_FULLPATH
    fi

    # 2. download:
    echo " "
    echo "Downloading and extracting xsd-4.0.0..."
    echo " "
    cd $DOWNLOADS_FOLDER_FULLPATH >> "${_cwd}/prequisites.log" 2>&1

    wget https://www.codesynthesis.com/download/xsd/4.0/linux-gnu/x86_64/xsd-4.0.0-x86_64-linux-gnu.tar.bz2 

    # 3. extract:
    tar xjf xsd-4.0.0-x86_64-linux-gnu.tar.bz2 >> "${_cwd}/prequisites.log" 2>&1

    # 4. move:
    echo "Moving xsd to /opt/xsd..."
    echo " "
    sudo mv xsd-4.0.0-x86_64-linux-gnu /opt/xsd >> "${_cwd}/prequisites.log" 2>&1

    # Return to running directory
    cd $_cwd

    # 5. cleanup
    XSD_TMP_FOLDER="${DOWNLOADS_FOLDER_FULLPATH}/xsd-4.0.0-x86_64-linux-gnu/"
    XSD_TMP_FILE="${DOWNLOADS_FOLDER_FULLPATH}/xsd-4.0.0-x86_64-linux-gnu.tar.bz2"
    
    echo "Cleaning up:"
    echo " ${XSD_TMP_FOLDER}"
    echo " ${XSD_TMP_FILE}"
    
    if [ -d $XSD_TMP_FOLDER ]; then
        rm -Rf $XSD_TMP_FOLDER
    fi
    
    if [ -e $XSD_TMP_FILE ]; then
        rm -f $XSD_TMP_FILE
    fi 

else
    echo " "
    echo "Skipping xsd installation."
fi  

echo " "
echo "Done."
echo " "
echo "Now, run ./make_dynadjust_gcc.sh to build dynadjust."
#echo "If mkl, xerces-c or xsd is missing, compilation of DynAdjust will not succeed."
echo " "
