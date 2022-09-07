#pragma once
#include "IOCR.h"
#include <tesseract/baseapi.h>
#include "Text.h"
#include <opencv2/dnn_superres.hpp>

class TesseractReader : public IOCR{
public:
    TesseractReader(const char* language){
        ocr = new tesseract::TessBaseAPI();
        ocr->Init(NULL, language, tesseract::OEM_LSTM_ONLY);
        ocr->SetPageSegMode(tesseract::PSM_SINGLE_LINE);   
        ocr->SetVariable("tessedit_char_blacklist" , "-‧;:^&*@|[]\\\"\'" );
    }

    ~TesseractReader(){
        ocr->End();
        delete ocr;
    }

    virtual void addTextImage(const Text& text){
        textImages.emplace_back(text);
    }

    virtual const std::vector<std::string>& getTextStrings(){
        for (int i =textLines.size(); i<textImages.size();i++){
            textLines.emplace_back(readText(textImages[i]));
        }
        return textLines;
    }

    virtual std::string readText(const Text& text){
        cv::Mat paddedImage = text.normalizeImage();
        ocr->SetImage(paddedImage.data,paddedImage.cols,paddedImage.rows,3,paddedImage.step);
        char* utf8Text = ocr->GetUTF8Text();
        std::string result = std::string(utf8Text);
        result.erase(remove_if(result.begin(), result.end(), isspace), result.end());
        result += "\n";
        delete[] utf8Text;
        return result;
    }
private:
    const static int BORDER_PADDING_SIZE = 4;
    tesseract::TessBaseAPI* ocr;
    std::vector<Text> textImages;
    std::vector<std::string> textLines;
    std::set<std::string> CHARS = {"A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P","Q","R","S","T","U","V","W","X","Y","Z"};
};