#!/bin/bash
yum -y install zsh vim git gcc-c++ gcc wget
yum -y install "boost*"
yum -y install "libconfig*"
yum -y install R
sudo chkconfig ntpd on
sudo service ntpd start
sudo mkdir /zentero
sudo chown zentero:zentero /zentero
mkdir /zentero/deploy
mkdir /zentero/live
sudo service iptables stop
sudo chkconfig iptables off
sudo yum -y install ntp ntpdate
sudo ntpdate cn.pool.ntp.org
sudo hwclock --systohc
sudo hwclock -w
