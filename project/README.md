# csci499_aranke

Build instructions 

Step 1: Getting the right VM 
1) Install vagrantbox
2) Do https://app.vagrantup.com/ubuntu/boxes/bionic64 to get the right box
3) vagrant ssh into your new machine!
4) Make sure it can compile .cpp files with 'sudo apt-get install g++'

Step 2: Get CMake as your build system using the "Install CMake through the Ubuntu Command Line" 
found here at https://vitux.com/how-to-install-cmake-on-ubuntu-18-04/

Step 3: Install Protocol Buffers and GRPC. The setup is quite tedious (taken from 
https://github.com/IvanSafonov/grpc-cmake-example) but pays dividends in that everything is 
derived from 'first principles' as much as possible (i.e. directly from the .proto files and 
we don't have to lug around too many .cpp, .h files)

1) sudo apt-get install build-essential autoconf libtool pkg-config automake curl  
2) git clone -b $(curl -L https://grpc.io/release) https://github.com/grpc/grpc  
cd grpc  
git submodule update --init  
3) cd ./third_party/protobuf  
./autogen.sh  
./configure --prefix=/opt/protobuf  
make -j `nproc`  
sudo make install  
4) cd ../..  
make -j `nproc` PROTOC=/opt/protobuf/bin/protoc   
sudo make prefix=/opt/grpc install  

Step 4: Clone this repo into your Vagrant box under /vagrant/
Now we have all our prerequisities installed. 

Step 5: 
1) Build it using: 

mkdir build  
cd build  
cmake ..  
make  

2) Run key-value store using ./Warble

Note: I took the config from this repo (https://github.com/IvanSafonov/grpc-cmake-example) which 
I figured was okay since this is setup/config and not central to Warble or core functionalities

TODO: Remove before final phase 1 submission  
Progress doc: 
1) Setup Vagrant project successfully
2) Enabled file syncing and created a file in host OS, made sure it appeared on guest OS and 
vice versa
3) Wrote a simple .cpp file and get it to compile on guest OS (Didn't work until 
'sudo apt-get install g++')
4) Installed CMake, GRPC, Protocol buffers
5) Write CMakeLists.txt and cmake/ for things to work
6) Compiled and ran test_file.cpp 
7) Examined the generated files for backend_kv_store.proto to make sure they're legit
8) This hopefully covers most of the setup, infrastructure 

What's next:
1) Create the actual Backend key-value service and accompanying Gtests by Friday.
2) Figure out logging 
