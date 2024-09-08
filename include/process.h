#ifndef PROCESS_H
#define PROCESS_H

#include <string>
/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
 public:
  Process(int pid) : pid_(pid), valid_(true), lastTotalTime_(0), lastTotalSystemTime_(0), cpuUtilization_(0.0) {
        updateInfo();
  }
  int Pid();                               // TODO: See src/process.cpp
  std::string User();                      // TODO: See src/process.cpp
  std::string Command();                   // TODO: See src/process.cpp
  float CpuUtilization();                  // TODO: See src/process.cpp
  std::string Ram();                       // TODO: See src/process.cpp
  long int UpTime();                       // TODO: See src/process.cpp
  bool operator<(Process const& a) const;  // TODO: See src/process.cpp

  // TODO: Declare any necessary private members
 private:
  int pid_ = {};
  bool valid_;
  float cpuUtilization_;
  unsigned long long totalTime_;
  unsigned long long lastTotalTime_;
  unsigned long long lastTotalSystemTime_;
  unsigned long long startTime_;

  void updateInfo();
  bool hasChanged();
};

#endif