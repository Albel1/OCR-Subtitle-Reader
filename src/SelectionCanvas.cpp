#include "SelectionCanvas.h"

SelectionCanvas::SelectionCanvas(int rows, int cols)
:rows(rows),cols(cols){
    selected.reserve(rows*cols);
    for (int i =0;i<rows*cols;i++){
        selected.emplace_back(false);
    }
}

bool SelectionCanvas::isSelectedAt(const cv::Point2i& p) const{
    return selected[p.y*cols+p.x];
}

void SelectionCanvas::selectPixel(const cv::Point2i& p){
    selected[p.y*cols+p.x] = true;
}
void SelectionCanvas::deselectPixel(const cv::Point2i& p){
    selected[p.y*cols+p.x] = false;
}
void SelectionCanvas::deselectAll(){
    std::fill(selected.begin(),selected.end(),false);
}
cv::Mat SelectionCanvas::toImage() const{
    cv::Mat image(rows,cols, CV_8UC3, cv::Scalar(0, 0, 0));
    for (int row = 0;row<rows;row++){
        for (int col = 0;col<cols;col++){
            image.at<cv::Vec3b>(row,col) = isSelectedAt({col,row}) ? WHITE_PIXEL:BLACK_PIXEL;
        }
    }
    return image;
}
int SelectionCanvas::getRows() const{
    return rows;
}
int SelectionCanvas::getCols() const{
    return cols;
}
