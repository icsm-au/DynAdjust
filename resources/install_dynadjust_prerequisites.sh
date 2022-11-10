#! /bin/bash

#################################################################################
#
# This script downloads, (optionally) builds and installs DynAdjust prerequisites
#   
#################################################################################

_distro_centos="CentOS Linux"
_distro_debian="Debian"
_distro_fedora="Fedora"
_distro_opensusel="openSUSE Leap"
_distro_opensuse="openSUSE"
_distro_rhels="Red Hat Enterprise Linux Server"
_distro_rhel="Red Hat Enterprise Linux"
_distro_ubuntu="Ubuntu"

#################################################################################
# Capture system variables and set defaults
if [[ -e "/etc/os-release" ]]; then
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
_script="install_dynadjust_prerequisites.sh"
_mode=0
_distribution=
_create_downloads_dir=0
#################################################################################


#################################################################################
# Common functions
#
# display example message
function example {
    echo -e "example: $_script -d Ubuntu -m 1"
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
    echo -e "  -d [ --distro ] arg    The linux distribution. Recognised distros include:"
    echo -e "                           - ${_distro_centos}"
    echo -e "                           - ${_distro_debian}"
    echo -e "                           - ${_distro_fedora}"
    echo -e "                           - ${_distro_opensuse}"
    echo -e "                           - ${_distro_rhel}"
    echo -e "                           - ${_distro_ubuntu}"
    echo -e "                         If not provided, I will try to get the distro from /etc/os-release."
    echo -e "  -m [ --mode ] arg      Mode of installing prerequisites:"
    echo -e "                           0: interactive (default)"
    echo -e "                           1: package manager"
    echo -e "                           2: build from source"
    echo -e "                           3: skip"
    echo -e "  -h [ --help ]          Prints this help message\n"
    example
    echo ""
}

# get argument parameters
while [[ "$1" != "" ]];
do
   case $1 in
   -d  | --distro ) shift
                    _distribution=$1
                	;;
   -m  | --mode )   shift
   					_mode=$1
			        ;;
   -h   | --help )  help
                    exit
                    ;;
   *)                     
                    echo "$script: illegal option $1"
                    usage
					example
					exit 1 # error
                    ;;
    esac
    shift
done

# Checks valid values
function args_check {
	if [[ $_mode -lt 0 ]] || [[ $_mode -gt 3 ]]; then
        # error
        echo -e "\nUnknown value: --mode $_mode"
	    help
	    exit 1 # error
	fi

    firstletter=${_distribution^}
    firstletter=${firstletter:0:1}

    # Check if distribution argument is not empty. If empty, _distro is set by default
    if [[ ! -z $_distribution ]]; then
        # Set distro based on first character
        if [[ "$firstletter" = "C" ]]; then
            _distro=${_distro_centos}
        elif [[ "$firstletter" = "R" ]]; then
            _distro=${_distro_rhel}
        elif [[ "$firstletter" = "F" ]]; then
            _distro=${_distro_fedora}
        elif [[ "$firstletter" = "O" ]]; then
            _distro=${_distro_opensuse}
        elif [[ "$firstletter" = "U" ]]; then
            _distro=${_distro_ubuntu}
        elif [[ "$firstletter" = "D" ]]; then
            _distro=${_distro_debian}
        else
            # error
            echo -e "\nerror: Unknown distribution: \"$_distribution\"\n"
            echo -e "  If your Linux distribution is based on a distro in the supported"
            echo -e "  list (e.g. Mint is based on Ubuntu), try running:"
            echo -e "    $ ${_script} -d base-distro\n"
            echo -e "  If this fails to work, please submit an issue at:"
            echo -e "  https://github.com/icsm-au/DynAdjust/issues including this message"
            echo -e "  and your distribution."
            help
            exit 1 # error
        fi
    fi

    if [[ "$_mode" -gt 0 ]]; then
        echo -e "\n==========================================================================="
        echo -e "Installing prerequisites automatically without user input..."
    fi

    # print outcome
    if [[ $1 -eq 1 ]]; then

        echo -e "\nos:      $OSTYPE";
        echo "distro:  $_distro";
        case $_mode in
            0) echo "mode:    interactive";;
            1) echo "mode:    package manager";;
            2) echo "mode:    build from source";;
            3) echo "mode:    skip";;
            *)
                # error
                echo -e "\nUnknown value: --mode $_mode"
                help
                exit 1 # error
            ;;
        esac
        echo -e "system:  $_system\n";

    fi

}
#################################################################################

