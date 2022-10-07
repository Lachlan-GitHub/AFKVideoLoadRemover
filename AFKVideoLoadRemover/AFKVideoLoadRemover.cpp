#include <iostream>
#include <iomanip>
#include "opencv2\highgui\highgui.hpp"

using namespace std;
using namespace cv;

int main()
{
    VideoCapture video("D:/Videos/LoadRemover/speedrun_720p_30fps.mp4");
    VideoCapture loadingMain("images/720p_MAIN.png");
    VideoCapture loadingProvingEntry("images/720p_ENTER_PROVING.png");
    VideoCapture loadingProvingExit("images/720p_EXIT_PROVING.png");

    Mat videoFrame, videoFrameCrop;
    Mat loadingMainFrame, loadingProvingEntryFrame, loadingProvingExitFrame;
    Mat differenceMain, differenceProvingEntry, differenceProvingExit;

    int loadingFrameCount = 0;
    int currentFrameCount = 0;
    int totalFrameCount = video.get(7);
    int completionPercentage = 0;
    double maxValMain, maxValProvingEntry, maxValProvingExit;

    loadingMain >> loadingMainFrame;
    loadingProvingEntry >> loadingProvingEntryFrame;
    loadingProvingExit >> loadingProvingExitFrame;

    while (1)
    {
        video >> videoFrame;
        if (videoFrame.empty()) break;
        videoFrameCrop = videoFrame(cv::Rect(64, 649, 81, 19));

        absdiff(videoFrameCrop, loadingMainFrame, differenceMain);
        absdiff(videoFrameCrop, loadingProvingEntryFrame, differenceProvingEntry);
        absdiff(videoFrameCrop, loadingProvingExitFrame, differenceProvingExit);

        minMaxLoc(differenceMain, NULL, &maxValMain);
        minMaxLoc(differenceProvingEntry, NULL, &maxValProvingEntry);
        minMaxLoc(differenceProvingExit, NULL, &maxValProvingExit);

        if (maxValMain < 100 || maxValProvingEntry < 100 || maxValProvingExit < 100)
        {
            loadingFrameCount++;

            currentFrameCount = video.get(1);
            if (currentFrameCount / (totalFrameCount / 100) != completionPercentage)
            {
                completionPercentage = currentFrameCount / (totalFrameCount / 100);
                cout << completionPercentage << "%" << endl;
            }            
        }
    }

    int frameRate = video.get(5);
    int milliseconds = (loadingFrameCount % frameRate) * (100.0 / frameRate);
    int seconds = (loadingFrameCount / frameRate) % 60;
    int minutes = (loadingFrameCount / frameRate) / 60;

    cout << "--------------------------------------------------------------------------" << endl;
    cout << "Load frames: " << loadingFrameCount << endl;
    cout << minutes << "m " << seconds << "s " << milliseconds << "ms  of load screens" << endl;
    cout << "--------------------------------------------------------------------------" << endl;

    video.release();
    loadingMain.release();
    loadingProvingEntry.release();
    loadingProvingExit.release();
    destroyAllWindows();

    return 0;
}