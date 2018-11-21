#!/usr/bin/env bash

set -e

sudo add-apt-repository multiverse
sudo apt-get -y update

sudo apt-get -y install p7zip
sudo apt-get -y install libboost-all-dev
sudo apt-get -y install libxerces-c-dev
sudo apt-get -y install cmake
sudo apt-get -y install make
sudo apt-get -y install g++
sudo apt-get -y install xsdcxx
sudo apt-get -y install git
sudo apt-get -y install xorg
sudo apt-get -y install vim-gnome

wget https://apt.repos.intel.com/intel-gpg-keys/GPG-PUB-KEY-INTEL-SW-PRODUCTS-2019.PUB
sudo apt-key add GPG-PUB-KEY-INTEL-SW-PRODUCTS-2019.PUB
sudo sh -c 'echo deb https://apt.repos.intel.com/mkl all main > /etc/apt/sources.list.d/intel-mkl.list'
sudo apt-get -y update
sudo apt-get -y install intel-mkl-64bit-2018.1-038

