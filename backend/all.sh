#!/bin/bash

#~/nick_private/backend/editor/install_command

cp ~/nick_private/backend/editor/zshrc ~
mv ~/zshrc ~/.zshrc
source ~/.zshrc

~/nick_private/backend/git/gitconfig.sh
~/nick_private/backend/init_os/all_install.sh

~/nick_private/backend/setupzeromq.sh
