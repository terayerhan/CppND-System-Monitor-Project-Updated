#include "processor.h"
#include "linux_parser.h"

// TODO: Return the aggregate CPU utilization
float Processor::Utilization() {
    long currentTotalJiffies = LinuxParser::Jiffies();
    long currentActiveJiffies = LinuxParser::ActiveJiffies();

    long totalDiff = currentTotalJiffies - previousTotalJiffies_;
    long activeDiff = currentActiveJiffies - previousActiveJiffies_;

    if(totalDiff == 0){ return 0.0;}

    // Cache the current values in the Processor object.
    previousTotalJiffies_ = currentTotalJiffies;
    previousActiveJiffies_ = currentActiveJiffies;

    return static_cast<float>(activeDiff) / totalDiff; 
}