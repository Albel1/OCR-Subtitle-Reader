#include "TextExtractor.h"
#include "SubtitleExtractor.h"
#include "Subtitle.h"
#include "GroundTruthCreator.h"

void checkFileExistence(std::vector<std::string>& arguments, int min, int max){
    std::vector<std::string> nonExistentFiles;
    for (int i=min;i<max;i++){
        if (!std::filesystem::exists(arguments[i])){
            nonExistentFiles.emplace_back(arguments[i]);
        }
    }
    if (!nonExistentFiles.empty()){
        std::string message = "File(s): ";
        for (std::string fileName : nonExistentFiles){
            message += fileName + " ";
        }
        message += "do not exist.";
        throw std::runtime_error(message);
    }
}
void handleExtract(std::vector<std::string>& arguments){
    arguments = {"", "/Users/laptop/Desktop/Projects/C++/SubReader/resources/vid2.mp4"};
    checkFileExistence(arguments,1,arguments.size());
    Properties::BaseTextExtractorProperties t;
    SubtitleExtractor extractor;
    for (int i=1;i<arguments.size();i++ ){
        extractor.readVideo(arguments[i]);
    }
}
void handleGenerate(std::vector<std::string>& arguments){

}

    static size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp){
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

#include <algorithm> 

int main(int argc, char * argv[]) {

    Properties::BaseTextExtractorProperties t;
    SubtitleExtractor extractor;
    extractor.readVideo("/Users/laptop/Desktop/Projects/C++/SubReader/resources/vid1.mp4");

    // std::vector<std::string> arguments(argv + 1, argv + argc);
    //checkFileExistence(arguments,1,arguments.size());
    // if (arguments.empty()) 
    //     throw std::runtime_error("No arguments given.");
    // std::string& command = arguments.front();
    // std::transform(command.begin(), command.end(), command.begin(),
    //     [](unsigned char c){ return std::tolower(c); });
    // if (command == "extract"){
    //     handleExtract(arguments);
    // }
    // else if (command == "generate"){
        
    // }
    // else{
    //     throw std::runtime_error("Unknown command \"" + command + "\" passed to SubReader.");
    // }
}

