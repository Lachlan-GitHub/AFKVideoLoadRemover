#ifndef LoadRemover_H
#define LoadRemover_H

#include "opencv2\highgui\highgui.hpp"

class LoadRemover
{
public:
	void begin();
	void generateVideoAndFrames();
	void promptDebugMode();
	void iterateFrames();
	void printPercentageDone();
	void printResultsAndDeleteVideos();

private:
	bool debugMode;
	bool lastFrameWasLoad;
	cv::VideoCapture video;
	std::vector<cv::VideoCapture> loadPatterns;
	std::vector<double> maxVals;
	std::vector<cv::Mat> loadFrameCrops;
	std::vector<cv::Mat> framesForDifference;
	cv::Mat videoFrame;
	cv::Mat videoFrameCrop;
	int loadingFrameCount;
	int totalFrameCount;
	int completionPercentage;
};

#endif //LoadRemover_H
