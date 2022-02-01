//
// Created by lukemartinlogan on 1/31/22.
//

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
