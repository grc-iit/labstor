//
// Created by lukemartinlogan on 2/9/22.
//

#include <labstor/userspace/util/timer.h>
#include <labstor/userspace/types/userspace_daemon.h>

struct shmem_stats {
    size_t ns_since_start_;
};

class StatsWorker : public labstor::DaemonWorker {
private:
    labstor::HighResMonotonicTimer t_;
    int region_id_;
    shmem_stats *stats_;
public:
    StatsWorker() {
        t_.Resume();
        //Create SHMEM region
    }

    int GetRegionID() {
    }

    void DoWork() {
        stats_->ns_since_start_ = (size_t)t_.GetNsecFromStart();
    }
};