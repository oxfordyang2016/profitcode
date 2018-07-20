#!/bin/bash

if [ ! -d "/run" ];then
mkdir /run
else
echo "run exiseted!"
fi

date_string=`date --rfc-3339=date`
if [ ! -d /run/$date_string ];then
mkdir /run/$date_string
echo "/run/"$date_string" exiseted!"
fi

cd /run/$date_string
mkdir bin
mkdir log
mkdir scripts

cd ~/deploy
cp ctpdata ctporder strat  /run/$date_string/bin/
cp BuildRunEnv.sh stop.sh  /run/$date_string/scripts/

rm /today
ln -s  /run/$date_string /today

yum -y install zsh
