#pragma once
#include "IOCR.h"
#include <tesseract/baseapi.h>
#include "Text.h"
#include <opencv2/dnn_superres.hpp>
#include <opencv2/opencv.hpp>
#include <memory>
#include <vector>
#include <algorithm>
#include <string>
#include <optional>
#include <nlohmann/json.hpp>
#include <curl/curl.h>
#include <regex>
#include <array>
#include "base64.h"
#include "FileType.h"

using json = nlohmann::json;

class OCRSpaceReader : public IOCR{
public:
    class Settings{
    public:
        enum EngineType {OCR_ENGINE_1,OCR_ENGINE_2,OCR_ENGINE_3};
        enum ValueType {FILE, BASE_64_IMAGE};
        Settings(std::string apikey, ValueType valueType, size_t maxSizeBytes = 1000000)
        :apikey(apikey),valueType(valueType),maxSizeBytes(maxSizeBytes){
        }
        Settings& setLanguage(std::string language){
            this->language = language;
            return *this;
        }
        Settings& setIsOverlayRequire(bool isOverlayRequire){
            this->isOverlayRequire = isOverlayRequire;
            return *this;
        }
        Settings& setFiletype(FileType filetype){
            this->filetype = filetype;
            return *this;
        }
        Settings& setDetectOrientation(bool detectOrientation){
            this->detectOrientation = detectOrientation;
            return *this;
        }
        Settings& setIsCreateSearchablePdf(bool isCreateSearchablePdf){
            this->isCreateSearchablePdf = isCreateSearchablePdf;
            return *this;
        }
        Settings& setIsSearchablePdfHideTextLayer(bool isSearchablePdfHideTextLayer){
            this->isSearchablePdfHideTextLayer = isSearchablePdfHideTextLayer;
            return *this;
        }
        Settings& setScale(bool scale){
            this->scale = scale;
            return *this;
        }
        Settings& setIsTable(bool isTable){
            this->isTable = isTable;
            return *this;
        }
        Settings& setEngineType(EngineType engineType){
            this->engineType = engineType;
            return *this; 
        }
        const Settings& build(){
            return *this;
        }
        size_t getMaxSizeBytes() const{
            return this->maxSizeBytes;
        }
        void buildParameters(CURL*& curl, struct curl_slist*& headers, curl_mime*& mime, curl_mimepart*& part, std::string base64Image) const{
            std::string keyStr = "apikey: " + apikey;
            headers = curl_slist_append(headers, keyStr.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            mime = curl_mime_init(curl);
            part = curl_mime_addpart(mime);
            curl_mime_name(part, "filetype");
            curl_mime_data(part, FILE_TYPE_TO_STRING[filetype].c_str(), CURL_ZERO_TERMINATED);
            part = curl_mime_addpart(mime);
            curl_mime_name(part, "language");
            curl_mime_data(part, language.c_str(), CURL_ZERO_TERMINATED);
            part = curl_mime_addpart(mime);
            curl_mime_name(part, "base64Image");
            std::string toSend = "data:image/"+ FILE_TYPE_TO_STRING[filetype] + ";base64," + base64Image;
            curl_mime_data(part, toSend.c_str(), CURL_ZERO_TERMINATED);
            part = curl_mime_addpart(mime);
            curl_mime_name(part, "scale");
            curl_mime_data(part, scale ? "true" : "false", CURL_ZERO_TERMINATED);
            part = curl_mime_addpart(mime);
            curl_mime_name(part, "isTable");
            curl_mime_data(part, isTable ? "true" : "false", CURL_ZERO_TERMINATED);
            part = curl_mime_addpart(mime);
            curl_mime_name(part, "detectOrientation");
            curl_mime_data(part, scale ? "true" : "false", CURL_ZERO_TERMINATED);
            part = curl_mime_addpart(mime);
            curl_mime_name(part, "isCreateSearchablePdf");
            curl_mime_data(part, isCreateSearchablePdf ? "true" : "false", CURL_ZERO_TERMINATED);
            part = curl_mime_addpart(mime);
            curl_mime_name(part, "isSearchablePdfHideTextLayer");
            curl_mime_data(part, isSearchablePdfHideTextLayer ? "true" : "false", CURL_ZERO_TERMINATED);
            part = curl_mime_addpart(mime);
            curl_mime_name(part, "OCREngine");
            switch (engineType){
                case OCR_ENGINE_1:
                    curl_mime_data(part, "1", CURL_ZERO_TERMINATED);
                    break;
                case OCR_ENGINE_2:
                    curl_mime_data(part, "2",CURL_ZERO_TERMINATED);
                    break;
                case OCR_ENGINE_3:
                    curl_mime_data(part, "3", CURL_ZERO_TERMINATED);
                    break;
                default:
                    break;
            }
            part = curl_mime_addpart(mime);
            curl_mime_name(part, "isOverlayRequired");
            curl_mime_data(part, isOverlayRequire ? "true" : "false", CURL_ZERO_TERMINATED);
        }

    private:
        const std::string apikey;
        const ValueType valueType;
        const size_t maxSizeBytes;
        std::string language = "chs";
        bool isOverlayRequire = false;
        FileType filetype = JPG;
        bool detectOrientation = false;
        bool isCreateSearchablePdf = false;
        bool isSearchablePdfHideTextLayer = false;
        bool scale = true;
        bool isTable = true;
        EngineType engineType = OCR_ENGINE_1;

    };

