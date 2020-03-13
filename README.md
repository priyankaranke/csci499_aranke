1) Create a new directory and clone csci499_aranke repo into that using 'git clone https://github.com/priyankaranke/csci499_aranke.git'

2) cd csci499_aranke  
   vagrant up
   vagrant ssh

3) If the commands from bootstrap.sh didn't run automatically on ssh, copy paste it 
all into terminal and let the packages install

4) Install CMake using instructions on 'Install CMake through the Ubuntu Command Line'
section on 'https://vitux.com/how-to-install-cmake-on-ubuntu-18-04/'

5) Get glog: 
'git clone https://github.com/google/glog.git'
cd glog
cmake -H. -Bbuild -G "Unix Makefiles"
cmake --build build

6) Build using:
mkdir build
cd build
cmake ..
make