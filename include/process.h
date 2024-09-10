#ifndef PROCESS_H
#define PROCESS_H

#include <string>
/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
 public:
  Process(int pid);
  bool isValid() const;
  bool hasChanged();
  float getCpuUtilization(unsigned long long totalSystemTime, unsigned long long lastTotalSystemTime );
  int Pid();                               // TODO: See src/process.cpp
  std::string User();                      // TODO: See src/process.cpp
  std::string Command();                   // TODO: See src/process.cpp
  float CpuUtilization();                  // TODO: See src/process.cpp
  std::string Ram();                       // TODO: See src/process.cpp
  long int UpTime();                       // TODO: See src/process.cpp
  bool operator>(Process const& a) const;  // TODO: See src/process.cpp

  // TODO: Declare any necessary private members
 private:
  int pid_ = {};
  bool valid_;
  float cpuUtilization_;
  unsigned long long totalTime_;
  unsigned long long lastTotalTime_;
  //unsigned long long lastTotalSystemTime_;
  unsigned long long startTime_;

  void updateInfo();
  
};

#endif