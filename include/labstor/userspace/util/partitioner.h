//
// Created by lukemartinlogan on 12/6/21.
//

#ifndef LABSTOR_PARTITIONER_H
#define LABSTOR_PARTITIONER_H

//Reference: https://stackoverflow.com/questions/63372288/getting-list-of-pids-from-proc-in-linux

#include <vector>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/sysinfo.h>
#include <sched.h>

class ProcessPartitioner {
public:
    inline bool isdigit(char digit) {
        return ('0'<=digit && digit<='9');
    }

    // Helper function to check if a struct dirent from /proc is a PID folder.
    int is_pid_folder(const struct dirent *entry) {
        const char *p;
        for (p = entry->d_name; *p; p++) {
            if (!isdigit(*p))
                return false;
        }
        return true;
    }

    void InitCoreMap(std::vector<bool> &core_map, int n_cpu) {
        for(int i = 0; i < n_cpu; ++i) {
            core_map[i] = false;
        }
    }

    bool Partition(int isol_pid, std::vector<bool> &core_map, int n_cpu) {
        DIR *procdir;
        FILE *fp;
        struct dirent *entry;
        int proc_pid;

        //Initialize CPU set
        int ret = 0;
        cpu_set_t cpus[n_cpu];
        CPU_ZERO(cpus);

        // Open /proc directory.
        procdir = opendir("/proc");
        if (!procdir) {
            perror("opendir failed");
            return false;
        }

        // Iterate through all files and folders of /proc.
        while ((entry = readdir(procdir))) {
            // Skip anything that is not a PID folder.
            if (!is_pid_folder(entry))
                continue;
            //Get the PID of the running process
            int proc_pid = atoi(entry->d_name);
            //Set the affinity of the process
            if(proc_pid != isol_pid) {
                sched_getaffinity(proc_pid, n_cpu, cpus);
                for(int i = 0; i < n_cpu; ++i) {
                    if(core_map[i]) {
                        CPU_CLR(i, cpus);
                    }
                }
                sched_setaffinity(proc_pid, n_cpu, cpus);
            } else {
                for(int i = 0; i < n_cpu; ++i) {
                    if(core_map[i]) {
                        CPU_SET(i, cpus);
                    } else {
                        CPU_CLR(i, cpus);
                    }
                }
                sched_setaffinity(proc_pid, n_cpu, cpus);
            }
        }
        closedir(procdir);
        return 0;
    }
};

#endif //LABSTOR_PARTITIONER_H
