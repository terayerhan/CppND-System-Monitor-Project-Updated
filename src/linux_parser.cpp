#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>

#include "linux_parser.h"

#include <filesystem>

using std::stof;
using std::string;
using std::to_string;
using std::vector;

namespace fs = std::filesystem;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  std::vector<int> pids;
    fs::path proc_path = kProcDirectory;

    for (const auto& entry : fs::directory_iterator(proc_path)) {
        if (entry.is_directory()) {
            std::string dir_name = entry.path().filename().string();
            if (std::all_of(dir_name.begin(), dir_name.end(), ::isdigit)) {
                // Convert directory name to PID
                int pid = std::stoi(dir_name);
                pids.push_back(pid);
            }
        }
    }

    return pids;
}

// TODO: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() { 
  std::ifstream meminfo(kProcDirectory + kMeminfoFilename);
  std::string line;
  std::string key;
  unsigned long long total_memory = 0;
  unsigned long long free_memory = 0;

  if (meminfo.is_open()) {
      while (std::getline(meminfo, line)) {
      std::istringstream iss(line);
      iss >> key;
        if (key == "MemTotal:") {
            iss >> total_memory;
        } else if (key == "MemFree:") {
            iss >> free_memory;
        }
      }
  }

  if (total_memory == 0) {
      return -1.0f; // Avoid division by zero
  }

  // Calculate memory utilization percentage
  float used_memory = static_cast<float>(total_memory - free_memory);
  return (used_memory / total_memory); 
}

// TODO: Read and return the system uptime
long LinuxParser::UpTime() { 
  std::ifstream file(kProcDirectory + kUptimeFilename);
  double uptime_seconds = 0;
  if (file.is_open()) {
      file >> uptime_seconds;

      file.close(); // Close the file after reading
  }

  return static_cast<long>(uptime_seconds); 
}

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { 
  std::ifstream stat_file(kProcDirectory + kStatFilename);
    if (stat_file.is_open()) {
        std::string line;
        std::getline(stat_file, line);  // Read the first line (CPU line)
        
        std::istringstream iss(line);
        std::string cpu_label;
        long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
        iss >> cpu_label >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice;

        // Total jiffies include all CPU times excluding guest times
        return (user + nice + system + idle + iowait + irq + softirq + steal);
    }
    return 0; 
}

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid[[maybe_unused]]) { return 0; }

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() { 
  std::ifstream stat_file(kProcDirectory + kStatFilename);
    if (stat_file.is_open()) {
        std::string line;
        std::getline(stat_file, line);  // Read the first line (CPU line)
        
        std::istringstream iss(line);
        std::string cpu_label;
        long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
        iss >> cpu_label >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice;

        // Active jiffies exclude idle and iowait times
        long active_jiffies = user + nice + system + irq + softirq + steal;
        return active_jiffies;
    }
    return 0; 
}

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() { return 0; }

// TODO: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() { return {}; }

// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() { 
  std::ifstream file(kProcDirectory + kStatFilename);
  std::string line;
  int total_processes = 0;

  if (file.is_open()) {
      while (std::getline(file, line)) {
          if (line.compare(0, 9, "processes") == 0) {
              // Extract the total number of processes created since boot
              std::istringstream iss(line);
              std::string key;
              iss >> key >> total_processes;
              break;
          }
      }
      file.close();
  } 

  return total_processes;
}

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  std::ifstream file(kProcDirectory + kStatFilename);
  std::string line;
  int running_processes = 0;

  if (file.is_open()) {
      while (std::getline(file, line)) {
          if (line.compare(0, 13, "procs_running") == 0) {
              // Extract the number of running processes
              std::istringstream iss(line);
              std::string key;
              iss >> key >> running_processes;
              break;
          }
      }
      file.close(); // Explicitly close the file
  } 

  return running_processes; 
}

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid[[maybe_unused]]) { return string(); }

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid[[maybe_unused]]) { return string(); }

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid[[maybe_unused]]) { return string(); }

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid[[maybe_unused]]) { return string(); }

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid[[maybe_unused]]) { return 0; }
