#pragma once
#include <string>
#include <fstream>
#include "IOCR.h"
#include "Text.h"
#include "TimeUnits.h"
#include "OCRSpaceReader.h"

class SubtitleCreator{
public:
    SubtitleCreator(long double fps, IOCR* ocr):fps(fps),ocr(ocr){};

    void addTimeStamp(unsigned int startFrameNumber,unsigned int endFrameNumber,const Text& text){
        timestamps.push_back(startFrameNumber);
        timestamps.push_back(endFrameNumber);
        ocr->addTextImage(std::move(text));
        printContents(startFrameNumber,endFrameNumber);
    }

    void printContents(int startFrameNumber, int endFrameNumber){
        std::cout << frameNumberToTimeStamp(startFrameNumber) << " --> " << frameNumberToTimeStamp(endFrameNumber) << std::endl;
    }
    void writeContentsToFile(const std::string& name) const{
        std::ofstream file(name + ".srt");
        file << getSRTContents();
        file.close();
    }
    std::string getSRTContents() const{
        std::string contents = "";
        size_t j = 0;
        std::vector<std::string> subtitleValues = ocr->getTextStrings();
        for (size_t i=0;i<timestamps.size();i+=2){
            contents += std::to_string(j+1);
            contents += "\n";
            contents += frameNumberToTimeStamp(timestamps[i]);
            contents += " --> ";
            contents += frameNumberToTimeStamp(timestamps[i+1]);
            contents += "\n";
            contents += subtitleValues[j];
            contents += "\n\n";
            j++;   
        }
        return contents;
    }
    std::string frameNumberToTimeStamp(unsigned int frameNumber) const{
        long double time = frameNumber/fps;
        int hours = (int) floor(time/TimeUnits::SECONDS_IN_HOUR);
        time -= hours*TimeUnits::SECONDS_IN_HOUR;
        int minutes = (int) floor(time/TimeUnits::SECONDS_IN_MINUTE);
        time -= minutes*TimeUnits::SECONDS_IN_MINUTE;
        int seconds = (int) floor(time);
        time -= seconds;
        int milliseconds = ((int) TimeUnits::MILLISECONDS_IN_SECOND*time);
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
    std::vector<std::string> processText() const{
        std::vector<std::string> subtitleValues;
        std::vector<std::string> strings = ocr->getTextStrings();
        for (int i =0; i<strings.size();i++){
            
        }
        // for (const Text& text : textValues){
        //     subtitleValues.emplace_back(ocr->readText(text));
        // }
        return subtitleValues;
    }
    const long double fps;
    std::vector<unsigned int> timestamps;
    std::vector<Text> textValues;  
    IOCR* ocr;
};