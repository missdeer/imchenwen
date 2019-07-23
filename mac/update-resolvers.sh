#!/bin/bash
brew update 
brew upgrade annie youtube-dl
pip3 install --upgrade you-get
git clone https://github.com/zhangn1985/ykdl.git
cd ykdl
python3 setup.py install
cd ..
rm -rf ykdl
