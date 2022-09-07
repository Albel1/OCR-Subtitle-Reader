#pragma once
#include <opencv2/opencv.hpp> 
#include <optional>
#include <iostream>
namespace Properties{
    struct TextExtractorProperties{
        const cv::Scalar lowerTextColor;
        const cv::Scalar upperTextColor;
        const std::optional<cv::Scalar> lowerOutlineColor;
        const std::optional<cv::Scalar> upperOutlineColor;
        const int minSelectionSize;
        const int maxSelectionSize;
        const double maxWidthHeightRatio;
        const double minWidthHeightRatio;
        const int verticalMergeTolerance;
        const int horizontalMergeTolerance;
        const int adjacentPixelSearchLength;
        const int leftCentreThreshold;
        const int rightCentreThreshold;
        const int characterGapThreshold;
        const int textDetectionVerticalTolerance;
        const int textDetectionHorizontalTolerance;
        const int leftScanBoundary;
        const int rightScanBoundary;
        const int topScanBoundary;
        const int bottomScanBoundary;
        const int rows;
        const int cols;
        const int minCanny;
        const int maxCanny;
        const double dilationSize;
        const double colorThreshold;
    };
    struct BaseTextExtractorProperties{
        const cv::Scalar lowerTextColor = cv::Scalar(0,210,0);
        const cv::Scalar upperTextColor = cv::Scalar(255,255,255);
        const std::optional<cv::Scalar> lowerOutlineColor = cv::Scalar(0,0,0);
        const std::optional<cv::Scalar> upperOutlineColor = cv::Scalar(255,80,255);
        const double minSelectionSize = 0.0000169932;
        const double maxSelectionSize = 0.00300000000;
        const double maxWidthHeightRatio = 13.0;
        const double minWidthHeightRatio = 0.06;
        const double verticalMergeTolerance = 0.01019329938;
        const double horizontalMergeTolerance = 0.00232275132;
        const int adjacentPixelSearchLength = 3;
        const double leftCentreThreshold = 0.45;
        const double rightCentreThreshold = 0.55;
        const double characterGapThreshold = 0.05645502645;
        const double textDetectionVerticalTolerance = 0.01527495008;
        const double textDetectionHorizontalTolerance = 0.00992063592;
        const double leftScanBoundary = 0.0;
        const double rightScanBoundary = 1.0;
        const double topScanBoundary = 0.89; //.84;//0.89;
        const double bottomScanBoundary = 0.97; //0.93;//0.97;
        const int minCanny = 600;
        const int maxCanny = 800;
        const double dilationSize = 0.8;
        const double colorThreshold = 0.9;
    };
    TextExtractorProperties toTextExtractorProperties(
        const BaseTextExtractorProperties& properties,const int rows, const int cols);
};