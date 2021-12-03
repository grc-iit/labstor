#!bin/bash

#LOG_DIR: the directory where you will store output logs

TEST_ROOT=$1
TEST_CASE=$2

DF=${TEST_CASE}
python3 ${TEST_ROOT}/parse.py ${LOG_DIR}/${DF}.txt ${LOG_DIR}/${DF}.json
python3 ${TEST_ROOT}/to_hier.py ${LOG_DIR}/${DF}.json ${LOG_DIR}/${DF}.json.collapsed

