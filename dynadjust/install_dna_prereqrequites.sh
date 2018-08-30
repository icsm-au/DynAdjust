
sudo dnf install p7zip
sudo dnf install boost
sudo dnf install boost-devel
sudo dnf install xerces-c
sudo dnf install xerces-c-devel
sudo dnf install cmake
sudo dnf install make
sudo dnf install gcc-c++
sudo dnf install xsd


sudo dnf config-manager --add-repo https://yum.repos.intel.com/mkl/setup/intel-mkl.repo
sudo rpm --import https://yum.repos.intel.com/intel-gpg-keys/GPG-PUB-KEY-INTEL-SW-PRODUCTS-2019.PUB
sudo dnf install intel-mkl-2018.1-038

