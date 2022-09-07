#pragma once
#include <opencv2/opencv.hpp> 
#include <vector>
#include "Constants.h"
#include "Properties.h"
#include "SelectionCanvas.h"

class Selection{
public:
    Selection(SelectionCanvas& selected, std::vector<cv::Point2i>& allPoints, double edgePixelOutlineColor);
    void deselect();
    void merge(Selection& toMerge);
    static bool validLineDistance(const Selection& s1, const Selection& s2,
                            int verticalTolerance, int horizontalTolerance);
    int getWidth() const;
    int getHeight() const;
    int getMaxRow() const;
    int getMinRow() const;
    int getMaxCol() const;
    int getMinCol() const;
    double getEdgePixelOutlineColorPercentage() const;
    double getTextPixelColorPercentage() const;
    void isDisabled() const;
    void isCleared() const;
    void enable();
    void disable();
    size_t numberOfPixels() const;
    const std::vector<cv::Point2i>& getAllPoints() const;
private:
  std::vector<cv::Point2i> allPoints;
    inline void clear(){
        allPoints.clear();
        maxRow=0;
        maxCol=0;
        minRow=0;
        minCol=0;        
    }
    const Properties::TextExtractorProperties* properties;
    SelectionCanvas* selected;
    bool disabled = false;
    bool cleared = false;
    int maxRow;
    int maxCol;
    int minRow;
    int minCol;
    double edgePixelOutlineColor;
};

namespace std {
  template <> struct hash<Selection*>{
    size_t operator()(const Selection* x) const{
        return (size_t)&(*x);
    }
  };
}