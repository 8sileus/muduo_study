#!/bin/bash

sourceDir=$(pwd)
echo ${sourceDir}

mkdir -p ${sourceDir}/build && \
cd ${sourceDir}/build && \
cmake .. && \
make 