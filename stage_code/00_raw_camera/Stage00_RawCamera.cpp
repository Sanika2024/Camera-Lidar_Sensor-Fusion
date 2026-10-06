#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

using namespace std;
using namespace cv;

int main()
{
    // ============================================================
    // STAGE 0
    // RAW CAMERA IMAGES
    // ============================================================

    string dataPath = "../";

    string imgBasePath =
        dataPath + "images/";

    string imgPrefix =
        "KITTI/2011_09_26/image_02/data/000000";

    string outputPath =
        "../results/stages/00_raw_camera/frames/";

    int imgStartIndex = 0;
    int imgEndIndex = 18;
    int imgStepWidth = 1;
    int imgFillWidth = 4;

    cout << endl;
    cout << "==============================================" << endl;
    cout << " STAGE 0: RAW CAMERA IMAGES" << endl;
    cout << "==============================================" << endl;

    cout << "Frames: "
         << imgStartIndex
         << " - "
         << imgEndIndex
         << endl;

    cout << "No detection or processing." << endl;
    cout << "Saving raw camera frames." << endl;

    // ============================================================
    // FRAME LOOP
    // ============================================================

    for (
        int imgIndex = imgStartIndex;
        imgIndex <= imgEndIndex;
        imgIndex += imgStepWidth
    )
    {
        // --------------------------------------------------------
        // Frame number
        // --------------------------------------------------------

        ostringstream imgNumber;

        imgNumber
            << setfill('0')
            << setw(imgFillWidth)
            << imgIndex;

        // --------------------------------------------------------
        // Input image
        // --------------------------------------------------------

        string inputFilename =
            imgBasePath +
            imgPrefix +
            imgNumber.str() +
            ".png";

        cv::Mat image =
            cv::imread(inputFilename);

        if (image.empty())
        {
            cerr
                << "ERROR: Could not load image: "
                << inputFilename
                << endl;

            continue;
        }

        // --------------------------------------------------------
        // Copy raw image
        // --------------------------------------------------------

        cv::Mat output =
            image.clone();

        // --------------------------------------------------------
        // Small label
        // --------------------------------------------------------

        string label =
            "STAGE 0 | RAW CAMERA | Frame: " +
            to_string(imgIndex);

        cv::putText(
            output,
            label,
            cv::Point(20, 35),
            cv::FONT_HERSHEY_SIMPLEX,
            0.75,
            cv::Scalar(255, 255, 255),
            2
        );

        // --------------------------------------------------------
        // Save
        // --------------------------------------------------------

        string outputFilename =
            outputPath +
            "frame_" +
            imgNumber.str() +
            ".png";

        cv::imwrite(
            outputFilename,
            output
        );

        cout
            << "Frame "
            << setw(2)
            << imgIndex
            << " | Saved"
            << endl;
    }

    cout << endl;
    cout << "==============================================" << endl;
    cout << " STAGE 0 COMPLETE" << endl;
    cout << "==============================================" << endl;

    return 0;
}