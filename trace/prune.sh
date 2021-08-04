#!bin/bash

#LOG_DIR: the directory where you will store output logs
#THRESHOLD: the minimum fraction of time that a function must take in order to be displayed

TEST_ROOT=$1
TEST_CASE=$2

DF=${TEST_CASE}
python3 ${TEST_ROOT}/prune.py ${LOG_DIR}/${DF}.json.collapsed ${LOG_DIR}/${DF}.json.collapsed.pruned ${THRESHOLD}
