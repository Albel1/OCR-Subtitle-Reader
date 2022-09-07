#pragma once
#include <opencv2/opencv.hpp> 
#include "Selection.h"
#include <algorithm>
#include <unordered_set>
#include <numeric>
#include "SelectionCanvas.h"
#include "Text.h"

class TextExtractor{
public:
    TextExtractor(const Properties::BaseTextExtractorProperties& baseProperties, const int rows, const int cols)
        :properties(Properties::toTextExtractorProperties(baseProperties,rows,cols)),
        selected(rows,cols){}

    std::optional<Text> extractText(cv::Mat& src){

        cv::cvtColor(src,src,cv::COLOR_BGR2HLS);
        if (properties.upperOutlineColor.has_value() && properties.lowerOutlineColor.has_value()){
            return extractTextWithOutline(src);
        }
        else{
            return extractTextWithoutOutline(src);
        }
        return {};
    }
private:
    cv::Mat result;
    const Properties::TextExtractorProperties properties;
    SelectionCanvas selected;
    void refineSelections(const cv::Mat& src, std::vector<Selection>& selections){
        std::sort(selections.begin(),selections.end(),
            [](const Selection& t1, const Selection& t2){
                return t1.getMinCol() < t2.getMinCol();
            }
        );
        std::sort(selections.begin(),selections.end(),
            [](const Selection& t1, const Selection& t2){
                return t1.getMinRow() < t2.getMinRow();
            }
        );
        const int TOL = 5;
        const Selection* s = &selections[0];
        std::array<int,2> longestMinRowGroup = {0,-1};
        int lastValue = -1;
        for (int j =0;j<selections.size()-1;j++){
            int minRowGroupSize = longestMinRowGroup[1] - longestMinRowGroup[0] + 1;
            if (minRowGroupSize >= selections.size() - j) break;
            s = &selections[j];
            if (s->getMinRow() > lastValue){
                lastValue = s->getMinRow();
                int i;
                for (i =j+1; i<selections.size();i++){
                    if ( selections[i].getMinRow() - s->getMinRow() > 5){
                        break;
                    }
                }
                if (minRowGroupSize < i - j){
                    longestMinRowGroup[0]=j;
                    longestMinRowGroup[1]=i-1;
                }
            }
        }
        int minRow = (selections[longestMinRowGroup[0]]).getMinRow()-TOL;
        //-----
        std::sort(selections.begin(),selections.end(),
            [](const Selection& t1, const Selection& t2){
                return t1.getMaxRow() < t2.getMaxRow();
            }
        );

        lastValue = -1;
        std::array<int,2> longestMaxRowGroup = {0,-1};
        for (int j =0;j<selections.size()-1;j++){
            int maxRowGroupSize = longestMaxRowGroup[1] - longestMaxRowGroup[0] + 1;
            if (maxRowGroupSize >= selections.size() - j) break;
            s = &selections[j];
            if (s->getMaxRow() > lastValue){
                lastValue = s->getMaxRow();
                int i;
                for (i =j+1; i<selections.size();i++){
                    if ( selections[i].getMaxRow() - s->getMaxRow() > 11){
                        break;
                    }
                }
                if (maxRowGroupSize < i - j){
                    longestMaxRowGroup[0]=j;
                    longestMaxRowGroup[1]=i-1;
                }
            }
        }
        int maxRow = (selections[longestMaxRowGroup[1]]).getMaxRow()+TOL;
        //------
        selections.erase(
            std::remove_if(selections.begin(),selections.end(),
                [this,minRow,maxRow](Selection& selection){
                    if (minRow <= selection.getMinRow() && selection.getMaxRow() <= maxRow) return false;
                    selection.deselect();
                    return true;
                }),
        selections.end());
    }

