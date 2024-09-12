#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "process.h"
#include "linux_parser.h"
#include <algorithm>
#include <thread>
#include <chrono>

using std::string;
using std::to_string;
using std::vector;

Process::Process(int pid) : pid_(pid), valid_(true), cpuUtilization_(0.0), lastTotalTime_(0)  {
        //updateInfo();
        MemoryUtilization();
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

void Process::MemoryUtilization() {
    std::ifstream file(LinuxParser::kProcDirectory + std::to_string(pid_) + 
                        LinuxParser::kStatusFilename);
    if (!file.is_open()) {
        memoryUtilization_ = 0;
        return ;  // If file can't be opened, return 0 memory usage
    }

    std::string line;
    const std::string key = "VmRSS:";
    while (std::getline(file, line)) {
        if (line.compare(0, key.size(), key) == 0) {  // Check if the line starts with "VmRSS:"
            // Find the position of the memory value and extract it directly
            std::size_t pos = line.find_first_of("0123456789");
            if (pos != std::string::npos) {
                long memory_kb = std::stof(line.substr(pos));
                memoryUtilization_ = memory_kb / 1024.0;  // Convert to MB
            }
            break;  // Exit the loop once "VmRSS:" is found
        }
    }    
}

// TODO: Return this process's ID
int Process::Pid() { return pid_; }

// TODO: Return this process's CPU utilization
float Process::CpuUtilization() { 
    // float cpuUtilization = 0;
    // long initialProcessJiffies = LinuxParser::ActiveJiffies(pid_);
    // long initialSystemJiffies = LinuxParser::Jiffies();

    // std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // long finalProcessJiffies = LinuxParser::ActiveJiffies(pid_);
    // long finalSystemJiffies = LinuxParser::Jiffies();

    // long deltaProcessJiffies = finalProcessJiffies - initialProcessJiffies;
    // long deltaSystemJiffies = finalSystemJiffies - initialSystemJiffies;

    // if (deltaSystemJiffies > 0) {
    //     cpuUtilization = deltaProcessJiffies / deltaSystemJiffies;
    // }

    // return cpuUtilization;

    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal; //, guest, guest_nice;
    unsigned long long user_old, nice_old, system_old, idle_old, iowait_old, irq_old, softirq_old, steal_old; // guest_old, guest_nice_old;
    unsigned long long process_time[2], process_time_old[2];

    auto read_cpu_stats = [&]() {
        std::ifstream stat_file("/proc/stat");
        std::string cpu_label;
        stat_file >> cpu_label
                  >> user >> nice >> system >> idle
                  >> iowait >> irq >> softirq >> steal
                  >> cpu_label >> cpu_label;
    };

    auto read_process_stats = [&]() {
        std::ifstream stat_file("/proc/" + std::to_string(pid_) + "/stat");
        std::string unused;
        for (int i = 0; i < 13; ++i) {
            stat_file >> unused;
        }
        stat_file >> process_time[0] >> process_time[1];
    };

    read_cpu_stats();
    read_process_stats();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    user_old = user; nice_old = nice; system_old = system; idle_old = idle;
    iowait_old = iowait; irq_old = irq; softirq_old = softirq; steal_old = steal;
    //guest_old = guest; guest_nice_old = guest_nice; 
    process_time_old[0] = process_time[0];
    process_time_old[1] = process_time[1];

    read_cpu_stats();
    read_process_stats();

    unsigned long long total_time = user + nice + system + idle + iowait + irq + softirq + steal;
    unsigned long long total_time_old = user_old + nice_old + system_old + idle_old + 
                                        iowait_old + irq_old + softirq_old + steal_old;

    unsigned long long total_delta = total_time - total_time_old;
    unsigned long long process_delta = (process_time[0] + process_time[1]) - 
                                       (process_time_old[0] + process_time_old[1]);

    return static_cast<float>(process_delta) / total_delta;
}

// TODO: Return the command that generated this process
string Process::Command() {     
    return LinuxParser::Command(pid_);    
}

// TODO: Return this process's memory utilization
string Process::Ram() { 
    return std::to_string(memoryUtilization_); 
}

// TODO: Return the user (name) that generated this process
string Process::User() { 
    return LinuxParser::User(pid_); 
}

// TODO: Return the age of this process (in seconds)
long int Process::UpTime() { 
    return LinuxParser::UpTime(pid_); 
}

// TODO: Overload the "less than" comparison operator for Process objects
// REMOVE: [[maybe_unused]] once you define the function
bool Process::operator>(Process const& a) const { 
    return memoryUtilization_ > a.memoryUtilization_; 
}