#!/bin/bash

echo ${LABSTOR_BIN}
${LABSTOR_BIN}/labmods/dummy/dummy_register
${LABSTOR_BIN}/labmods/kernel/mq_driver/mq_driver_register
${LABSTOR_BIN}/labmods/kernel/ipc_test/ipc_test_register
${LABSTOR_BIN}/labmods/filesystems/generic_posix/mount.generic_posix
${LABSTOR_BIN}/labmods/filesystems/labstor_fs/mkfs.labfs "lab::/home" "" "0"