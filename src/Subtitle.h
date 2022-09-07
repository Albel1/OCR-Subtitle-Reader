#pragma once
#include <iostream>
#include <fstream>
#include <array>
#include <vector>

class Subtitle{
public:
    Subtitle(unsigned long int startTime,unsigned long int endTime,std::string value)
        :startTime(startTime),endTime(endTime),value(value){
    }
    long unsigned int getStartTime() const{
        return startTime;
    }
    long unsigned int getEndTime() const{
        return endTime;
    }
    std::string getValue() const{
        return value;
    }

    static std::vector<Subtitle> parseSubtitlesFromSRT(const std::string& fileName){
        std::ifstream file(fileName, std::ios_base::in);
        int count = 0;
        std::vector<Subtitle> subtitles;
        std::string value;
        std::array<unsigned long int,2> times;
        for (std::string line; std::getline(file, line); ) {
            count++;
            if (count == 2){
                times = parseTime(line);
            }
            else if (count == 3){
                value = line;
            }
            else if (count == 4){
                subtitles.emplace_back(times[0],times[1],value);
                count = 0;
            }
        }
        return subtitles;
    }
    std::string toString(){
        return toTimeStampString(startTime) + " --> " + toTimeStampString(endTime) + "\n" + value + "\n"; 
    }
    std::string toTimeStampString(long unsigned int t) const{
        long double time = (long double) t;
        int hours = (int) 
            floor(time/TimeUnits::MILLISECONDS_IN_HOUR );
        time -= hours*TimeUnits::MILLISECONDS_IN_HOUR;
        int minutes = (int) floor(time/TimeUnits::MILLISECONDS_IN_MINUTE);
        time -= minutes*TimeUnits::MILLISECONDS_IN_MINUTE;
        int seconds = (int) floor(time/TimeUnits::MILLISECONDS_IN_SECOND);
        time -= seconds*TimeUnits::MILLISECONDS_IN_SECOND;
        int milliseconds = time;
        std::string timeStamp = "";
        timeStamp += ( (hours < 10 ? "0":"") + std::to_string(hours));
        timeStamp += ":";
        timeStamp += ( (minutes < 10 ? "0":"") + std::to_string(minutes));
        timeStamp += ":";
        timeStamp += ( (seconds < 10 ? "0":"") + std::to_string(seconds));
        timeStamp += ",";
        if (milliseconds < 10)
            timeStamp += ( "00" + std::to_string(milliseconds));
        else if (milliseconds < 100)
            timeStamp += ( "0" + std::to_string(milliseconds));
        else
            timeStamp += std::to_string(milliseconds);
        return timeStamp;
    }
private:
    static std::array<unsigned long int,2> parseTime(const std::string& string){
        //00:00:02,112 --> 00:00:03,359
        const int TIME_STAMP_LENGTH = 12;
        std::string lowerTimeStamp = string.substr(0, TIME_STAMP_LENGTH);
        std::string upperTimeStamp = string.substr(string.size()-TIME_STAMP_LENGTH, string.size());
        assert(upperTimeStamp.length() == TIME_STAMP_LENGTH);
        return {parseTimeStamp(lowerTimeStamp), parseTimeStamp(upperTimeStamp)};
    }
    static unsigned long int parseTimeStamp(const std::string& string){
        unsigned long int hours = std::stoi(string.substr(0,2));
        unsigned long int minutes = std::stoi(string.substr(3,5));
        unsigned long int seconds = std::stoi(string.substr(6,8));
        unsigned long int milliseconds = std::stoi(string.substr(9,12));
        return ( 
              hours*TimeUnits::MILLISECONDS_IN_HOUR
            + minutes*TimeUnits::MILLISECONDS_IN_MINUTE
            + seconds*TimeUnits::MILLISECONDS_IN_SECOND
            + milliseconds );
    }
    long int startTime;
    long int endTime;
    std::string value;
};