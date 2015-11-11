#!/bin/bash

cd nRF51822-BLEMIDI 
mkdir build
cd build
cmake ./../
make
./bletest