    std::optional<Text> extractTextWithOutline(const cv::Mat& src){
        auto totalStart = std::chrono::high_resolution_clock::now();
        auto start = std::chrono::high_resolution_clock::now();
        selected.deselectAll();
        drawSelectionZone(src);

        cv::Mat outlineColor,contours;
        cv::Mat textColor(src.rows,src.cols,CV_8UC3);

        cv::Canny(src,contours,properties.minCanny,properties.maxCanny);
        cv::Mat element = getStructuringElement( cv::MORPH_RECT,
                            cv::Size( 2*properties.dilationSize + 1, 2*properties.dilationSize+1 ),
                            cv::Point( properties.dilationSize, properties.dilationSize ) );

        cv::cvtColor(contours,contours,cv::COLOR_GRAY2BGR);
        cv::inRange(src,*properties.lowerOutlineColor,*properties.upperOutlineColor,outlineColor);
        cv::inRange(src,properties.lowerTextColor,properties.upperTextColor,textColor);

        cv::cvtColor(src,src,cv::COLOR_HLS2BGR);
        dbgImage = src.clone();
        cv::cvtColor(src,src,cv::COLOR_BGR2HLS);

        std::vector<Selection> selections = initialSelection(src,contours);
        if (selections.size() <= 1){
            return {};
        }
        start = std::chrono::high_resolution_clock::now();
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> duration = end-start;
        drawImage(selections);

        selections.erase(
            std::remove_if(selections.begin(),selections.end(),
                [this](Selection& selection){
                    if (  properties.topScanBoundary <= selection.getMinRow() 
                          && selection.getMaxRow() <= properties.bottomScanBoundary
                          && selection.getEdgePixelOutlineColorPercentage() >= 0.625
                          && (selection.getHeight() >=3 || selection.getWidth() >= 3) ) 
                        return false;
                    selection.deselect();
                    return true;
                }),
        selections.end());

        drawImage(selections);
        std::vector<Selection> smallerSelections;
        for (int i =0;i<selections.size();i++){
            Selection& selection = selections[i];
            if (!isValidSelection(src,selection) || selection.getEdgePixelOutlineColorPercentage() < 0.85 ){
                selection.disable();
                smallerSelections.push_back(std::move(selection));
                selections.erase(selections.begin()+i);
                i--;
            }
        }
        if  (selections.size() <= 1){
            return {};
        }
        result = selected.toImage();
        drawImage(selections);
        refineSelections(src, selections);
        drawImage(selections);
        if  (selections.size() <= 1){
            return {};
        }
        std::optional<Text> text = findTextRow(textColor, selections,smallerSelections);
        if (text.has_value()){
            cv::imwrite("/Users/laptop/Desktop/Projects/C++/SubReader/out/frames/result.png", text->getImage());   
        }
        if (selections.size() <= 1 
        || abs( (text->getMaxCol()+text->getMinCol())/2 - (properties.leftScanBoundary+properties.rightScanBoundary)/2) > 15) {
            return {};
        }
        return text;
    }

