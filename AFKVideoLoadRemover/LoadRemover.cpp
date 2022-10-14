#include "LoadRemover.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <boost/filesystem/path.hpp>
#include <iostream>
#include <iomanip>
#include <vector>
#include <fstream>
#include <filesystem>

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
    bool isValidInput = false;

    std::fstream fileStream;
    while (!isValidInput)
    {
        std::cout << "Path to video file (inlcuding file name): ";
        std::getline(std::cin >> std::ws, userInput);

        boost::filesystem::path videoPath(userInput);

        fileStream.open(userInput);
        
        if (fileStream.fail())
        {   
            std::cout << "ERROR: File not found" << std::endl << std::endl;
            fileStream.close();
        }
        else if (videoPath.extension().compare(std::string(".mp4")) != 0 && videoPath.extension().compare(std::string(".m4a")) != 0 && videoPath.extension().compare(std::string(".webm")) != 0)
        {
            std::cout << "ERROR: File type must be mp4, m4a, or webm" << std::endl << std::endl;
            fileStream.close();
        }
        else
        {
            isValidInput = true;
        }
    }
    isValidInput = false;

    video = cv::VideoCapture(userInput);
    framerate = video.get(5);
    totalFrameCount = video.get(7);

    system("CLS");

    while (!isValidInput)
    {
        std::cout << "Amount of different kinds of load screens: ";
        std::getline(std::cin >> std::ws, userInput);

        try
        {
            if (stoi(userInput) < 1 || stoi(userInput) > 99)
            {
                std::cout << "ERROR: Input must be >= 1 and <= 99" << std::endl << std::endl;
            }
            else if (!std::all_of(userInput.begin(), userInput.end(), ::isdigit))
            {
                std::cout << "ERROR: Input must only contain numbers" << std::endl << std::endl;
            }
            else
            {
                isValidInput = true;
            }
        }
        catch (std::exception e)
        {
            std::cout << "ERROR: Input must be a number" << std::endl << std::endl;
        }
    }
    isValidInput = false;

    uniqueLoadScreenCount = stoi(userInput);

    cv::Mat mat;
    double num = 0.0;
    for (int i = 0; i < uniqueLoadScreenCount; i++)
    {
        loadFrameCrops.push_back(mat);
        framesForDifference.push_back(mat);
        maxVals.push_back(num);
    }

    system("CLS");

    for (int i = 0; i < uniqueLoadScreenCount; i++)
    {
        while (!isValidInput)
        {
            std::cout << "Load screen timestamp (hhmmss): ";
            std::getline(std::cin >> std::ws, userInput);

            if (userInput.size() != 6)
            {
                std::cout << "ERROR: Timestamp length must equal 6" << std::endl << std::endl;
            }
            else
            {
                if (!std::all_of(userInput.begin(), userInput.end(), ::isdigit))
                {
                    std::cout << "ERROR: Timestamp must only contain numbers" << std::endl << std::endl;
                }
                else
                {
                    isValidInput = true;
                }
            }
        }
        isValidInput = false;

        int seconds = ((userInput[4] * 10) + userInput[5]) - 528;
        int minutes = ((userInput[2] * 10) + userInput[3]) - 528;
        int hours = ((userInput[0] * 10) + userInput[1]) - 528;
        int totalSeconds = seconds + (minutes * 60) + (hours * 3600);

        video.set(cv::CAP_PROP_POS_MSEC, totalSeconds * 1000);
        cv::Mat frame;
        video >> frame;
        cv::Mat frameCrop = frame(cv::Rect(64, 649, 81, 19));
        loadFrameCrops[i] = frameCrop;
    }

    fileStream.close();

    video.set(cv::CAP_PROP_POS_MSEC, 0);
}

void LoadRemover::promptDebugMode()
{
    system("CLS");

    std::string userInput;
    while (userInput.compare("Y") != 0 && userInput.compare("N"))
    {
        std::cout << "Enable debug mode? (Y/N): ";
        std::getline(std::cin >> std::ws, userInput);
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