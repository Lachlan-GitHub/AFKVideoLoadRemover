#include <iostream>
#include "opencv2\highgui\highgui.hpp"

using namespace std;
using namespace cv;

int main()
{
    //Loading sources
    VideoCapture video("D:/speedrun_1080p_60fps.mp4");                      //Video to remove loads on
    VideoCapture loadingMain("1080p_MAIN.png");                  //'Loading...' from the main load screen
    VideoCapture loadingProvingEntry("1080p_ENTER_PROVING.png"); //'Loading...' from the Proving entrance load screen
    VideoCapture loadingProvingExit("1080p_EXIT_PROVING.png");   //'Loading...' from the Proving exit load screen

    //Creating frames
    Mat videoFrame, videoFrameCrop;                                             //Frame from the video and a cropped frame that targets the 'loading...' in videoFrame
    Mat loadingMainFrame, loadingProvingEntryFrame, loadingProvingExitFrame;    //These are the cropped frames that the video compares against. They contain the 'Loading...' in each of the 3 different load screens
    Mat differenceMain, differenceProvingEntry, differenceProvingExit;          //Each of these are layered over their respective frame, in the line above, to find similarity
    int loadScreenBuffer = 0;                                                   //Once 60 frames in a row are found to be load screens, we can start incrementing loadingFrameCount
    double loadingFrameCount = 0;                                               //Increments each time a frame is determined to be a load screen
    double maxValMain, maxValProvingEntry, maxValProvingExit;                   //Stores numbers between 0-255. The lower the number, the greater the chance of currently compared frames being a load screen.

    //Load the cropped 'Loading...' images into frames for use
    loadingMain >> loadingMainFrame;
    loadingProvingEntry >> loadingProvingEntryFrame;
    loadingProvingExit >> loadingProvingExitFrame;

    while (1)
    {
        //Get frame, check if it exists, and then crop it to the positon of 'Loading...'
        video >> videoFrame;
        if (videoFrame.empty()) break;
        videoFrameCrop = videoFrame(cv::Rect(98, 975, 116, 26));

        //Get the absolute difference of each compared pixel
        absdiff(videoFrameCrop, loadingMainFrame, differenceMain);
        absdiff(videoFrameCrop, loadingProvingEntryFrame, differenceProvingEntry);
        absdiff(videoFrameCrop, loadingProvingExitFrame, differenceProvingExit);

        //Store the value of greatest difference found with absdiff()
        minMaxLoc(differenceMain, NULL, &maxValMain);
        minMaxLoc(differenceProvingEntry, NULL, &maxValProvingEntry);
        minMaxLoc(differenceProvingExit, NULL, &maxValProvingExit);

        //If a pixel has a large difference, then it is most likely not a load screen. We start incrementing our frame count after 60 load screen frames
        if (maxValMain < 100 || maxValProvingEntry < 100 || maxValProvingExit < 100)
        {
            loadScreenBuffer++;
            if (loadScreenBuffer == 60)
            {
                loadingFrameCount += 60;
            }
            else if (loadScreenBuffer > 60)
            {
                loadingFrameCount++;
            }
        }
        else
        {
            loadScreenBuffer = 0;
        }
        
        //Uncomment this code to have the video play alongside the maxVals for testing
        //imshow("", videoFrame);
        //char c = (char) waitKey(1);
        //if (c == 27) break;
        //cout << maxValMain << " " << maxValProvingEntry << " " << maxValProvingExit << endl;
    }

    //Result formatting and printing
    int frameRate = 60;
    int seconds = loadingFrameCount / frameRate;
    int secondsRemainder = seconds % 60;
    int minutes = seconds / 60;
    cout << "--------------------------------------------------------------------------" << endl;
    cout << "Load frames: " << loadingFrameCount << endl;
    cout << minutes << "m " << secondsRemainder << "s of load screens" << endl;
    cout << "--------------------------------------------------------------------------" << endl;

    //Deletions
    video.release();
    loadingMain.release();
    loadingProvingEntry.release();
    loadingProvingExit.release();
    destroyAllWindows();

    return 0;
}