#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>

#include "linux_parser.h"

//#include <filesystem>
#include <pwd.h>

using std::stof;
using std::string;
using std::to_string;
using std::vector;

//namespace fs = std::filesystem;

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
// vector<int> LinuxParser::Pids() {
//   std::vector<int> pids;
//     fs::path proc_path = kProcDirectory;

//     for (const auto& entry : fs::directory_iterator(proc_path)) {
//         if (entry.is_directory()) {
//             std::string dir_name = entry.path().filename().string();
//             if (std::all_of(dir_name.begin(), dir_name.end(), ::isdigit)) {
//                 // Convert directory name to PID
//                 int pid = std::stoi(dir_name);
//                 pids.push_back(pid);
//             }
//         }
//     }

//     return pids;
// }
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}


// TODO: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() { 
  std::ifstream meminfo(kProcDirectory + kMeminfoFilename);
    if (!meminfo.is_open()) {
        return -1.0; // Unable to open file
    }

    std::string line;
    unsigned long long total_memory = 0;
    unsigned long long free_memory = 0;
    unsigned long long buffers = 0;
    unsigned long long cached = 0;
    unsigned long long sReclaimable = 0;

    while (std::getline(meminfo, line)) {
        if (line.compare(0, 9, "MemTotal:") == 0) {
            total_memory = std::stoull(line.substr(9));
        } else if (line.compare(0, 8, "MemFree:") == 0) {
            free_memory = std::stoull(line.substr(8));
        } else if (line.compare(0, 8, "Buffers:") == 0) {
            buffers = std::stoull(line.substr(8));
        } else if (line.compare(0, 7, "Cached:") == 0) {
            cached = std::stoull(line.substr(7));
        } else if (line.compare(0, 13, "SReclaimable:") == 0) {
            sReclaimable = std::stoull(line.substr(13));
        }

        if (total_memory && free_memory && buffers && cached && sReclaimable) {
            break;  // We have all the information we need
        }
    }

    if (total_memory == 0) {
        return -1.0; // Avoid division by zero
    }

    // Calculate used memory, considering buffers and cache
    unsigned long long used_memory = total_memory - free_memory - buffers - cached - sReclaimable;

    // Calculate memory utilization as a percentage
    return static_cast<float>(used_memory) / total_memory; 
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
long LinuxParser::ActiveJiffies(int pid) {   
  std::ifstream statFile(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (!statFile.is_open()) {
      return 0; // Return 0 instead of throwing an exception
  }
  std::string line;
  if (std::getline(statFile, line)) {
      std::istringstream iss(line);
      std::string token;
      // Skip the first 13 fields
      for (int i = 0; i < 13; ++i) {
          iss >> token;
      }
      long utime, stime;
      if (iss >> utime >> stime) {
          return utime + stime;
      }
  }
  return 0; // Return 0 if unable to read or parse the file
}

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
long LinuxParser::IdleJiffies() { 
  std::ifstream filestream(kProcDirectory + kStatFilename);
  std::string line;
  std::string cpu;
  long idleJiffies = 0;

  if (filestream.is_open()) {
      std::getline(filestream, line);
      std::istringstream linestream(line);
      linestream >> cpu;  // Read the "cpu" label
      long user, nice, system, idle;

      // Read the CPU jiffies
      linestream >> user >> nice >> system >> idle;

      // Idle jiffies are the 4th field (idle time)
      idleJiffies = idle;

      filestream.close();
  }

  return idleJiffies; 
}

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
string LinuxParser::Command(int pid) { 
  std::ifstream cmdline_file(kProcDirectory + std::to_string(pid) + kCmdlineFilename);
    if (!cmdline_file.is_open()) {
        return "";  // Return an empty string if the file cannot be opened
    }

    std::string command;
    std::getline(cmdline_file, command, '\0');  // Read the command line

    // Replace null characters with spaces (as arguments are separated by '\0')
    std::replace(command.begin(), command.end(), '\0', ' ');

    // Check if trimming is necessary by inspecting the last character
    if (!command.empty() && isspace(command.back())) {
        // Trim trailing whitespace
        command.erase(command.find_last_not_of(" \n\r\t") + 1);
    }

    return command; 
}

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid) { 
  std::ifstream status_file(kProcDirectory + std::to_string(pid) + kStatusFilename);
    if (!status_file.is_open()) {
        return "";  // Return empty string if the file can't be opened
    }

    std::string line, key;
    const std::string target_key = "VmRSS:";
    long memory_kb = 0;

    // Optimized loop to find "VmRSS" and extract memory value in kilobytes
    while (std::getline(status_file, line)) {
        if (line.compare(0, target_key.size(), target_key) == 0) {
            // Find the position of the first digit and extract the memory in KB
            std::size_t pos = line.find_first_of("0123456789");
            if (pos != std::string::npos) {
                memory_kb = std::stol(line.substr(pos));  // Extract and convert to long
            }
            break;  // Exit the loop once the memory value is found
        }
    }

    // Convert KB to MB and return the result as a string
    return std::to_string(memory_kb / 1024);
}

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid) { 
  std::ifstream status_file(kProcDirectory + std::to_string(pid) + kStatusFilename);
    if (!status_file.is_open()) {
        return "";  // Return empty string if the file can't be opened
    }

    std::string line;
    const std::string target_key = "Uid:";

    // Read and parse the status file to find the UID
    while (std::getline(status_file, line)) {
        if (line.compare(0, target_key.size(), target_key) == 0) {
            std::istringstream iss(line);
            std::string key, uid_str;
            iss >> key >> uid_str;
            if (!uid_str.empty()) {
                return uid_str;  // Return UID as string
            }
            break;  // Exit loop if UID is empty or not found
        }
    }

    // Return an empty string if UID is not found
    return "";

}

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid) { 
  // Retrieve UID
    std::string uid_str = Uid(pid);
    if (uid_str.empty()) {
        return "unknown";  // Return "unknown" if UID could not be retrieved
    }

    // Convert UID to username
    uid_t uid = static_cast<uid_t>(std::stoi(uid_str));
    struct passwd* pw = getpwuid(uid);
    return pw ? pw->pw_name : "unknown";  // Return "unknown" if the username cannot be found

}

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid) { 
  std::ifstream stat_file(kProcDirectory + std::to_string(pid) + kStatFilename);
    if (!stat_file.is_open()) {
        return 0;  // Return 0 if the file cannot be opened
    }

    std::string line;
    std::getline(stat_file, line);
    std::istringstream linestream(line);

    std::string ignore;
    long starttime = 0;

    // Skip the first 21 fields
    for (int i = 0; i < 21; ++i) {
        linestream >> ignore;
    }

    linestream >> starttime;  // 22nd field is the process start time in clock ticks

    long clock_ticks_per_second = sysconf(_SC_CLK_TCK);
    long process_start_time_in_seconds = starttime / clock_ticks_per_second;

    // Get the system uptime
    long system_uptime = UpTime();

    // Time since the process started = system uptime - process start time
    long time_since_process_started = system_uptime - process_start_time_in_seconds;

    return time_since_process_started; 
}
