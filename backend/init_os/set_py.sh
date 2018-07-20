#!/bin/bash

sudo yum -y install wget
wget -P ~ https://pypi.python.org/packages/41/5f/6da80400340fd48ba4ae1c673be4dc3821ac06cd9821ea60f9c7d32a009f/setuptools-38.4.0.zip#md5=3426bbf31662b4067dc79edc0fa21a2e
unzip -d ~ ~/setuptools-38.4.0.zip
python ~/setuptools-38.4.0/setup.py install

wget -P ~ https://pypi.python.org/packages/11/b6/abcb525026a4be042b486df43905d6893fb04f05aac21c32c638e939e447/pip-9.0.1.tar.gz#md5=35f01da33009719497f01a4ba69d63c9
tar zxvf ~/pip-9.0.1.tar.gz -C ~
python ~/pip-9.0.1/setup.py install

pip install numpy scipy matplotlib tensorflow

wget -P ~ https://mirrors.tuna.tsinghua.edu.cn/anaconda/archive/Anaconda3-5.0.1-Linux-x86_64.sh
chmod 777 ~/Anaconda3-5.0.1-Linux-x86_64.sh
cd ~
./Anaconda3-5.0.1-Linux-x86_64.sh -y
conda install numpy python tensorflow numpy scipy
