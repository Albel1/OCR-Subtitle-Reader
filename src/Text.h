#pragma once
#include "SelectionCanvas.h"
#include "Selection.h"
#include <opencv2/opencv.hpp>
#include <algorithm>

class Text{
public:
    static const int EDGE_GAP = 0;
    Text(cv::Mat& image, SelectionCanvas& selectionCanvas, const std::vector<std::vector<Selection*>>& textRows)
    :rows(textRows.size()){
        if (textRows.empty()) return;
        for (const std::vector<Selection*>& textRow : textRows){
            characterCount += textRow.size();
        }
        minCol = INT_MAX;
        minRow = INT_MAX;
        for (std::vector<Selection*> textRow : textRows){
            for (Selection* selection : textRow){
                if (selection->getMinCol() < minCol) minCol = selection->getMinCol();
                else if (selection->getMaxCol() > maxCol) maxCol = selection->getMaxCol();
                if (selection->getMinRow() < minRow) minRow = selection->getMinRow();
                else if (selection->getMaxRow() > maxRow) maxRow = selection->getMaxRow();
            }
        }
    }
    Text(cv::Mat& image, SelectionCanvas& selectionCanvas, std::vector<Selection*>& textRow, std::vector<Selection>& smallertextRow)
        :rows(1){
        characterCount = textRow.size();
        if (textRow.empty()) return;
        minCol = INT_MAX;
        minRow = INT_MAX;     

        for (Selection* selection : textRow){
            if (selection->getMinCol() < minCol) minCol = selection->getMinCol();
            if (selection->getMaxCol() > maxCol) maxCol = selection->getMaxCol();
            if (selection->getMinRow() < minRow) minRow = selection->getMinRow();
            if (selection->getMaxRow() > maxRow) maxRow = selection->getMaxRow();
        }
        int newMinCol = minCol;
        int newMaxCol = maxCol;
        int newMinRow = minRow;
        int newMaxRow = maxRow;
        for (Selection& s : smallertextRow){
            if (minCol <= s.getMinCol()+8 && s.getMaxCol() <= maxCol+8
            &&  minRow <= s.getMinRow()+8 && s.getMaxRow() <= maxRow+8  ){
                newMinCol = std::min(newMinCol,s.getMinCol());
                newMinRow = std::min(newMinRow,s.getMinRow());
                newMaxCol = std::max(newMaxCol,s.getMaxCol());
                newMaxRow = std::max(newMaxRow,s.getMaxRow());
                s.enable();
                textRow.push_back(&s);
            }
        }
        minCol = newMinCol;
        maxCol = newMaxCol;
        newMinRow = minRow;
        newMaxRow = maxRow;
        int midCol = image.cols/2;
        if ( (maxCol-midCol)-(midCol-minCol) > 0){
            int diff = (maxCol-midCol)-(midCol-minCol);
            int newMinCol = minCol;
            for (Selection& s : smallertextRow){
                if (minCol > s.getMaxCol()
                &&  minRow <= s.getMinRow()+8 && s.getMaxRow() <= maxRow+8
                &&  abs( (maxCol-midCol)-(midCol-s.getMinCol()) ) <= diff){
                    s.enable();
                    textRow.push_back(&s);
                    newMinCol = std::min( s.getMinCol() ,minCol);
                }
            }
            minCol = newMinCol;
        }
        else if ( (midCol-minCol)-(maxCol-midCol) > 0){
            int diff = (midCol-minCol)-(maxCol-midCol);
            int newMaxCol = maxCol;
            for (Selection& s : smallertextRow){
                if (maxCol < s.getMinCol()
                &&  minRow <= s.getMinRow()+8 && s.getMaxRow() <= maxRow+8
                &&  abs ( (midCol-minCol)-(s.getMaxCol()-midCol )) <= diff){
                    s.enable();
                    textRow.push_back(&s);
                    newMaxCol = std::max( s.getMaxCol() ,newMaxCol);
                }
            }
            maxCol = newMaxCol;
        }

        getSection(selectionCanvas, image);
    }
    void normalizeImage(int desiredHeight){
        cv::Mat paddedImage;
        cv::copyMakeBorder(image, paddedImage, 
            BORDER_PADDING_SIZE,  BORDER_PADDING_SIZE,  
            BORDER_PADDING_SIZE,  BORDER_PADDING_SIZE, cv::BORDER_CONSTANT);
        int scaledHeight = desiredHeight + 2*BORDER_PADDING_SIZE;
        int scaledWidth = ( ((double)(scaledHeight)/paddedImage.rows)*paddedImage.cols );
        cv::resize(paddedImage,paddedImage, cv::Size(scaledWidth, scaledHeight), cv::INTER_LINEAR);
        this->image.release();
        this->image = paddedImage;
    }
    cv::Mat normalizeImage() const{
        cv::Mat paddedImage;
        cv::copyMakeBorder(image, paddedImage, 
            BORDER_PADDING_SIZE,  BORDER_PADDING_SIZE,  
            BORDER_PADDING_SIZE,  BORDER_PADDING_SIZE, cv::BORDER_CONSTANT);
        return paddedImage;
    }