    OCRSpaceReader(const Settings settings):
    settings(settings){
    }

    virtual void addTextImage(const Text& text){
        cv::imwrite("/Users/laptop/Desktop/Projects/C++/SubReader/out/frames/result.png", text.getImage());
        cv::Mat concatenatedImage = concatenateToImage(text.normalizeImage());
        cv::imwrite("/Users/laptop/Desktop/Projects/C++/SubReader/out/frames/result.jpg", concatenatedImage);
        if ( imageSize(concatenatedImage) >= settings.getMaxSizeBytes()){
            sendAPIRequest();
            image = std::make_unique<cv::Mat>(text.normalizeImage());
        }
        else{
            image = std::make_unique<cv::Mat>(std::move(concatenatedImage));
        }
    }
    virtual const std::vector<std::string>& getTextStrings(){
        sendAPIRequest();
        return strings;
    }
public:
    void sendAPIRequest(){
        CURL *curl;
        CURLcode res;
        std::string readBuffer;
        curl = curl_easy_init();
        if(curl) {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
            curl_easy_setopt(curl, CURLOPT_URL, "https://api.ocr.space/parse/image");
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
            struct curl_slist *headers = NULL; 
            curl_mime *mime;
            curl_mimepart *part;
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            settings.buildParameters(curl, headers, mime, part,imageToBase64String());
            curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            res = curl_easy_perform(curl);
            curl_mime_free(mime);
            std::string s = json::parse(readBuffer)["ParsedResults"][0]["ParsedText"];
            std::string tmp; 
            std::stringstream ss(s);
            while(getline(ss, tmp, '\n')){
                strings.push_back(std::regex_replace(tmp, std::regex("\\s+$"), ""));
            }
        }
        curl_easy_cleanup(curl);
    }
    static size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp){
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }
    std::string imageToBase64String(){
        std::vector<uchar> buf;
        cv::imencode(".jpg", *image, buf);
        unsigned char const* enc_msg = reinterpret_cast<unsigned char*>(buf.data());
        std::string encoded = base64_encode(enc_msg, buf.size());
        return encoded;
    }
    cv::Mat concatenateToImage(const cv::Mat& toConcat){
        int a = toConcat.rows+image->rows;
        int b = std::max<size_t>(toConcat.cols,image->cols);
        cv::Mat newImage(a,b,CV_8UC3, cv::Scalar(0, 0, 0));
        for (int row=0;row<image->rows;row++){
            for (int col=0;col<image->cols;col++){
                auto point = cv::Point(col,row);
                newImage.at<cv::Vec3b>(point) = image->at<cv::Vec3b>(point);
            }
        }
        for (int row=0;row<toConcat.rows;row++){
            for (int col=0;col<toConcat.cols;col++){
                newImage.at<cv::Vec3b>(cv::Point(col,row+image->rows)) = toConcat.at<cv::Vec3b>(cv::Point(col,row));
            }
        }
        return newImage;
    }
    size_t imageSize(cv::Mat& img){
        if (img.rows ==0 || img.cols == 0) return 0;
        std::vector<uchar> buf;
        cv::imencode(".jpg", img, buf, std::vector<int>() );
        return buf.size();
    }
    std::vector<std::string> strings;
    std::unique_ptr<cv::Mat> image = std::make_unique<cv::Mat>(0,0,CV_8UC1, cv::Scalar(0, 0, 0)); 
    const std::string fileExtension = ".jpg";
    const Settings settings;
};