    std::optional<Text> extractTextWithoutOutline(const cv::Mat& src){
        auto totalStart = std::chrono::high_resolution_clock::now();
        auto start = std::chrono::high_resolution_clock::now();
        selected.deselectAll();
        drawSelectionZone(src);

        cv::Mat outlineColor,contours;
        cv::Mat textColor(src.rows,src.cols,CV_8UC3);

        cv::Canny(src,contours,properties.minCanny,properties.maxCanny);
        cv::cvtColor(contours,contours,cv::COLOR_GRAY2BGR);
        std::vector<Selection> selections = initialSelection(src,contours);
        if (selections.size() <= 1){
            return {};
        }
        start = std::chrono::high_resolution_clock::now();
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> duration = end-start;
        drawImage(selections);

        selections.erase(
            std::remove_if(selections.begin(),selections.end(),
                [this](Selection& selection){
                    if ( properties.topScanBoundary <= selection.getMinRow() 
                          && selection.getMaxRow() <= properties.bottomScanBoundary
                          && (selection.getHeight() >=5 || selection.getWidth() >= 5) ) 
                        return false;
                    selection.deselect();
                    return true;
                }),
        selections.end());

        drawImage(selections);

        std::vector<Selection> smallerSelections;
        for (int i =0;i<selections.size();i++){
            Selection& selection = selections[i];
            if (!isValidSelection(src,selection) ){
                selection.disable();
                smallerSelections.push_back(std::move(selection));
                selections.erase(selections.begin()+i);
                i--;
            }
        }
        if  (selections.size() <= 1){
            return {};
        }
        result = selected.toImage();

        refineSelections(src, selections);
        drawImage(selections);
        if  (selections.size() <= 1){
            return {};
        }
        std::optional<Text> text = findTextRow(textColor, selections,smallerSelections);

        if (selections.size() <= 1 
        || abs( (text->getMaxCol()+text->getMinCol())/2 - (properties.leftScanBoundary+properties.rightScanBoundary)/2) > 15) {
            return {};
        }
        return text;
    }
    void drawImage(std::vector<Selection> selections){
        cv::Mat image = selected.toImage();
        drawSelectionBoxes(image,selections);
        cv::imwrite("/Users/laptop/Desktop/Projects/C++/SubReader/out/frames/result.png", image);
    }
    std::vector<Selection> initialSelection(const cv::Mat& src, const cv::Mat& contours){
        std::vector<Selection> selections;  
        for(int row=properties.topScanBoundary; row<properties.bottomScanBoundary; row++){
            for(int col=properties.leftScanBoundary; col<properties.rightScanBoundary; col++){
                const cv::Vec3b& pixel = src.at<cv::Vec3b>(row,col);
                if (    isTextColor(pixel)
                    && !selected.isSelectedAt({col,row})
                    && isNearOutline(contours,{col,row}) ){  
                        selectText(selections,src,{col,row});
                }
            }
        }
        return selections;
    }


    bool isNearOutline(const cv::Mat& image, const cv::Point2i& p){
        int maxRow = std::min(p.y+properties.adjacentPixelSearchLength,image.rows-1);
        int minRow = std::max(p.y-properties.adjacentPixelSearchLength,0);
        int maxCol = std::min(p.x+properties.adjacentPixelSearchLength,image.cols-1);
        int minCol = std::max(p.x-properties.adjacentPixelSearchLength,0);
        
        for (int row=minRow;row<=maxRow;row++){
            for (int col=minCol;col<=maxCol;col++){
                if (image.at<cv::Vec3b>({col,row})[1] < 10) return true;
            }
        }
        return false;
    }
    cv::Mat dbgImage;
    void selectText(std::vector<Selection>& selections, const cv::Mat& image, const cv::Point2i& start){
        std::queue<cv::Point2i> q;
        std::vector<cv::Point2i> allPoints;
        allPoints.reserve(16);
        selected.selectPixel(start); 
        q.push(start);
        double brightness = 0;
        double j = 0;
        std::vector<double> brightnessValues;
        while (!q.empty()){
            cv::Point2i p = q.front();
            q.pop();
            const cv::Point2i left(p.x-1,p.y);
            const cv::Point2i right(p.x+1,p.y);
            const cv::Point2i top(p.x,p.y-1);
            const cv::Point2i bottom(p.x,p.y+1);
            if (p.x > 0 && !selected.isSelectedAt(left)){
                if (isTextColor( image.at<cv::Vec3b>(left) )){//result.at<cv::Vec3b>(left)[1] > 200 ){
                    q.emplace(left);
                    selected.selectPixel(left);
                }
                else{
                    if (isBorder(image,left)) brightness += 1.0;
                    j++;
                    dbgImage.at<cv::Vec3b>(left) = {255,0,255};
                }
            }
            if (p.x < image.cols-1  && !selected.isSelectedAt(right)){
                if (isTextColor(image.at<cv::Vec3b>(right))){//result.at<cv::Vec3b>(right)[1] > 200 ){
                    q.emplace(right);
                    selected.selectPixel(right); 
                }
                else{
                    if (isBorder(image,right)) brightness += 1.0;
                    j++;            
                    dbgImage.at<cv::Vec3b>(left) = {255,0,255};
                }              
            }
            if (p.y > 0 && !selected.isSelectedAt(top)){
                if (isTextColor(image.at<cv::Vec3b>(top))){//result.at<cv::Vec3b>(top)[1] > 200 ){
                    q.emplace(top);
                    selected.selectPixel(top);
                }
                else{
                    if (isBorder(image,top)) brightness += 1.0;
                    j++;
                    dbgImage.at<cv::Vec3b>(left) = {255,0,255};
                }
            }
            if (p.y < image.rows-1 && !selected.isSelectedAt(bottom)){
                if (isTextColor(image.at<cv::Vec3b>(bottom))){//result.at<cv::Vec3b>(bottom)[1] > 200 ){
                    q.emplace(bottom);
                    selected.selectPixel(bottom);
                }
                else{
                    if (isBorder(image,bottom)) brightness += 1.0;
                    j++;
                    dbgImage.at<cv::Vec3b>(left) = {255,0,255};
                }
            }
            allPoints.emplace_back(std::move(p));
        }
        selections.emplace_back(selected,allPoints ,brightness/j);
    }
    
