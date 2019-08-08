#
read -p "Press [Enter] key to start installation..."
#
# RHEL-8 distributables
sudo dnf install bzip2 boost boost-devel cmake make gcc-c++ git wget
#
# install intel mkl
#read -p "Press [Enter] key to install Intel MKL..."
#sudo dnf config-manager --add-repo https://yum.repos.intel.com/mkl/setup/intel-mkl.repo
#sudo rpm --import https://yum.repos.intel.com/intel-gpg-keys/GPG-PUB-KEY-INTEL-SW-PRODUCTS-2019.PUB
sudo dnf install intel-mkl
#
# INSTALL XERCES-C (3.1.4)
read -p "Press [Enter] key to install Xerces C..."
# 1. create install dir:
if [ ! -d "/opt/xerces-c/3.1.4" ]; then
    sudo mkdir /opt/xerces-c/
    sudo mkdir /opt/xerces-c/3.1.4
fi

if [ ! -d "~/downloads" ]; then
    mkdir ~/downloads
fi

# 2. download:
cd ~/downloads
wget http://archive.apache.org/dist/xerces/c/3/sources/xerces-c-3.1.4.tar.gz
# 3. extract: 
tar xvzf xerces-c-3.1.4.tar.gz
# 4. compile:
cd ./xerces-c-3.1.4
./configure --prefix=/opt/xerces-c/3.1.4
make -j
sudo make install
#
# INSTALL XSD
read -p "Press [Enter] key to install XSD..."
# 1. create install dir:
if [ ! -d "/opt/xsd" ]; then
    sudo mkdir /opt/xsd
fi
# 2. download:
cd ~/downloads
wget https://www.codesynthesis.com/download/xsd/4.0/linux-gnu/x86_64/xsd-4.0.0-x86_64-linux-gnu.tar.bz2
# 3. extract:
tar xjf xsd-4.0.0-x86_64-linux-gnu.tar.bz2
# 4. move:
sudo mv xsd-4.0.0-x86_64-linux-gnu /opt/xsd
#
# INSTALL DYNADJUST
# 1. create install dirs:
read -p "Press [Enter] key to install DynAdjust..."
if [ ! -d "/opt/dynadjust/gcc/1_0_0" ]; then
    sudo mkdir /opt/dynadjust
    sudo mkdir /opt/dynadjust/gcc
    sudo mkdir /opt/dynadjust/gcc/1_0_0
fi
if [ ! -d "~/bin" ]; then
    mkdir ~/bin
fi
# 2. create build dir:
if [ ! -d "~/dev" ]; then
    mkdir ~/dev
fi
if [ ! -d "~/dev/icsm" ]; then
    mkdir ~/dev/icsm
fi
# 3. download:
cd ~/dev/icsm
git clone https://github.com/icsm-au/DynAdjust.git
# 4. build:
cd ~/dev/icsm/DynAdjust/dynadjust
chmod +x ./make_dynadjust_gcc.sh
./make_dynadjust_gcc.sh
cd ~/
#
# Don't forget to add the bin directory to path in ~/.bash_profile
#    EXPORT PATH=$PATH:$HOME/.local/bin:$HOME/bin
#
# end

