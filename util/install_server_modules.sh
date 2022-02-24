#!/bin/bash

${LABSTOR_BIN}/modules/dummy/dummy_register
${LABSTOR_BIN}/modules/kernel/mq_driver/mq_driver_register
${LABSTOR_BIN}/modules/kernel/ipc_test/ipc_test_register
${LABSTOR_BIN}/modules/filesystems/generic_posix/mount.generic_posix