    bool isBorder(const cv::Mat& image, const cv::Point2i& start){
        const int SCAN_DISTANCE = 10;
        int minCol = std::max(0,start.x-SCAN_DISTANCE);
        int maxCol = std::min(image.cols-1,start.x+SCAN_DISTANCE);
        int minRow = std::max(0,start.y-SCAN_DISTANCE);
        int maxRow = std::min(image.rows-1,start.y+SCAN_DISTANCE);
        for (int col = start.x;col <= maxCol;col++){
            auto p = image.at<cv::Vec3b>({col , start.y});
            if (isTextColor(p)) break;
            if (isOutlineColor(p)) return true;
        }
        for (int col = start.x;col >= minCol;col--){
            auto p = image.at<cv::Vec3b>({col , start.y});
            if (isTextColor(p)) break;
            if (isOutlineColor(p)) return true;
        }
        for (int row = start.y;row <= maxRow;row++){
            auto p = image.at<cv::Vec3b>({start.x , row});
            if (isTextColor(p)) break;
            if (isOutlineColor(p)) return true;
        }
        for (int row = start.y;row >= minRow;row--){
            auto p = image.at<cv::Vec3b>({start.x , row});
            if (isTextColor(p)) break;
            if (isOutlineColor(p)) return true;
        }
        return false;
    }
    
