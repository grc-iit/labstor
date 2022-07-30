
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

#ifndef LABSTOR_IO_LOGGER_H
#define LABSTOR_IO_LOGGER_H

#include <list>
#include <vector>
#include <string>
#include <cstdio>
#include <sstream>

namespace labstor {

struct IOLogEntry {
    int tid_;
    size_t io_size_;
    double io_time_;
    IOLogEntry() = default;
    IOLogEntry(int tid, size_t io_size, double io_time_us) {
        tid_ = tid;
        io_size_ = io_size;
        io_time_ = io_time_us;
    }
    void Serialize(std::stringstream &ss) {
        ss << tid_;
        ss << io_size_;
        ss << io_time_;
        ss << "\n";
    }
    static void CSVHeader(std::stringstream &ss) {
        ss << "tid;io_size;io_time\n";
    }
};

class IOLogger {
private:
    std::vector<std::list<IOLogEntry>> logs_;
public:
    IOLogger(int max_threads) {
        logs_.resize(max_threads);
    }
    void Log(int tid, size_t io_size, double io_time_us) {
        logs_[tid].emplace_back(tid, io_size, io_time_us);
    }
    void ToCSV(std::string path) {
        FILE *fp = fopen(path.c_str(), "w");
        std::stringstream ss;
        IOLogEntry::CSVHeader(ss);
        for(size_t i = 0; i < logs_.size(); ++i) {
            for(auto &entry : logs_[i]) {
                entry.Serialize(ss);
            }
        }
        std::string text = std::move(ss.str());
        fwrite(text.c_str(), text.size(), 1, fp);
    }
};

}

#endif //LABSTOR_IO_LOGGER_H