# check arguments (1 prints results, 0 doesn't)
#args_check 1
args_check 0

_repo_intel_yum="https://yum.repos.intel.com/oneapi"
_repo_intel_apt="https://apt.repos.intel.com/oneapi"
_gpg_intel_yum="https://yum.repos.intel.com/intel-gpg-keys/GPG-PUB-KEY-INTEL-SW-PRODUCTS.PUB"
_gpg_intel_apt="https://apt.repos.intel.com/intel-gpg-keys/GPG-PUB-KEY-INTEL-SW-PRODUCTS.PUB"
_gpg_intel_keyfile="GPG-PUB-KEY-INTEL-SW-PRODUCTS.PUB"
_toolset_apt="apt"
_toolset_yum="yum"
_toolset_dnf="dnf"
_toolset_zyp="zypper"
_format_deb="deb"
_format_rpm="rpm"
_no_ask="-y"

# 1 GET OS, DISTRO AND TOOLS
if [[ "$OSTYPE" == "linux"* ]]; then
    # Linux
    if [[ "$_distro" == "CentOS"* || "$_distro" == "Red Hat"* ]]; then
        # NAME="CentOS Linux"
        # NAME="Red Hat Enterprise Linux Server"
        _repo_intel="$_repo_intel_yum"
        _gpg_intel="$_gpg_intel_yum"
        _toolset="$_toolset_yum"
        _format="$_format_rpm"
    elif [[ "$_distro" == "Fedora"* ]]; then
        # NAME=Fedora
        _repo_intel="$_repo_intel_yum"
        _gpg_intel="$_gpg_intel_yum"
        _toolset="$_toolset_dnf"
        _format="$_format_rpm"
    elif [[ "$_distro" == *"SUSE"* || "$_distro" == "SLES"* ]]; then
        # NAME="openSUSE Leap"
        # NAME=openSUSE
        _repo_intel="$_repo_intel_yum"
        _gpg_intel="$_gpg_intel_yum"
        _toolset="$_toolset_zyp"
        _format="$_format_rpm"
	_no_ask="-n"
    elif [[ "$_distro" == "Ubuntu" || "$_distro" == "Debian"* ]]; then
        # NAME="Ubuntu"
        # NAME="Debian GNU/Linux"
        _repo_intel="$_repo_intel_apt"
        _gpg_intel="$_gpg_intel_apt"
        _toolset="$_toolset_apt"
        _format="$_format_deb"
    else
        # Unknown.
        echo " "
        echo "I don't know where to find $OSTYPE or Intel MKL repos for $_distro and am going to quit."
        echo " "
        exit
    fi
elif [[ "$OSTYPE" == "darwin"* ]]; then
    # Mac OSX
    echo " "
    echo "I don't know how to handle $OSTYPE and am going to quit."
    echo " "
    exit
else
    # Unknown.
    echo " "
    echo "I don't know how to handle $OSTYPE and am going to quit."
    echo " "
    exit
fi

# Debian	.deb	apt, apt-cache, apt-get, dpkg
# Ubuntu	.deb	apt, apt-cache, apt-get, dpkg
# CentOS	.rpm	yum
# Fedora	.rpm	dnf
# SuSE      .rpm    zypper

echo -e "\n==========================================================================="
echo "Installation of dynadjust prerequisites"
echo " "
echo "Environment:"
echo " distro:          $_distro"
echo " system:          $_system"
echo " format:          $_format"
echo " package manager: $_toolset"
echo "Installing:"
echo " boost            https://boost.org (via $_toolset repos)"
echo " intel mkl        $_repo_intel"
echo " xerces-c         http://archive.apache.org"
echo " xsd              https://www.codesynthesis.com"

#
# determine if user needs prompting
case $_mode in
    0) # interactive
        echo " "
        read -r -p "Is this ok [Y/n]: " response;;
    *) # proceed without asking
        response="y";;
esac

if [[ "$response" =~ ^([nN][oO]|[nN])$ ]]
then    
    exit
else
    echo " "
fi

# get current directory
_cwd="$PWD"

echo "Prerequisites Log" | tee "$_cwd/prerequisites.log"

#
# get standard yum repos for building c++ projects
echo -e "\n==========================================================================="
echo "Checking for the following packages (install if missing):"

