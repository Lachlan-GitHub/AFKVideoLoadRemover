#include "LoadRemover.hpp"
#include "opencv2\highgui\highgui.hpp"
#include <iostream>
#include <iomanip>
#include <vector>

void LoadRemover::begin()
{
    generateVideoAndFrames();
    promptDebugMode();
    iterateFrames();
    printResultsAndDeleteVideos();
}

void LoadRemover::generateVideoAndFrames()
{
    video = cv::VideoCapture("D:/Videos/LoadRemover/speedrun_720p_30fps.mp4");
    loadPatterns.push_back(cv::VideoCapture("images/720p_MAIN.png"));
    loadPatterns.push_back(cv::VideoCapture("images/720p_ENTER_PROVING.png"));
    loadPatterns.push_back(cv::VideoCapture("images/720p_EXIT_PROVING.png"));

    totalFrameCount = video.get(7);

    cv::Mat mat;
    double num = 0.0;

    for (int i = 0; i < 3; i++)
    {
        loadFrameCrops.push_back(mat);
        framesForDifference.push_back(mat);
        maxVals.push_back(num);
    }

    loadPatterns[0] >> loadFrameCrops[0];
    loadPatterns[1] >> loadFrameCrops[1];
    loadPatterns[2] >> loadFrameCrops[2];
}

void LoadRemover::promptDebugMode()
{
    system("CLS");

    std::cout << "Debug mode will:" << std::endl;
    std::cout << " - Show a video player that is pausable/unpausable with the 'P' key " << std::endl;
    std::cout << " - Pause on the frame after a load screen has finished" << std::endl;
    std::cout << "      - If it suddenly pauses when there was no recent load, then " << std::endl;
    std::cout << "        it has incorrectly detected a frame as a load " << std::endl << std::endl;

    std::string userInput;
    while (userInput.compare("Y") != 0 && userInput.compare("N"))
    {
        std::cout << "Enable debug mode? (Y/N):";
        std::cin >> userInput;
    }

    if (userInput.compare("Y") == 0)
    {
        debugMode = true;
    }
}

void LoadRemover::iterateFrames()
{
    while (1)
    {
        video >> videoFrame;
        if (videoFrame.empty())
        {
            break;
        }
        videoFrameCrop = videoFrame(cv::Rect(64, 649, 81, 19));

        absdiff(videoFrameCrop, loadFrameCrops[0], framesForDifference[0]);
        absdiff(videoFrameCrop, loadFrameCrops[1], framesForDifference[1]);
        absdiff(videoFrameCrop, loadFrameCrops[2], framesForDifference[2]);

        minMaxLoc(framesForDifference[0], NULL, &maxVals[0]);
        minMaxLoc(framesForDifference[1], NULL, &maxVals[1]);
        minMaxLoc(framesForDifference[2], NULL, &maxVals[2]);

        if (maxVals[0] < 100.0 || maxVals[1] < 100 || maxVals[2] < 100)
        {
            loadingFrameCount++;
            printPercentageDone();
            lastFrameWasLoad = true;
        }

        if (debugMode)
        {
            imshow("Debug Output", videoFrame);

            char c = cv::waitKey(1);
            if (c == 27)
            {
                break;
            }
            else if (c == 'p')
            {
                while (cv::waitKey(1) != 'p');
            }
            else if (lastFrameWasLoad && maxVals[0] >= 100.0 && maxVals[1] >= 100 && maxVals[2] >= 100)
            {
                lastFrameWasLoad = false;
                while (cv::waitKey(1) != 'p');
            }
        }
    }
}

void LoadRemover::printPercentageDone()
{
    int currentFrameCount = video.get(1);

    if (currentFrameCount / (totalFrameCount / 100) != completionPercentage)
    {
        system("CLS");

        completionPercentage = currentFrameCount / (totalFrameCount / 100);
        std::cout << completionPercentage << "%" << std::endl;
    }
}

void LoadRemover::printResultsAndDeleteVideos()
{
    system("CLS");

    int frameRate = video.get(5);
    int milliseconds = (loadingFrameCount % frameRate) * (100.0 / frameRate);
    int seconds = (loadingFrameCount / frameRate) % 60;
    int minutes = (loadingFrameCount / frameRate) / 60;

    video.release();
    loadFrameCrops[0].release();
    loadFrameCrops[1].release();
    loadFrameCrops[2].release();
    cv::destroyAllWindows();

    system("CLS");

    std::cout << "--------------------------------------------------------------------------" << std::endl;
    std::cout << "Load frames: " << loadingFrameCount << std::endl;
    std::cout << minutes << "m " << seconds << "s " << milliseconds << "ms  of load screens" << std::endl;
    std::cout << "--------------------------------------------------------------------------" << std::endl;
}