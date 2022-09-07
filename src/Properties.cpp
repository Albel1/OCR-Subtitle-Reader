#include "Properties.h"

Properties::TextExtractorProperties Properties::toTextExtractorProperties(
        const BaseTextExtractorProperties& properties,const int rows, const int cols){
    long int total = rows*cols;
    return {
        properties.lowerTextColor,
        properties.upperTextColor,
        properties.lowerOutlineColor,
        properties.upperOutlineColor,
        static_cast<int>(properties.minSelectionSize*total),
        static_cast<int>(properties.maxSelectionSize*total),
        properties.maxWidthHeightRatio,
        properties.minWidthHeightRatio,
        static_cast<int>(properties.verticalMergeTolerance*rows),
        static_cast<int>(properties.horizontalMergeTolerance*cols),
        properties.adjacentPixelSearchLength,
        static_cast<int>(properties.leftCentreThreshold*cols),
        static_cast<int>(properties.rightCentreThreshold*cols),
        static_cast<int>(properties.characterGapThreshold*cols),
        static_cast<int>(properties.textDetectionVerticalTolerance*rows),
        static_cast<int>(properties.textDetectionHorizontalTolerance*cols),
        static_cast<int>(properties.leftScanBoundary*cols),
        static_cast<int>(properties.rightScanBoundary*cols),
        static_cast<int>(properties.topScanBoundary*rows),
        static_cast<int>(properties.bottomScanBoundary*rows),
        rows,
        cols,
        properties.minCanny,
        properties.maxCanny,
        properties.dilationSize,
        properties.colorThreshold
        };
};