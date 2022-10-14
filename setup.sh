#!/bin/bash

sudo apt-get install libboost-all-dev
sudo apt install aptitude
aptitude search boost

SERVER_PORT=33333
GRN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color
printf "${GRN}INSTALL FINISH${NC}\ninsert SERVER_PORT: "
read SERVER_PORT
if ! echo "export SERVER_PORT="$SERVER_PORT"" >> ~/.bashrc && source ~/.bashrc; then
  printf "${RED}Error: SERVER_PORT variable not set\n"
  echo 'Try: export SERVER_PORT="$SERVER_PORT"'
  exit 1
fi
printf "${GRN}Variable set successfully${NC}\nSetup finish"