# Install basic packages from package manager
if [[ "$_format" == "rpm" ]]; then
    echo " bzip2, wget, cmake, make, gcc-c++, git and boost + boost-devel..."
    echo " "
    sudo "$_toolset" update
    if [[ "$_distro" == *"SUSE"* || "$_distro" == "SLES"* ]]; then
        sudo "$_toolset" "$_no_ask" install bzip2 wget cmake make gcc-c++ git boost-devel libboost_system* libboost_filesystem* libboost_timer* libboost_thread* libboost_program_options*
    else
        sudo "$_toolset" "$_no_ask" install bzip2 wget cmake make gcc-c++ git boost boost-devel
    fi
elif [[ "$_format" == "deb" ]]; then
    echo " bzip2, wget, cmake, make, gcc-c++, git and libboost-all-dev..."
    echo " "
    sudo "$_toolset" update
    sudo "$_toolset" "$_no_ask" install cmake pkg-config build-essential
    sudo "$_toolset" "$_no_ask" install bzip2 wget make gcc git libboost-system-dev libboost-filesystem-dev libboost-timer-dev libboost-thread-dev libboost-program-options-dev
else
    echo " "
    echo "I don't know how to handle $OSTYPE or $_distro and am going to quit."
    echo " "
    exit
fi

echo -e "\n==========================================================================="
echo "Installation of Intel Math kernel Library (MKL):"

#
# determine if/how to install intel mkl
case $_mode in
    0) # interactive
        echo " "
        read -r -p "Download and install Intel MKL [Y/n]: " mklresponse;;
    3) # skip
        echo " "
        mklresponse="n";;
esac

#
# install intel mkl
if [[ "$mklresponse" =~ ^([nN][oO]|[nN])$ ]]
then    
    echo -e "Skipping Intel MKL installation.\n"
else
    # Install MKL for rpm based distros (Fedora, CentOS, Red Hat, SUSE, OpenSUSE)
    # From:
    #  https://www.intel.com/content/www/us/en/develop/documentation/installation-guide-for-intel-oneapi-toolkits-linux/top/installation/install-using-package-managers/yum-dnf-zypper.html
    if [[ "$_format" == "rpm" ]]; then

cat << EOF > /tmp/oneAPI.repo
[oneAPI]
name=Intel® oneAPI repository
baseurl=$_repo_intel_yum
enabled=1
gpgcheck=1
repo_gpgcheck=1
gpgkey=$_gpg_intel
EOF
        sudo mv /tmp/oneAPI.repo /etc/yum.repos.d

        if [[ "$_distro" == *"SUSE"* || "$_distro" == "SLES"* ]]; then
            sudo "$_toolset" addrepo "$_repo_intel" oneAPI
            sudo "$_format" --import "$_gpg_intel"
            sudo "$_toolset" "$_no_ask" install intel-intel-oneapi-compiler-dpcpp-cpp intel-oneapi-mkl-devel intel-oneapi-tbb-devel
        else
            sudo "$_toolset" "$_no_ask" install intel-intel-oneapi-compiler-dpcpp-cpp intel-oneapi-mkl-devel intel-oneapi-tbb-devel
        fi

    # Install MKL for deb based distros (Ubuntu, Debian)
    # From:
    #  https://www.intel.com/content/www/us/en/develop/documentation/installation-guide-for-intel-oneapi-toolkits-linux/top/installation/install-using-package-managers/apt.html
    elif [[ "$_format" == "deb" ]]; then
        # download the key to system keyring
        wget -O- "$_gpg_intel" | gpg --dearmor | sudo tee /usr/share/keyrings/oneapi-archive-keyring.gpg > /dev/null
        # add signed entry to apt sources and configure the APT client to use Intel repository:
        echo "deb [signed-by=/usr/share/keyrings/oneapi-archive-keyring.gpg] https://apt.repos.intel.com/oneapi all main" | sudo tee /etc/apt/sources.list.d/oneAPI.list
        sudo "$_toolset" -y install intel-intel-oneapi-compiler-dpcpp-cpp intel-oneapi-mkl-devel intel-oneapi-tbb-devel
    else
        echo " "
        echo "I don't know how to handle $OSTYPE or $_distro and am going to quit."
        echo " "
        exit
    fi

    if [[ -e "$_gpg_intel_keyfile" ]]; then
        rm -f "$_gpg_intel_keyfile"
    fi
fi

DOWNLOADS_FOLDER="~/downloads"
eval DOWNLOADS_FOLDER_FULLPATH="$DOWNLOADS_FOLDER"


