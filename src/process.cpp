#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "process.h"
#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

Process::Process(int pid) : pid_(pid), valid_(true), lastTotalTime_(0), lastTotalSystemTime_(0), cpuUtilization_(0.0) {
        updateInfo();
}

bool Process::isValid() const {
        return valid_;
}

float Process::getCpuUtilization(unsigned long long totalSystemTime) {
    if (hasChanged()) {
        updateInfo();
    }
    if (lastTotalSystemTime_ > 0) {
        unsigned long long deltaTotalTime = totalTime_ - lastTotalTime_;
        unsigned long long deltaSystemTime = totalSystemTime - lastTotalSystemTime_;
        cpuUtilization_ = (deltaTotalTime *1.0) / deltaSystemTime;
    }
    lastTotalTime_ = totalTime_;
    lastTotalSystemTime_ = totalSystemTime;
    return cpuUtilization_;
}

void Process::updateInfo() {
    // Read /proc/[pid]/stat to get the latest CPU times and process info
    std::ifstream file(LinuxParser::kProcDirectory + std::to_string(pid_) + 
                       LinuxParser::kStatFilename);
    if (!file.is_open()) {
        valid_ = false;
        return;
    }

    std::string line;
    if (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string token;

        // Skip fields until utime (14th field)
        for (int i = 0; i < 13; ++i) iss >> token;

        unsigned long long utime, stime;
        if (iss >> utime >> stime) {
            totalTime_ = utime + stime;
        }

        // Skip fields until starttime (22nd field)
        for (int i = 0; i < 7; ++i) iss >> token;
        if (iss >> startTime_) {
            valid_ = true;
        }
    } else {
        valid_ = false;
    }
}

bool Process::hasChanged() {
    std::ifstream file(LinuxParser::kProcDirectory + std::to_string(pid_) + 
                       LinuxParser::kStatFilename);
    if (!file.is_open()) {
        return true;
    }

    std::string line;
    if (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string token;
        for (int i = 0; i < 21; ++i) iss >> token;
        unsigned long long currentStartTime;
        if (iss >> currentStartTime) {
            return currentStartTime != startTime_;
        }
    }
    return true;
}

// TODO: Return this process's ID
int Process::Pid() { return pid_; }

// TODO: Return this process's CPU utilization
float Process::CpuUtilization() { return totalTime_ - lastTotalTime_; }

// TODO: Return the command that generated this process
string Process::Command() { return string(); }

// TODO: Return this process's memory utilization
string Process::Ram() { return string(); }

// TODO: Return the user (name) that generated this process
string Process::User() { return string(); }

// TODO: Return the age of this process (in seconds)
long int Process::UpTime() { return 0; }

// TODO: Overload the "less than" comparison operator for Process objects
// REMOVE: [[maybe_unused]] once you define the function
bool Process::operator>(Process const& a) const { 
    return cpuUtilization_ > a.cpuUtilization_; 
}