#pragma once
#include <string.h>
#include <opencv2/opencv.hpp>
#include "Text.h"

class IOCR{
public:
    virtual const std::vector<std::string>& getTextStrings() = 0;
    virtual void addTextImage(const Text& text) = 0;
};