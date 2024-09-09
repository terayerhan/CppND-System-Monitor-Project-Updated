#include <unistd.h>
#include <cstddef>
#include <set>
#include <string>
#include <vector>

#include "process.h"
#include "processor.h"
#include "system.h"

#include "linux_parser.h"
#include <chrono>
#include <filesystem>

using std::set;
using std::size_t;
using std::string;
using std::vector;

// TODO: Return the system's CPU
Processor& System::Cpu() { return cpu_; }

// TODO: Return a container composed of the system's processes
vector<Process>& System::Processes() {
    unsigned long long totalSystemTime = LinuxParser::Jiffies();

    // Get the current time within updateProcesses
    unsigned long long currentTime = std::chrono::steady_clock::now().time_since_epoch() / std::chrono::milliseconds(1);

    // Only update the existing processes, removing invalid ones
    for (auto it = processes_.begin(); it != processes_.end();) {
        if (!it->isValid() || it->hasChanged()) {
            it = processes_.erase(it);
        } else {
            it->getCpuUtilization(totalSystemTime, currentTime);
            ++it;
        }
    }

    // Add new processes
    for (const auto& entry : std::filesystem::directory_iterator(LinuxParser::kProcDirectory)) {
        if (entry.is_directory() && std::isdigit(entry.path().filename().string()[0])) {
            int pid = std::stoi(entry.path().filename().string());

            // Check if the process is already tracked
            auto it = std::find_if(processes_.begin(), processes_.end(),
                                    [pid](Process& p) { return p.Pid() == pid; });

            if (it == processes_.end()) { // Add new process if not already tracked
                Process process(pid);
                if (process.isValid()) {
                    process.getCpuUtilization(totalSystemTime, currentTime);
                    processes_.push_back(std::move(process));
                }
            }
        }
    }

    // Sort processes by CPU utilization in descending order
    std::sort(processes_.begin(), processes_.end(), std::greater<Process>());

    return processes_; 
}

// TODO: Return the system's kernel identifier (string)
std::string System::Kernel() { return LinuxParser::Kernel(); }

// TODO: Return the system's memory utilization
float System::MemoryUtilization() { return LinuxParser::MemoryUtilization(); }

// TODO: Return the operating system name
std::string System::OperatingSystem() { return LinuxParser::OperatingSystem(); }

// TODO: Return the number of processes actively running on the system
int System::RunningProcesses() { return LinuxParser::RunningProcesses(); }

// TODO: Return the total number of processes on the system
int System::TotalProcesses() { return LinuxParser::TotalProcesses(); }

// TODO: Return the number of seconds since the system started running
long int System::UpTime() { return LinuxParser::UpTime(); }