#include "LoadRemover.hpp"
#include "opencv2\highgui\highgui.hpp"
#include <iostream>
#include <iomanip>
#include <vector>

void LoadRemover::begin()
{
    startSetup();
    promptDebugMode();
    iterateFrames();
    printResultsAndDeleteVideos();
}

void LoadRemover::startSetup()
{
    std::string userInput;

    std::cout << "Currently only supports 720p videos. Use a 60fps recording for better accuracy." << std::endl;
    std::cout << "Enter the location of your video file (Example: 'D:\\Videos\\LoadRemover\\speedrun.mp4'): ";
    std::cin >> userInput;
    std::cout << std::endl;

    video = cv::VideoCapture(userInput);
    framerate = video.get(5);
    totalFrameCount = video.get(7);

    system("CLS");

    std::cout << "Enter the amount of unqiue load screens that this video has: ";
    std::cin >> userInput;

    uniqueLoadScreenCount = std::stoi(userInput);

    cv::Mat mat;
    double num = 0.0;
    for (int i = 0; i < uniqueLoadScreenCount; i++)
    {
        loadFrameCrops.push_back(mat);
        framesForDifference.push_back(mat);
        maxVals.push_back(num);
    }

    system("CLS");

    std::cout << "Enter the timestamps of the different kind of load screens you would like to capture." << std::endl;
    std::cout << "Format your input like this: 'hh mm ss'. Example: '01 15 56'. The load screen displayed at 01h15m56s will be captured." << std::endl;

    for (int i = 0; i < uniqueLoadScreenCount; i++)
    {
        std::cout << "Load screen timestamp: ";
        std::getline(std::cin >> std::ws, userInput);

        if (userInput.compare("END") != 0)
        {
            std::string num;
            std::stringstream ss(userInput);
            std::vector<std::string> tokens;
            while (ss >> num)
            {
                tokens.push_back(num);
            }

            int seconds = std::stoi(tokens[2]);
            int minutes = std::stoi(tokens[1]);
            int hours = std::stoi(tokens[0]);

            int totalSeconds = seconds + (minutes * 60) + (hours * 3600);

            video.set(cv::CAP_PROP_POS_MSEC, totalSeconds * 1000);
            cv::Mat frame;
            video >> frame;
            cv::Mat frameCrop = frame(cv::Rect(64, 649, 81, 19));
            loadFrameCrops[i] = frameCrop;
        }
    }

    video.set(cv::CAP_PROP_POS_MSEC, 0);
}

void LoadRemover::promptDebugMode()
{
    system("CLS");

    std::cout << "Debug mode will:" << std::endl;
    std::cout << " - Show a video player that is pausable/unpausable with the 'P' key " << std::endl;
    std::cout << " - Pause on the frame after a load screen has finished" << std::endl;
    std::cout << "      - If it pauses when the previous frame wasn't a load: it has incorrectly detected a frame as a load" << std::endl;
    std::cout << "      - If it pauses during a load: it isn't detecting a load frame as a load " << std::endl;
    std::cout << " - Output text when it detects a frame as a load" << std::endl << std::endl;

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

        for (int i = 0; i < uniqueLoadScreenCount; i++)
        {
            absdiff(videoFrameCrop, loadFrameCrops[i], framesForDifference[i]);
            minMaxLoc(framesForDifference[i], NULL, &maxVals[i]);
        }

        double threshold = 80.0;
        double lowestMaxVal = *std::min_element(std::begin(maxVals), std::end(maxVals));

        if (lowestMaxVal < threshold)
        {
            loadingFrameCount++;
            printPercentageDone();
            lastFrameWasLoad = true;

            if (debugMode)
            {
                std::cout << "LOAD " << loadingFrameCount << " | ";
                for (double num : maxVals)
                {
                    std::cout << num << " ";
                }
                std::cout << std::endl;
            }
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
            else if (lastFrameWasLoad && lowestMaxVal >= threshold)
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

    int milliseconds = (loadingFrameCount % framerate) * (100.0 / framerate);
    int seconds = (loadingFrameCount / framerate) % 60;
    int minutes = (loadingFrameCount / framerate) / 60;

    video.release();
    for (cv::Mat mat : loadFrameCrops)
    {
        mat.release();
    }
    cv::destroyAllWindows();

    system("CLS");

    std::cout << "--------------------------------------------------------------------------" << std::endl;
    std::cout << "Load frames: " << loadingFrameCount << std::endl;
    std::cout << minutes << "m " << seconds << "s " << milliseconds << "ms  of load screens" << std::endl;
    std::cout << "--------------------------------------------------------------------------" << std::endl;
}