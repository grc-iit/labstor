#!/bin/bash

echo ${LABSTOR_BIN}
${LABSTOR_BIN}/modules/dummy/dummy_register
${LABSTOR_BIN}/modules/kernel/mq_driver/mq_driver_register
${LABSTOR_BIN}/modules/kernel/ipc_test/ipc_test_register
${LABSTOR_BIN}/modules/filesystems/generic_posix/mount.generic_posix
${LABSTOR_BIN}/modules/filesystems/labstor_fs/mkfs.labfs "lab::/home" "" "0"