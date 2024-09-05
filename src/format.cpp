#include <string>

#include "format.h"

using std::string;

// TODO: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
// REMOVE: [[maybe_unused]] once you define the function
string Format::ElapsedTime(long uptime_seconds) { 
    long int hours = uptime_seconds / 3600;
    uptime_seconds %= 3600;
    long int minutes = uptime_seconds / 60;
    long int seconds = uptime_seconds % 60;

    return std::to_string(hours) + ":" +
           std::to_string(minutes) + ":" +
           std::to_string(seconds); 
}