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
    LoadRemover::startSetup();
    LoadRemover::promptDebugMode();
    LoadRemover::iterateFrames();
    LoadRemover::printResultsAndDeleteVideos();
}

cv::Point point1;
cv::Point point2;
void getMouseClickPosition(int event, int x, int y, int flags, void* userdata)
{
    if (event == cv::EVENT_LBUTTONDOWN)
    {
        if (point1.x == NULL)
        {
            point1.x = x;
            point1.y = y;
        }
        else if (point2.x == NULL)
        {
            point2.x = x;
            point2.y = y;
            cv::destroyAllWindows();
        }
    }
    else if (event == cv::EVENT_RBUTTONDOWN)
    {
        cv::destroyAllWindows();
    }
}

void LoadRemover::startSetup()
{
    std::string userInput;
    bool isValidInput = false;

    std::fstream fileStream;
    while (!isValidInput)
    {
        std::cout << "Please refer to the user guide to learn how to use this load remover" << std::endl << std::endl;

        std::cout << "Path to video file: ";
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
    resolution = video.get(4);
    framerate = video.get(5);
    totalFrameCount = video.get(7);

    std::system("CLS");

    while (!isValidInput)
    {
        std::cout << "Amount of different kinds of load screens: ";
        std::getline(std::cin >> std::ws, userInput);

        try
        {
            if (std::stoi(userInput) < 1 || std::stoi(userInput) > 99)
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

    uniqueLoadScreenCount = std::stoi(userInput);

    cv::Mat mat;
    double num = 0.0;
    for (int i = 0; i < uniqueLoadScreenCount; i++)
    {
        loadFrameCrops.push_back(mat);
        framesForDifference.push_back(mat);
        maxVals.push_back(num);
    }

    std::system("CLS");

    bool isConfirmedCorrect = false;
    while (!isConfirmedCorrect)
    {
        for (int i = 0; i < uniqueLoadScreenCount; i++)
        {
            while (!isValidInput)
            {
                std::system("CLS");

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

            if (i == 0)
            {
                cv::namedWindow("Area Selection");
                cv::setMouseCallback("Area Selection", getMouseClickPosition, NULL);
                cv::imshow("Area Selection", frame);
                cv::waitKey();
                cropArea.x = point1.x;
                cropArea.y = point1.y;
                cropArea.width = point2.x - point1.x;
                cropArea.height = point2.y - point1.y;
            }

            cv::Mat frameCrop = frame(cropArea);
            loadFrameCrops[i] = frameCrop;
        }

        std::system("CLS");

        std::cout << "Your frame selections will be shown. Close each one to move onto the next. Press enter to proceed: ";
        std::cin.ignore();

        for (int i = 0; i < loadFrameCrops.size(); i++)
        {
            std::string windowName = std::to_string(i + 1) + "/" + std::to_string(loadFrameCrops.size());

            cv::namedWindow(windowName);
            cv::setMouseCallback(windowName, getMouseClickPosition, NULL);
            cv::imshow(windowName, loadFrameCrops[i]);

            std::system("CLS");

            cv::waitKey(0);
        }

        std::system("CLS");

        while (userInput.compare("Y") != 0 && userInput.compare("N"))
        {
            std::cout << "Proceed with these selections? (Y/N): ";
            std::getline(std::cin >> std::ws, userInput);
        }

        if (userInput.compare("Y") == 0)
        {
            isConfirmedCorrect = true;
        }
        else
        {
            point1.x = NULL;
            point2.x = NULL;
        }
    }

    fileStream.close();

    video.set(cv::CAP_PROP_POS_MSEC, 0);
}

void LoadRemover::promptDebugMode()
{
    std::system("CLS");

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
        videoFrameCrop = videoFrame(cropArea);

        for (int i = 0; i < uniqueLoadScreenCount; i++)
        {
            cv::absdiff(videoFrameCrop, loadFrameCrops[i], framesForDifference[i]);
            cv::minMaxLoc(framesForDifference[i], NULL, &maxVals[i]);
        }

        double threshold = 80.0;
        double lowestMaxVal = *std::min_element(std::begin(maxVals), std::end(maxVals));

        if (lowestMaxVal < threshold)
        {
            loadingFrameCount++;
            
            int currentFrameCount = video.get(1);

            if (currentFrameCount / (totalFrameCount / 100) != completionPercentage)
            {
                system("CLS");

                completionPercentage = currentFrameCount / (totalFrameCount / 100);
                std::cout << completionPercentage << "%" << std::endl;
            }

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
            cv::imshow("Debug Output", videoFrame);

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

void LoadRemover::printResultsAndDeleteVideos()
{
    std::system("CLS");

    int milliseconds = (loadingFrameCount % framerate) * (100.0 / framerate);
    int seconds = (loadingFrameCount / framerate) % 60;
    int minutes = (loadingFrameCount / framerate) / 60;

    video.release();
    for (cv::Mat mat : loadFrameCrops)
    {
        mat.release();
    }
    cv::destroyAllWindows();

    std::system("CLS");

    std::cout << "--------------------------------------------------------------------------" << std::endl;
    std::cout << "Load frames: " << loadingFrameCount << std::endl;
    std::cout << minutes << "m " << seconds << "s " << milliseconds << "ms  of load screens" << std::endl;
    std::cout << "--------------------------------------------------------------------------" << std::endl;

    std::system("pause");
}