#
# INSTALL XERCES-C (3.1.4)
echo -e "\n==========================================================================="
echo -e "Installation of Apache Xerces-C++ XML Parser (xerces-c)\n"
#
# determine how to install xerces-c
case $_mode in
    0) # interactive
        COLUMNS=1
        PS3='Select which method to use to install xerces-c: '
        select opt in "Package manager ($_toolset)" "Build xerces-c from source and install to /opt" "Skip installation"
        do
            case $opt in
                "Package manager ($_toolset)")
                    break
                    ;;
                "Build xerces-c from source and install to /opt")
                    break
                    ;;
                "Skip installation")
                    break
                    ;;
                *) 
                    echo "invalid option $REPLY"
                    ;;
            esac
        done
        ;;
    1) # package manager
        REPLY=1
        ;;
    2) # bould from source
        REPLY=2
        ;;
    3) # skip
        REPLY=3
        ;;
esac

# Package manager
if [[ $REPLY == 1 ]]; then
    echo " "
    echo "Installing xerces-c via $_toolset..."

    # Install xerces-c for rpm based distros (Fedora, CentOS, Red Hat, SUSE, OpenSUSE)
    if [[ "$_format" == "rpm" ]]; then
        if [[ "$_distro" == *"SUSE"* || "$_distro" == "SLES"* ]]; then
            sudo "$_toolset" "$_no_ask" install xerces-c-devel
        else
            sudo "$_toolset" "$_no_ask" install xerces-c-devel
        fi
    # Install xerces-c for deb based distros (Ubuntu, Debian)
    elif [[ "$_format" == "deb" ]]; then
        sudo "$_toolset" -y install libxerces-c-dev
    else
        echo " "
        echo "I don't know how to handle $OSTYPE or $_distro and am going to quit."
        echo " "
        exit
    fi

    echo " "

# Build from source
elif [[ $REPLY == 2 ]]; then
    echo " "
    echo "Installing xerces-c from source..."

    # 1. create install dir:
    if [[ ! -d "/opt/xerces-c" ]]; then
        sudo mkdir /opt/xerces-c/
    fi 
    
    if [[ ! -d "/opt/xerces-c/3.1.4" ]]; then
        sudo mkdir /opt/xerces-c/3.1.4
    fi    

    if [[ ! -d "$DOWNLOADS_FOLDER_FULLPATH" ]]; then
        mkdir "$DOWNLOADS_FOLDER_FULLPATH"
        _create_downloads_dir=1
        echo -e "\n Creating ${DOWNLOADS_FOLDER} folder to store temporary files."
    else
        echo -e "\n ${DOWNLOADS_FOLDER} folder exists. Using this folder to store temporary files."
    fi

    # 2. download:
    echo " "
    echo "Downloading xerces-c 3.1.4..."
    echo " "
    cd "$DOWNLOADS_FOLDER_FULLPATH"

    wget http://archive.apache.org/dist/xerces/c/3/sources/xerces-c-3.1.4.tar.gz

    # 3. extract: 
    tar xvzf xerces-c-3.1.4.tar.gz | tee -a "$_cwd/prerequisites.log" > /dev/null

    # 4. compile:
    echo "Building xerces-c (be patient)..."
    cd ./xerces-c-3.1.4
    ./configure --prefix=/opt/xerces-c/3.1.4 | tee -a "$_cwd/prerequisites.log" > /dev/null
    make -j $(nproc) | tee -a "$_cwd/prerequisites.log" > /dev/null
    
    echo "Installing xerces-c to /opt/xerces-c/3.1.4..."
    sudo make install | tee -a "$_cwd/prerequisites.log" > /dev/null
    echo "Done."
    
    # Return to running directory
    cd "$_cwd"

    # 5. cleanup
    XERCES_TMP_FOLDER="$DOWNLOADS_FOLDER_FULLPATH/xerces-c-3.1.4/"
    XERCES_TMP_FILE="$DOWNLOADS_FOLDER_FULLPATH/xerces-c-3.1.4.tar.gz"
    
    echo "Cleaning up:"
    echo " $XERCES_TMP_FOLDER"
    echo " $XERCES_TMP_FILE"
    
    if [[ -d "$XERCES_TMP_FOLDER" ]]; then
        rm -Rf "$XERCES_TMP_FOLDER"
    fi

    if [[ -e "$XERCES_TMP_FILE" ]]; then
        rm -f "$XERCES_TMP_FILE"
    fi

    if [[ _create_downloads_dir -eq 1 ]]; then
        #echo -e "\n ${DOWNLOADS_FOLDER} removed"
        rm -R "$DOWNLOADS_FOLDER_FULLPATH"    
    fi
    
else
    echo " "
    echo "Skipping xerces-c installation."
fi

