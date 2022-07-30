
/*
 * Copyright (C) 2022  SCS Lab <scslab@iit.edu>,
 * Luke Logan <llogan@hawk.iit.edu>,
 * Jaime Cernuda Garcia <jcernudagarcia@hawk.iit.edu>
 * Jay Lofstead <gflofst@sandia.gov>,
 * Anthony Kougkas <akougkas@iit.edu>,
 * Xian-He Sun <sun@iit.edu>
 *
 * This file is part of LabStor
 *
 * LabStor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef LABSTOR_KERNEL_CLIENT_H
#define LABSTOR_KERNEL_CLIENT_H

#include <pthread.h>
#include <memory>
#include <vector>

#include <unistd.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#include <labstor/types/basics.h>

#define NETLINK_USER 31

namespace labstor::Kernel {

class NetlinkClient {
private:
    int sockfd_;
public:
    inline bool IsConnected() { return sockfd_ >= 0; }
    bool Connect();
    static bool SendMSG(void *serialized_buf, size_t buf_size);
    static bool RecvMSG(void *buf, size_t buf_size);
};

}

#endif //LABSTOR_KERNEL_CLIENT_H