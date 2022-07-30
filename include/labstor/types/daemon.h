
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

#ifndef LABSTOR_DAEMON_H
#define LABSTOR_DAEMON_H

#include <labstor/userspace/util/errors.h>
#include <sys/sysinfo.h>
#include <sched.h>
#include <thread>
#include <future>

namespace labstor {

class DaemonWorker {
public:
    virtual void DoWork() = 0;
};

class Daemon {
protected:
    int n_cpu_;
    int affinity_;
    std::shared_ptr<DaemonWorker> worker_;
public:
    Daemon() {
        n_cpu_ = get_nprocs_conf();
        affinity_ = -1;
    }

    void SetWorker(std::shared_ptr<DaemonWorker> worker) {
        worker_ = worker;
    }

    inline std::shared_ptr<DaemonWorker> GetWorker() {
        return worker_;
    }

    virtual void Start() = 0;
    virtual void Pause() = 0;
    virtual void Resume() = 0;
    virtual void Wait() = 0;
    virtual void Stop() = 0;
    virtual void SetAffinity(int cpu_id) = 0;
};

}

#endif //LABSTOR_DAEMON_H