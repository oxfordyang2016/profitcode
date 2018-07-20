#!/bin/bash

cd
cd nick_private
git fetch origin
git reset --hard origin/master
make simtrade


ssh -i ~/.ssh/ali-privatekey root@127.0.0.1 "mkdir deploy"
cd build/bin
scp -i ~/.ssh/ali-privatekey ctpdata ctporder strat root@127.0.0.1:~/deploy
cd ~/nick_private/scripts
scp -i ~/.ssh/ali-privatekey BuildRunEnv.sh stop.sh StartData.sh StartOrder.sh StartStrat.sh root@127.0.0.1:~/deploy
