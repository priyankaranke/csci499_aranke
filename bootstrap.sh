sudo apt-get update
sudo apt-get -y install g++
sudo apt-get -y install build-essential autoconf libtool pkg-config automake curl
sudo apt-get -y install libgtest-dev

git clone -b $(curl -L https://grpc.io/release) https://github.com/grpc/grpc
cd grpc
git submodule update --init

cd ./third_party/protobuf
./autogen.sh
./configure --prefix=/opt/protobuf
make -j nproc
sudo make install

cd ../..
make -j nproc PROTOC=/opt/protobuf/bin/protoc
sudo make prefix=/opt/grpc install
