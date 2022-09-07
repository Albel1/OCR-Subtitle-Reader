#include <array>
#include <vector>
#include <filesystem>
#include "Subtitle.h"
#include <regex>
    #include <codecvt>

namespace fs = std::filesystem;

class GroundTruthCreator{
public:
    GroundTruthCreator(Properties::BaseTextExtractorProperties baseProperties, 
            fs::path outputDirectory,int frameDivisions=3)
        :baseProperties(baseProperties),outputDirectory(outputDirectory),
            frameDivisions(frameDivisions){}

    void createGroundTruth(fs::path inputDirectory, bool appendToDirectory=true){
        std::vector<fs::path> subtitleFiles;
        std::vector<fs::path> videoFiles;
        int maxFileNumber = 0;
        for (const auto& file : fs::directory_iterator(inputDirectory)){
            if (file.path().extension() == ".srt" && std::regex_match(file.path().stem().string(),digits) ){
                subtitleFiles.push_back(file);
                int fileNumber = std::stoi(file.path().stem());
                if (fileNumber > maxFileNumber) maxFileNumber = fileNumber;
            }
        }
        std::vector<bool> found(maxFileNumber);
        for (const auto& file : fs::directory_iterator(inputDirectory)){
            int fileNumber;
            if (std::regex_match(file.path().stem().string(),digits)
                && file.path().extension() != ".srt" 
                && (fileNumber = std::stoi(file.path().stem())) >= 1){
                if (fileNumber <= maxFileNumber) found[fileNumber-1]=true;
                videoFiles.push_back( file.path() );
            }
        }
        std::sort(subtitleFiles.begin(),subtitleFiles.end(),
            [](const fs::path& p1, const fs::path& p2){
                return std::stoi(p1.stem())-std::stoi(p2.stem());
            }
        );
        std::sort(videoFiles.begin(),videoFiles.end(),
            [](const fs::path& p1, const fs::path& p2){
                return std::stoi(p1.stem())-std::stoi(p2.stem());
            }
        );
        std::string errorString = "";
        for (int i =0; i<maxFileNumber;i++){
            if (!found[i]) errorString 
                += "Video corresponding to " 
                    + subtitleFiles[i].filename().string() + " not found\n";
        }
        if (errorString != "") throw std::logic_error(errorString);
        std::vector<std::pair<fs::path,fs::path>> pathPairs;
        for (int i =0;i<subtitleFiles.size();i++) 
            pathPairs.emplace_back(videoFiles[i],subtitleFiles[i]);
        if (appendToDirectory){
            count = getMaxFileNumberOfOutputDirectory();
        }
        createGroundTruthIter(pathPairs);
    }
    int getMaxFileNumberOfOutputDirectory(){
        int maxNumber = 0;
        for (const auto& file : fs::directory_iterator(outputDirectory)){
            if (std::regex_match(file.path().stem().string(),digits)){
                maxNumber = std::max(maxNumber,std::stoi(file.path().stem().string()));
            }
        }
        return maxNumber;
    }

