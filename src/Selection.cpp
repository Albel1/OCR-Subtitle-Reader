#include "Selection.h"

Selection::Selection(SelectionCanvas& selected, std::vector<cv::Point2i>& _allPoints, double edgePixelOutlineColor)
    :selected(&selected),allPoints(std::move(_allPoints)),edgePixelOutlineColor(edgePixelOutlineColor){
        auto [minCol,maxCol] = (std::minmax_element(allPoints.begin(), allPoints.end(), 
            [](cv::Point2i p1,cv::Point2i p2){return p1.x< p2.x;}));
        this->maxCol = (*maxCol).x;
        this->minCol = (*minCol).x;
        auto [minRow,maxRow] = (std::minmax_element(allPoints.begin(), allPoints.end(), 
            [](cv::Point2i p1,cv::Point2i p2){return p1.y< p2.y;}));
        this->maxRow = (*maxRow).y;
        this->minRow = (*minRow).y;
    };
void Selection::disable(){
    if (allPoints.empty()) return;
    for (cv::Point2i p : allPoints){
        selected->deselectPixel(p);
    }
    disabled = true;
}
void Selection::enable(){
    if (allPoints.empty()) return;
    for (cv::Point2i p : allPoints){
        selected->selectPixel(p);
    }
    disabled = false;
}
void Selection::deselect(){
    disable();
    clear();
}
const std::vector<cv::Point2i>& Selection::getAllPoints() const{
    return allPoints;
}
double Selection::getEdgePixelOutlineColorPercentage() const{
    return this->edgePixelOutlineColor;
}
void Selection::merge(Selection& toMerge){
    std::vector<cv::Point2i> toMergePoints = toMerge.allPoints;
    allPoints.insert(allPoints.end(),toMergePoints.begin(),toMergePoints.end());
    maxCol = std::max(getMaxCol(), toMerge.getMaxCol());
    minCol = std::min(getMinCol(), toMerge.getMinCol());
    maxRow = std::max(getMaxRow(), toMerge.getMaxRow());
    minRow = std::min(getMinRow(), toMerge.getMinRow());
    toMerge.clear();
}
bool Selection::validLineDistance(const Selection& s1, const Selection& s2,
    int verticalTolerance, int horizontalTolerance){
    bool left  = s2.getMaxCol() < s1.getMinCol();
    bool right = s1.getMaxCol() < s2.getMinCol();
    bool top   = s2.getMaxRow() < s1.getMinRow();
    bool bottom= s1.getMaxRow() < s2.getMinRow();

    if (!left && !right && !bottom && !top) return true;
    if (left && !top && !bottom) return (s1.getMinCol()-s2.getMaxCol()) < horizontalTolerance;
    if (right && !top && !bottom) return (s2.getMinCol()-s1.getMaxCol()) < horizontalTolerance;
    if (top && !left && !right) return (s1.getMinRow() - s2.getMaxRow()) < verticalTolerance;
    if (bottom && !left && !right) return (s2.getMinRow() - s1.getMaxRow()) < verticalTolerance;

    return false;
}

int Selection::getWidth() const{
    return maxCol-minCol;
}
int Selection::getHeight() const {
    return maxRow-minRow;
}
int Selection::getMaxRow() const{
    return maxRow;
}
int Selection::getMinRow() const{
    return minRow;
}
int Selection::getMaxCol() const{
    return maxCol;
}
int Selection::getMinCol() const{
    return minCol;
}
size_t Selection::numberOfPixels() const{
    return allPoints.size();
}