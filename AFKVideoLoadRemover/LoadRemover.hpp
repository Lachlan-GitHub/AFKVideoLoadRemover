#ifndef LoadRemover_H
#define LoadRemover_H

#include "opencv2\highgui\highgui.hpp"

class LoadRemover
{
public:
	void begin();
	void startSetup();
	void promptDebugMode();
	void iterateFrames();
	void printPercentageDone();
	void printResultsAndDeleteVideos();

private:
	bool debugMode;
	bool lastFrameWasLoad;
	cv::VideoCapture video;
	std::vector<cv::Mat> loadFrameCrops;
	std::vector<cv::Mat> framesForDifference;
	std::vector<double> maxVals;
	cv::Mat videoFrame;
	cv::Mat videoFrameCrop;
	int uniqueLoadScreenCount;
	int loadingFrameCount;
	int framerate;
	int totalFrameCount;
	int completionPercentage;
};

#endif //LoadRemover_H
