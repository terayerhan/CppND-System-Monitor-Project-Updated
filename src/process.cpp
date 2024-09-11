#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "process.h"
#include "linux_parser.h"
#include <algorithm>

using std::string;
using std::to_string;
using std::vector;

Process::Process(int pid) : pid_(pid), valid_(true), lastTotalTime_(0), cpuUtilization_(0.0) {
        updateInfo();
}

bool Process::isValid() const {
        return valid_;
}

float Process::getCpuUtilization(unsigned long long totalSystemTime, unsigned long long lastTotalSystemTime) {
    if (hasChanged()) {
        updateInfo();
    }
    else {
        updateInfo();
        unsigned long long deltaSystemTime = totalSystemTime - lastTotalSystemTime;

        if (deltaSystemTime > 0) {
            unsigned long long deltaTotalTime = totalTime_ - lastTotalTime_;            
            cpuUtilization_ = (deltaTotalTime *1.0) / deltaSystemTime;
        }

    }
    
    lastTotalTime_ = totalTime_;
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

float Process::MemoryUtilization() {
    std::ifstream file(LinuxParser::kProcDirectory + std::to_string(pid_) + 
                        LinuxParser::kStatusFilename);
    if (!file.is_open()) {
        return 0.0;  // If file can't be opened, return 0 memory usage
    }

    std::string line;
    const std::string key = "VmRSS:";
    while (std::getline(file, line)) {
        if (line.compare(0, key.size(), key) == 0) {  // Check if the line starts with "VmRSS:"
            // Find the position of the memory value and extract it directly
            std::size_t pos = line.find_first_of("0123456789");
            if (pos != std::string::npos) {
                float memory_kb = std::stof(line.substr(pos));
                memoryUtilization_ = memory_kb / 1024.0;  // Convert to MB
            }
            break;  // Exit the loop once "VmRSS:" is found
        }
    }
    return memoryUtilization_;
}

// TODO: Return this process's ID
int Process::Pid() { return pid_; }

// TODO: Return this process's CPU utilization
float Process::CpuUtilization() { return cpuUtilization_; }

// TODO: Return the command that generated this process
string Process::Command() { 
    std::ifstream cmdline_file(LinuxParser::kProcDirectory + std::to_string(pid_) +
                                LinuxParser::kCmdlineFilename);
    
    if (!cmdline_file.is_open()) {
        return "Error: Unable to open cmdline file";
    }
    
    std::string command;
    std::getline(cmdline_file, command, '\0');
    
    // Trim trailing spaces
    command.erase(command.find_last_not_of(" \n\r\t") + 1);
    
    return command;  
    
}

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