    void createGroundTruth(  std::vector<std::pair<fs::path,fs::path>>& pathPairs){
        std::vector<std::pair<fs::path,fs::path>> inputPairs;
        for (auto& [path1,path2] : pathPairs){
            if (path1.extension() == ".srt" && path2.extension() == ".srt"){
                throw std::invalid_argument("Files: " + path1.filename().generic_string() 
                + " and " + path2.filename().generic_string() + " both have extensions \".srt\"; should"
                + " have one video file and one .srt file"  );
            }
            else if (path1.extension() != ".srt" && path2.extension() != ".srt"){
                throw std::invalid_argument("Neither files: " + path1.filename().generic_string()
                + " and " + path2.filename().generic_string() + " have extension \".srt\"; should"
                + " have one video file and one .srt file"  );
            }
            fs::path& srtFile = path1.extension() == ".srt" ? path1 : path2;
            fs::path& videoFile =  path1.extension() == ".srt" ? path2 : path1;
            inputPairs.emplace_back(videoFile,srtFile);
        }
        createGroundTruthIter(inputPairs);
    }
private:
    void createGroundTruthIter(  std::vector<std::pair<fs::path,fs::path>>& inputPairs){
        for (auto& [videoFile,srtFile] : inputPairs){
            createGroundTruthSingle(videoFile,srtFile);
        }
    }
    // void createGroundTruthSingle(fs::path videoFile, fs::path srtFile){
    //     cv::VideoCapture cap(videoFile);
    //     if (!cap.isOpened()){
    //         std::cout << "Cannot open the video file" << std::endl;
    //         return;
    //     }
    //     std::vector<Subtitle> subs = Subtitle::parseSubtitlesFromSRT(srtFile);
    //     int sub_index = 0;
    //     double fps = cap.get(cv::CAP_PROP_FPS); //get the frames per seconds of the video
    //     cv::Mat frame;
    //     cap.read(frame);
    //     double minSubtitleDuration = 0.25; //seconds
    //     int currentFrame = 1 ;
    //     Properties::BaseTextExtractorProperties t;
    //     TextExtractor textExtractor(t,frame.rows,frame.cols);
    //     int totalFrames = (int) cap.get(cv::CAP_PROP_FRAME_COUNT); 
    //     const double noTextTimeJump = 0.5;
    //     while (currentFrame < totalFrames-1){
    //         int startFrame = currentFrame; 
    //         cap.set(cv::CAP_PROP_POS_FRAMES,currentFrame);  
    //         cap.read(frame);  
    //         std::optional<Text> text = textExtractor.extractText(frame);   
    //         if (text.has_value())
    //             cv::imwrite("/Users/laptop/Desktop/Projects/C++/SubReader/out/frames/result.png", text->getImage());
    //         int lo = currentFrame;
    //         int hi;
    //         if (!text.has_value()){
    //             int curr = startFrame;
    //             int frameJump  = (int) (noTextTimeJump*fps);
    //             std::optional<Text> t = {};
    //             while (!t.has_value()){
    //                 curr += frameJump;
    //                 cap.set(cv::CAP_PROP_POS_FRAMES,curr);
    //                 if (!cap.read(frame)){
    //                     std::cout << "Didn't work :/" << std::endl;
    //                 } 
    //                 cv::imwrite("/Users/laptop/Desktop/Projects/C++/SubReader/out/frames/result.png", frame);
    //                 t = textExtractor.extractText(frame);
    //                 if (t.has_value() )
    //                     cv::imwrite("/Users/laptop/Desktop/Projects/C++/SubReader/out/frames/result.png", t->getImage());
    //                 if (!t.has_value()) lo = curr;
    //                 if (curr+frameJump >= totalFrames-1){
    //                     curr = totalFrames-1;
    //                     break;
    //                 }
    //             }
    //             hi = curr;
    //         }
    //         else{
    //             const int maxDistance = 7;
    //             hi = std::min(currentFrame+(int)(maxDistance*fps),totalFrames-1);
    //         }
    //         while (lo < hi) {
    //             int mid = (lo + hi) / 2;
    //             cap.set(cv::CAP_PROP_POS_FRAMES,mid);
    //             if (!cap.read(frame)){
    //                 std::cout << "Didn't work :/" << std::endl;
    //                 break;
    //             }
    //             cv::imwrite("/Users/laptop/Desktop/Projects/C++/SubReader/out/frames/result.png", frame);
    //             auto text1 = textExtractor.extractText(frame);
    //             if (text1.has_value())
    //                 cv::imwrite("/Users/laptop/Desktop/Projects/C++/SubReader/out/frames/result.png", text1->getImage());
    //             if (text.has_value() != text1.has_value() ){
    //                 hi = mid-1;
    //             }
    //             else if (!text.has_value() && !text1.has_value() ){
    //                 currentFrame = mid;
    //                 lo = mid+1;
    //             }
    //             else if (text->isSameText(*text1)){
    //                 currentFrame = mid;
    //                 lo = mid+1;
    //             }
    //             else{
    //                 hi = mid-1;
    //             }
    //         }
    //         if (text.has_value() && (currentFrame-startFrame)/fps >= minSubtitleDuration){
    //             long unsigned int lowerTime = (startFrame/fps)*1000;
    //             long unsigned int upperTime = ((currentFrame)/fps)*1000;
    //             cv::imwrite("/Users/laptop/Desktop/Projects/C++/SubReader/out/frames/result.png", text->getImage());
    //             while (true){
    //                 Subtitle& s = subs[sub_index];
    //                 std::cout << s.getValue() << std::endl;
    //                 long unsigned int avg = (s.getStartTime()+s.getEndTime())/2 ;
    //                 const int BORDER_PADDING_SIZE = 4; 
    //                 if ( lowerTime <= avg && avg <= upperTime ){
    //                     cv::Mat im = text->normalizeImage();
    //                     writeToFile(im,s.getValue());
    //                     break;
    //                 }
    //                 else if (lowerTime > avg){
    //                     s = subs[++sub_index];
    //                     avg = (s.getStartTime()+s.getEndTime())/2 ;
    //                     if (upperTime < avg){
    //                         break;
    //                     }
    //                 }
    //                 else if (upperTime < avg){
    //                     sub_index--;
    //                     s = subs[--sub_index];
    //                     avg = (s.getStartTime()+s.getEndTime())/2 ;
    //                     if (upperTime < avg){
    //                         break;
    //                     }
    //                 }
    //                 else{
    //                     std::cout << "broken" << std::endl;
    //                     break;                       
    //                 }
    //             }
    //         }
    //         currentFrame=lo;
    //         currentFrame+=1;
    //     }
    // }
    void createGroundTruthSingle(fs::path videoFile, fs::path srtFile){
        cv::Mat frame;
        int c = 0;
        cv::VideoCapture cap(videoFile.generic_string());
        cap.read(frame);
        TextExtractor textExtractor(baseProperties,frame.rows,frame.cols);
        double fps = cap.get(cv::CAP_PROP_FPS); 
        std::vector<Subtitle> subtitles = Subtitle::parseSubtitlesFromSRT(srtFile);
        std::optional<Text> previousText;
        int k = c;
        unsigned int totalTime =  1000.0*((long double) cap.get(cv::CAP_PROP_FRAME_COUNT)) / cap.get(cv::CAP_PROP_FPS);
        for (int i=k;i<subtitles.size();i++){
            Subtitle subtitle = subtitles[i];
            if (subtitle.getStartTime() > totalTime || subtitle.getEndTime() > totalTime) break;
            std::cout << ++c << std::endl;
            std::optional<Text> text = {};
            unsigned int timeJump = (unsigned int) ceil((subtitle.getEndTime()-subtitle.getStartTime())/(frameDivisions + 2.0));
            for (int time=subtitle.getStartTime()+timeJump;time<subtitle.getEndTime()-timeJump;time+=timeJump){
                cap.set(cv::CAP_PROP_POS_MSEC, time);
                if (!cap.read(frame)){
                    throw std::logic_error("Could not read frame for video " + videoFile.filename().generic_string() );
                }
                cv::imwrite("/Users/laptop/Desktop/Projects/C++/SubReader/out/frames/result.png", frame);
                auto otherText = textExtractor.extractText(frame);
                if (!text.has_value() 
                    && (!previousText.has_value() || !previousText->isSameText(*otherText) ) ) 
                    text = std::move(otherText);
                else if (text.has_value() && !text->isSameText(*otherText) ){
                    std::cout << "Did not get consistent Text within subtitle time frame" 
                    " for video "+ videoFile.filename().generic_string() << std::endl;
                    // throw std::logic_error("Did not get consistent Text within subtitle time frame" 
                    // " for video "+ videoFile.filename().generic_string() );
                }
            }
            if (!text.has_value()){
                std::cout << "Could not extract text " << subtitle.getValue() << std::endl;
            }
            else{
                // int expectedWidth = expectedCharacterWidth*utf8Length(subtitle.getValue()); 
                // int actualWidth = text->getMaxCol()-text->getMinCol();
                // if ( (actualWidth - expectedWidth) > 10) std::cout << "Check " << count+1 << std::endl;
                // else if ( (expectedWidth - actualWidth) > 10 ) std::cout << "Check " << count+1 << std::endl;
                cv::Mat im = text->normalizeImage();
                writeToFile(im,subtitle.getValue());
                previousText = std::move(text);
            }
        }
    }
    // int getAverageCharacterWidth(cv::VideoCapture& cap, std::vector<Subtitle>& subtitles, TextExtractor& textExtractor ){
    //     cv::Mat frame;
    //     double j = 0.0;
    //     double characterWidth = 0.0;
    //     std::optional<Text> previousText = {};
    //     for (int i =0;i < std::min(10, (int)subtitles.size()) ;i++){
    //         Subtitle subtitle = subtitles[i];
    //         std::optional<Text> text = {};
    //         unsigned int timeJump = (unsigned int) ceil((subtitle.getEndTime()-subtitle.getStartTime())/(frameDivisions + 2.0));
    //         for (int time=subtitle.getStartTime()+timeJump;time<subtitle.getEndTime()-timeJump;time+=timeJump){
    //             cap.set(cv::CAP_PROP_POS_MSEC, time);
    //             if (!cap.read(frame)){
    //                 throw std::logic_error("Could not read frame for video");
    //             }
    //             auto otherText = textExtractor.extractText(frame);
    //             if (!text.has_value() 
    //                 && (!previousText.has_value() || !previousText->isSameText(*otherText) ) ) 
    //                 text = std::move(otherText);
    //         }
    //         if (text.has_value()){
    //             j += 1.0;
    //             std::cout <<  utf8Length(subtitle.getValue()) << std::endl;
    //             characterWidth += (text->getMaxCol()-text->getMinCol())/ ( (double) utf8Length(subtitle.getValue()) ) ;
    //             previousText = std::move(text);
    //         }

    //     }
    //     return characterWidth/j;
    // }

    int utf8Length(const std::string& utf8){
        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
        return conv.from_bytes(utf8).size();
    }
    void writeToFile(const cv::Mat& image, const std::string& string) {
        std::string filePath = outputDirectory.generic_string() + "/" + std::to_string(++count); 
        std::ofstream file(filePath + ".gt.txt");
        file << string;
        file.close();
        cv::imwrite(filePath + ".png",image);
        std::cout << filePath << std::endl;
    }

    std::regex digits = std::regex("\\d+");
    fs::path outputDirectory;
    int frameDivisions;
    Properties::BaseTextExtractorProperties baseProperties;
    unsigned int count = 0;
    std::cmatch cm;
};