#
# INSTALL XSD
echo -e "\n==========================================================================="
echo -e "Installation of Codesynthesis XSD: XML Data Binding for C++ (xsd)\n"
#
# determine how to install xsd
case $_mode in
    0) # interactive
        COLUMNS=1
        PS3='Select which method to use to install xsd: '
        select opt in "Package manager ($_toolset)" "Download and install xsd to /opt" "Skip installation"
        do
            case $opt in
                "Package manager ($_toolset)")
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
        ;;
    1) # package manager
        REPLY=1
        ;;
    2) # bould from source
        REPLY=2
        ;;
    3) # skip
        REPLY=3
        ;;
esac

# Package manager
if [[ $REPLY == 1 ]]; then
    echo " "
    echo "Installing xsd via $_toolset..."
    echo "WARNING: DynAdjust has been developed with xsd version 4.0.0."
    echo "         Later versions may cause compilation to fail."
    echo " "

    # Install xsd for rpm based distros (Fedora, CentOS, Red Hat, SUSE, OpenSUSE)
    if [[ "$_format" == "rpm" ]]; then        
	if [[ "$_distro" == *"SUSE"* || "$_distro" == "SLES"* ]]; then
            sudo "$_toolset" "$_no_ask" install xsd
        else
            sudo "$_toolset" "$_no_ask" install xsd
        fi
    # Install xsd for deb based distros (Ubuntu, Debian)
    elif [[ "$_format" == "deb" ]]; then
        sudo "$_toolset" -y install xsdcxx
    else
        echo " "
        echo "I don't know how to handle $OSTYPE or $_distro and am going to quit."
        echo " "
        exit
    fi

    echo " "

# Install from source
elif [[ $REPLY == 2 ]]; then
    echo " "
    echo "Installing xsd from source..."

    # 1. create install dir:
    if [[ ! -d "/opt/xsd" ]]; then
        sudo mkdir /opt/xsd
    fi

    if [[ ! -d "$DOWNLOADS_FOLDER_FULLPATH" ]]; then
        mkdir "$DOWNLOADS_FOLDER_FULLPATH"
        _create_downloads_dir=1
        echo -e "\n Creating ${DOWNLOADS_FOLDER} folder to store temporary files."
    else
        echo -e "\n ${DOWNLOADS_FOLDER} folder exists. Using this folder to store temporary files."
    fi

    # 2. download:
    echo " "
    echo "Downloading and extracting xsd-4.0.0..."
    echo " "
    cd "$DOWNLOADS_FOLDER_FULLPATH"

    wget https://www.codesynthesis.com/download/xsd/4.0/linux-gnu/x86_64/xsd-4.0.0-x86_64-linux-gnu.tar.bz2 

    # 3. extract:
    tar xjf xsd-4.0.0-x86_64-linux-gnu.tar.bz2 | tee -a "$_cwd/prerequisites.log" > /dev/null

    # 4. move:
    echo "Moving xsd to /opt/xsd..."
    echo " "
    sudo mv xsd-4.0.0-x86_64-linux-gnu /opt/xsd | tee -a "$_cwd/prerequisites.log" > /dev/null

    # Return to running directory
    cd "$_cwd"

    # 5. cleanup
    XSD_TMP_FOLDER="$DOWNLOADS_FOLDER_FULLPATH/xsd-4.0.0-x86_64-linux-gnu/"
    XSD_TMP_FILE="$DOWNLOADS_FOLDER_FULLPATH/xsd-4.0.0-x86_64-linux-gnu.tar.bz2"
    
    echo "Cleaning up:"
    echo " $XSD_TMP_FOLDER"
    echo " $XSD_TMP_FILE"
    
    if [[ -d "$XSD_TMP_FOLDER" ]]; then
        rm -Rf "$XSD_TMP_FOLDER"
    fi
    
    if [[ -e "$XSD_TMP_FILE" ]]; then
        rm -f "$XSD_TMP_FILE"
    fi 

    if [[ _create_downloads_dir -eq 1 ]]; then
        #echo -e "\n ${DOWNLOADS_FOLDER} removed"
        rm -R "$DOWNLOADS_FOLDER_FULLPATH"    
    fi

else
    echo " "
    echo "Skipping xsd installation."
fi  

echo " "
echo "Done."
echo " "
echo "If all prerequisites have been installed successfully, run ./make_dynadjust_gcc.sh to build dynadjust."
echo "Note that if boost, mkl, xerces-c or xsd is missing, compilation of DynAdjust will not succeed."
echo " "
