#pragma once
#include "SubtitleCreator.h"
#include "TesseractReader.h"
#include "Subtitle.h"
#include "OCRSpaceReader.h"
// #include <tesseract/baseapi.h>



class SubtitleExtractor{
public:
    void readVideo(const std::string& fileName){
        cv::VideoCapture cap(fileName);
        if (!cap.isOpened()){
            std::cout << "Cannot open the video file" << std::endl;
            return;
        }
        int sub_index = 0;
        double fps = cap.get(cv::CAP_PROP_FPS); //get the frames per seconds of the video
        //TesseractReader reader("lei5");
        OCRSpaceReader reader({"K84097410888957", OCRSpaceReader::Settings::BASE_64_IMAGE});

        SubtitleCreator subtitles(fps,&reader);
        std::cout << "Frame per seconds : " << fps << std::endl;
        cv::Mat frame;
        cap.read(frame); // read a new frame from video
        double minSubtitleDuration = 0.25; //seconds
        int currentFrame = 1 ;
        Properties::BaseTextExtractorProperties t;
        TextExtractor textExtractor(t,frame.rows,frame.cols);
        int totalFrames = (int) cap.get(cv::CAP_PROP_FRAME_COUNT); 
        cap.set(cv::CAP_PROP_POS_FRAMES,currentFrame);
        cap.read(frame);  
        std::optional<Text> as = textExtractor.extractText(frame);   
        const double noTextTimeJump = 0.5;
        while (currentFrame < totalFrames-1){
            int startFrame = currentFrame; 
            cap.set(cv::CAP_PROP_POS_FRAMES,currentFrame);  
            cap.read(frame);  
            std::optional<Text> text = textExtractor.extractText(frame);   
            if (text.has_value())
                cv::imwrite("/Users/laptop/Desktop/Projects/C++/SubReader/out/frames/result.png", text->getImage());
            int lo = currentFrame;
            int hi;
            if (!text.has_value()){
                int curr = startFrame;
                int frameJump  = (int) (noTextTimeJump*fps);
                std::optional<Text> t = {};
                while (!t.has_value()){
                    std::cout << lo << std::endl;
                    curr += frameJump;
                    cap.set(cv::CAP_PROP_POS_FRAMES,curr);
                    if (!cap.read(frame)){
                        std::cout << "Didn't work :/" << std::endl;
                    } 
                    cv::imwrite("/Users/laptop/Desktop/Projects/C++/SubReader/out/frames/result.png", frame);
                    t = textExtractor.extractText(frame);
                    if (!t.has_value()) lo = curr;
                    if (curr+frameJump >= totalFrames-1){
                        curr = totalFrames-1;
                        break;
                    }
                }
                hi = curr;
            }
            else{
                const int maxDistance = 7;
                hi = std::min(currentFrame+(int)(maxDistance*fps),totalFrames-1);
            }
            while (lo < hi) {
                std::cout << lo << std::endl;
                int mid = (lo + hi) / 2;
                cap.set(cv::CAP_PROP_POS_FRAMES,mid);
                if (!cap.read(frame)){
                    std::cout << "Didn't work :/" << std::endl;
                    break;
                }
                cv::imwrite("/Users/laptop/Desktop/Projects/C++/SubReader/out/frames/result.png", frame);
                auto text1 = textExtractor.extractText(frame);
                if (text.has_value() != text1.has_value() ){
                    hi = mid-1;
                }
                else if (!text.has_value() && !text1.has_value() ){
                    currentFrame = mid;
                    lo = mid+1;
                }
                else if (text->isSameText(*text1)){
                    currentFrame = mid;
                    lo = mid+1;
                }
                else{
                    hi = mid-1;
                }
            }
            if (text.has_value() && (currentFrame-startFrame)/fps >= minSubtitleDuration){
                subtitles.addTimeStamp(startFrame,currentFrame,*text);
            }
            currentFrame=lo;
            currentFrame+=1;
        }
        subtitles.writeContentsToFile("subsa");
    }
};