
sudo apt-get install p7zip
sudo apt-get install libboost-all-dev
sudo apt-get install libxerces-c-dev
sudo apt-get install cmake
sudo apt-get install make
sudo apt-get install g++
sudo apt-get install xsdcxx


wget https://apt.repos.intel.com/intel-gpg-keys/GPG-PUB-KEY-INTEL-SW-PRODUCTS-2019.PUB
sudo apt-key add GPG-PUB-KEY-INTEL-SW-PRODUCTS-2019.PUB
sudo sh -c 'echo deb https://apt.repos.intel.com/mkl all main > /etc/apt/sources.list.d/intel-mkl.list'
sudo apt-get update && sudo apt-get install intel-mkl-64bit-2018.1-038

