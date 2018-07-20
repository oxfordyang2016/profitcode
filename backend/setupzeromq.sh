#!/bin/bash

wget -P ~ https://github.com/zeromq/zeromq4-1/releases/download/v4.1.2/zeromq-4.1.2.tar.gz
cd
tar zxvf zeromq-4.1.2.tar.gz
cd zeromq-4.1.2
./configure --without-libsodium
sudo make
sudo make install
sudo cp ~/nick_private/tutorial/zeromq/zmq.hpp /usr/local/include
wget -P ~ https://download.libsodium.org/libsodium/releases/libsodium-1.0.13.tar.gz
cp ~/nick_private/backend/libsodium-1.0.13.tar.gz ~
cd
tar zxvf libsodium-1.0.13.tar.gz
cd libsodium-1.0.13
./configure
sudo make
sudo make install
cd /usr/local/lib
sudo mv libsodium.so.18 libsodium.so.4
