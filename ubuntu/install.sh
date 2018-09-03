#!/bin/sh
cat <<EOD > /etc/apt/sources.list.d/intelproducts.list
deb https://apt.repos.intel.com/intelpython binary/
deb https://apt.repos.intel.com/mkl all main
deb https://apt.repos.intel.com/ipp all main
deb https://apt.repos.intel.com/tbb all main
deb https://apt.repos.intel.com/daal all main
deb https://apt.repos.intel.com/mpi all main
EOD
wget https://apt.repos.intel.com/intel-gpg-keys/GPG-PUB-KEY-INTEL-SW-PRODUCTS-2019.PUB
apt-key add GPG-PUB-KEY-INTEL-SW-PRODUCTS-2019.PUB
rm GPG-PUB-KEY-INTEL-SW-PRODUCTS-2019.PUB
apt-get update
apt-get install -y \
    g++ \
    cmake \
    libboost-all-dev \
    intel-mkl-2018.3-051 \
    libxerces-c-dev \
    xsdcxx \
