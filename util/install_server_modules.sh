#!/bin/bash

echo ${LABSTOR_BIN}
${LABSTOR_BIN}/labmods/dummy/dummy_register
${LABSTOR_BIN}/labmods/ipc_test/ipc_test_register
${LABSTOR_BIN}/labmods/mq_driver/mount.mq_driver dev::/nvme0n1 /dev/nvme0n1
${LABSTOR_BIN}/labmods/iosched/no_op/mount.no_op dev::/nvme0n1/iosched dev::/nvme0n1
#${LABSTOR_BIN}/labmods/block_fs/mount.block_fs
#${LABSTOR_BIN}/labmods/generic_posix/mount.generic_posix
#${LABSTOR_BIN}/labmods/labstor_fs/mkfs.labfs "lab::/home" "" "0"