    const cv::Mat getImage() const{
        return image;
    }
    const int getMaxRow() const{
        return maxRow;
    }
    const int getMaxCol() const{
        return maxCol;
    }
    const int getMinRow() const{
        return minRow;
    }
    const int getMinCol() const{
        return minCol;
    }
    const int pixelRows() const{
        return maxRow-minRow;
    }
    const int pixelCols() const{
        return maxCol-minCol;
    }
    const void setImage(cv::Mat newImage,int maxCol, int minCol,int maxRow, int minRow){
        this->image.release();
        this->image=newImage;
        this->maxCol=maxCol;
        this->minCol=minCol;
        this->maxRow=maxRow;
        this->minRow=minRow;
    }
    bool isSameText(Text& o){
        if ( abs(this->getMinCol() - o.getMinCol() ) > 20 || abs(this->getMaxCol() - o.getMaxCol()) > 20
            || abs(this->getMinRow() - o.getMinRow()) > 10 || abs(this->getMaxRow() - o.getMaxRow()) > 10) {
                return false;
        }
        const cv::Mat& im1 = this->getImage();
        const cv::Mat& im2 = o.getImage();
        int colStart = std::max(this->getMinCol(),o.getMinCol());
        int colEnd =   std::min(this->getMaxCol(),o.getMaxCol());
        int rowStart = std::max(this->getMinRow(),o.getMinRow());
        int rowEnd =   std::min(this->getMaxRow(),o.getMaxRow());
        int im1rowDiff = im1.rows-rowEnd;
        int im2rowDiff = im2.rows-rowEnd;
        int differenceCount = 0;
        if (rowEnd-rowStart+1 <= 0 || colEnd-colStart+1 <= 0){
            return false;
        }
        cv::Mat image(rowEnd-rowStart+1,colEnd-colStart+1, CV_8UC3, cv::Scalar(0, 0, 0));
        int col1Diff = 0;
        int col2Diff = 0;
        int row1Diff = 0;
        int row2Diff = 0;
        if (o.getMinCol() > this->getMinCol()){
            col1Diff = o.getMinCol()-this->getMinCol();
            col2Diff = 0;
        }
        else{
            col1Diff = 0;
            col2Diff = this->getMinCol()-o.getMinCol();
        }
        if (o.getMinRow() > this->getMinRow()){
            row1Diff = o.getMinRow()-this->getMinRow();
            row2Diff = 0;
        }
        else{
            row1Diff = 0; 
            row2Diff = this->getMinRow()-o.getMinRow();      
        }
        double totalPixelsSize = (double)(colEnd-colStart)*(rowEnd-rowStart);
        int x = 0;
        int y = 0;
        for (int col = colStart;col<colEnd;col++){
            y = 0;
            for (int row = rowStart;row<rowEnd;row++ ){
                if (im1.at<cv::Vec3b>({col-colStart+col1Diff,row-rowStart+row1Diff}) != im2.at<cv::Vec3b>({col-colStart+col2Diff,row-rowStart+row2Diff})){
                    if (++differenceCount/totalPixelsSize > 0.3){
                        return false;
                    }
                }
                else if (im1.at<cv::Vec3b>({col-colStart+col1Diff,row-rowStart+row1Diff})[1] > 100){
                    image.at<cv::Vec3b>({x,y}) = WHITE_PIXEL;
                }
                y++;
            }
            x++;
        }
        this->setImage(image,colEnd,colStart,rowEnd,rowStart);
        return true;
    }

private:    
    const void getSection(SelectionCanvas& selectionCanvas, cv::Mat& toCopy){
        int maxCol = std::min(this->maxCol+EDGE_GAP,selectionCanvas.getCols());
        int minCol = std::max(this->minCol-EDGE_GAP,0);
        int maxRow = std::min(this->maxRow+EDGE_GAP,selectionCanvas.getRows());
        int minRow = std::max(this->minRow-EDGE_GAP,0);
        int imageWidth = maxCol-minCol;
        int imageHeight = maxRow-minRow;
        this->image = {imageHeight,imageWidth, CV_8UC3, cv::Scalar(0, 0, 0)};
        int y = 0;
        for (int row = minRow;row<maxRow;row++){
            int x = 0;
            for (int col = minCol;col<maxCol;col++){
                this->image.at<cv::Vec3b>(y,x) = selectionCanvas.isSelectedAt({col,row}) ? WHITE_PIXEL:BLACK_PIXEL;
                // this->image.at<cv::Vec3b>(y,x) = toCopy.at<cv::Vec3b>({col,row})[0] == 0 
                //     ? BLACK_PIXEL:WHITE_PIXEL;
                x++;
            }
            y++;
        }
    }
    void createImage(SelectionCanvas& selectionCanvas){
        int maxCol = std::min(this->maxCol+EDGE_GAP,selectionCanvas.getCols());
        int minCol = std::max(this->minCol-EDGE_GAP,0);
        int maxRow = std::min(this->maxRow+EDGE_GAP,selectionCanvas.getRows());
        int minRow = std::max(this->minRow-EDGE_GAP,0);
        int imageWidth = maxCol-minCol;
        int imageHeight = maxRow-minRow;
        cv::Mat image(imageHeight,imageWidth, CV_8UC3, cv::Scalar(0, 0, 0));
        int y = 0;
        for (int row = minRow;row<=maxRow;row++){
            int x = 0;
            for (int col = minCol;col<=maxCol;col++){
                image.at<cv::Vec3b>(y,x) = selectionCanvas.isSelectedAt({col,row}) 
                    ? WHITE_PIXEL:BLACK_PIXEL;
                x++;
            }
            y++;
        }
    }

    int maxRow = -1;
    int maxCol = -1;
    int minRow = -1;
    int minCol = -1;
    int characterCount = 0;
    int rows;
    cv::Mat image;
    const static int BORDER_PADDING_SIZE = 8;
};