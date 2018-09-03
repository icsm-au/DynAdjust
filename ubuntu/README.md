Ubuntu build environment
========================

To build using a vagrant managed virtual machine use:

``` sh
vagrant up
vagrant ssh -c /vagrant/build_dynadjust.sh
```

dynadjust will be built in the build subdirectory of the vagrant user.
Currently this is configured for ubuntu 18.04 (bionic).  Edit Vagrantfile
to choose a different distribution. eg:

``
config.vm.box = "ubuntu/xenial64"
```

To build on the host machine use

``` sh
# Install build dependencies
sudo install.sh
# Create build directory and build
mkdir build
cd build 
cmake ../../dynadjust
make
```
