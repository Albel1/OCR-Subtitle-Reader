#pragma once
#include <vector>
#include <opencv2/opencv.hpp> 
#include "Constants.h"


class SelectionCanvas{
public:
    SelectionCanvas(int rows, int cols);
    bool isSelectedAt(const cv::Point2i& p) const;
    void selectPixel(const cv::Point2i& p);
    void deselectPixel(const cv::Point2i& p);
    void deselectAll();
    cv::Mat toImage() const;
    int getRows() const;
    int getCols() const;
private:
    std::vector<bool> selected;
    const int rows;
    const int cols;
};