    std::optional<Text> findTextRow(cv::Mat& image, std::vector<Selection>& selections, std::vector<Selection>& smallerSelections){
        std::unordered_set<Selection*> allGroupedSelections;
        std::sort(selections.begin(),selections.end(),[](const Selection& t1, const Selection& t2){
            return t1.getMinCol() < t2.getMinCol();
        });
        std::vector<std::array<int,2>> textGroups;
        int k =0;
        int i =0;
        for (i =0; i<selections.size()-1;i++){
            if ( selections[i+1].getMinCol() - selections[i].getMaxCol() > properties.characterGapThreshold){
                textGroups.emplace_back(std::array<int,2>{k,i});
                k = i+1;
            }
        }
        textGroups.emplace_back(std::array<int,2>{k,i});
        std::array<int,2> longestGroup = *std::max_element(textGroups.begin(),textGroups.end(), 
            [](std::array<int,2>& x1, std::array<int,2>& x2){
            return (x1[1]-x1[0]) < (x2[1]-x2[0]);
        });
        std::vector<Selection*> toReturn;
        for (int i =longestGroup[0];i<=longestGroup[1];i++){
            toReturn.emplace_back(&selections[i]);
        }
        for (Selection& selection : selections){
            bool contains = false;
            for (const Selection* s : toReturn){
                if (s == &selection){
                    contains = true;
                    break;
                }
            }
            if (!contains) selection.deselect();
        }
        if (toReturn.size() <= 1) return {};
        return Text(image, selected,toReturn,smallerSelections);
    }
    void mergeSelections(std::vector<Selection>& selections, int verticalMergeTolerance, int horizontalMergeTolerance){
        std::sort(selections.begin(),selections.end(),[](const Selection& t1, const Selection& t2){
            return t1.getMinCol() < t2.getMinCol();
        });
        int i = 0;
        while (i < selections.size()){
            Selection& merge = selections[i];
            for (int j=i+1;j<selections.size();j++){
                Selection& s = selections[j];
                if (Selection::validLineDistance(merge,s,
                verticalMergeTolerance,horizontalMergeTolerance) ){
                    merge.merge(s);
                    selections.erase(selections.begin()+j);
                    j--;
                }
            }
        i++;
        }
    }
    bool isValidSelection(const cv::Mat& src, const Selection& selection){
        size_t numberOfPixels = selection.numberOfPixels();
        if ( (selection.getHeight() < 15 && selection.getWidth() < 15)
            || selection.getWidth() < 8 || selection.getHeight() > 100 || selection.getWidth() > 100 ){
            return false;
        }
        return true;
    }
    bool isAboveColorThreshold(const cv::Mat& src, const Selection& selection){
        unsigned int count = 0;
        for (const cv::Point2i& p : selection.getAllPoints()){
            if ( isTextColor( src.at<cv::Vec3b>(p)) ){
                count++;
            }
        }
        return (double)count/selection.numberOfPixels() > properties.colorThreshold;
    }
    bool isTextColor(const cv::Vec3b& pixel){
        return properties.lowerTextColor[0] <= pixel[0] && pixel[0] <= properties.upperTextColor[0]
            && properties.lowerTextColor[1] <= pixel[1] && pixel[1] <= properties.upperTextColor[1]
            && properties.lowerTextColor[2] <= pixel[2] && pixel[2] <= properties.upperTextColor[2];
    }

    bool isOutlineColor(const cv::Vec3b& pixel){
        return (*properties.lowerOutlineColor)[0] <= pixel[0] 
            && pixel[0] <= (*properties.upperOutlineColor)[0]
            && (*properties.lowerOutlineColor)[1] <= pixel[1] 
            && pixel[1] <= (*properties.upperOutlineColor)[1]
            && (*properties.lowerOutlineColor)[2] <= pixel[2] 
            && pixel[2] <= (*properties.upperOutlineColor)[2];
    }
    void drawSelectionBoxes(cv::Mat& mat, std::vector<Selection>& toDraw){
        static const int THICKNESS = 2;
        for (Selection& s : toDraw){
            cv::Point2i topLeftCorner = cv::Point2i(s.getMinCol(),s.getMinRow());
            cv::Point2i bottomRightCorner = cv::Point2i(s.getMaxCol(),s.getMaxRow());
            cv::rectangle(mat,topLeftCorner,bottomRightCorner,cv::Scalar(255,0,255),THICKNESS);
        }
    }
    void drawSelectionZone(cv::Mat mat){
        mat = mat.clone();
         cv::cvtColor(mat,mat,cv::COLOR_HLS2BGR);
        static const int THICKNESS = 2;
        cv::Point2i topLeftCorner = cv::Point2i(properties.leftScanBoundary,properties.topScanBoundary);
        cv::Point2i bottomRightCorner = cv::Point2i(properties.rightScanBoundary,properties.bottomScanBoundary);
        cv::rectangle(mat,topLeftCorner,bottomRightCorner,cv::Scalar(255,0,255),THICKNESS);
        cv::imwrite("/Users/laptop/Desktop/Projects/C++/SubReader/out/frames/result.png", mat);
    }
};