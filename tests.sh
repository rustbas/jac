#!/bin/bash

# set -xe

result_name="test.txt"

for file in data/*.txt
do
    archive_name=$(basename "${file}.jacz")
    echo ${file} ${archive_name}
    time -p ./main.out -c -i ${file} -o ${archive_name}
    time -p ./main.out -x -i ${archive_name} -o ${result_name}
    diff ${file} ${result_name}
    rm ${result_name}
done
