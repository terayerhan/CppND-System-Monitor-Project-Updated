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

float Process::getCpuUtilization(unsigned long long totalSystemTime, unsigned long long currentTime) {
    if (hasChanged()) {
        updateInfo();
    }
    if (lastTotalSystemTime_ > 0) {
        unsigned long long deltaTotalTime = totalTime_ - lastTotalTime_;
        unsigned long long deltaSystemTime = totalSystemTime - lastTotalSystemTime_;
        cpuUtilization_ = (deltaTotalTime * 100.0) / deltaSystemTime;
    }
    lastTotalTime_ = totalTime_;
    lastTotalSystemTime_ = totalSystemTime;
    return cpuUtilization_;
}

// TODO: Return this process's ID
int Process::Pid() { return pid_; }

// TODO: Return this process's CPU utilization
float Process::CpuUtilization() { 
    long currentTotalJiffies = LinuxParser::Jiffies();
    long currentActiveJiffies = LinuxParser::ActiveJiffies(pid_);

    long totalDiff = currentTotalJiffies - previousTotalJiffies_;
    long activeDiff = currentActiveJiffies - previousActiveJiffies_;

    if(totalDiff == 0){ return 0.0;}

    // Cache the current values in the Processor object.
    previousTotalJiffies_ = currentTotalJiffies;
    previousActiveJiffies_ = currentActiveJiffies;

    return static_cast<float>(activeDiff) / totalDiff;  
}

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
bool Process::operator<(Process const& a[[maybe_unused]]) const { return true; }