#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "lidarData.hpp"

using namespace std;
using namespace cv;

int main()
{
    // ============================================================
    // STAGE 3
    // ALL LiDAR POINTS -> CAMERA IMAGE PROJECTION
    // ============================================================

    // Program is executed from build/
    string dataPath = "../";

    string imgBasePath = dataPath + "images/";

    string imgPrefix =
        "KITTI/2011_09_26/image_02/data/000000";

    string lidarPrefix =
        "KITTI/2011_09_26/velodyne_points/data/000000";

    string outputPath =
        "../results/stages/03_lidar_projection/frames/";

    int imgStartIndex = 0;
    int imgEndIndex = 18;
    int imgStepWidth = 1;
    int imgFillWidth = 4;

    // ============================================================
    // CAMERA-LiDAR CALIBRATION
    // Same calibration used by the original project
    // ============================================================

    cv::Mat P_rect_00(
        3, 4, cv::DataType<double>::type
    );

    cv::Mat R_rect_00(
        4, 4, cv::DataType<double>::type
    );

    cv::Mat RT(
        4, 4, cv::DataType<double>::type
    );

    // ------------------------------------------------------------
    // RT : LiDAR -> Camera coordinate transformation
    // ------------------------------------------------------------

    RT.at<double>(0,0) = 7.533745e-03;
    RT.at<double>(0,1) = -9.999714e-01;
    RT.at<double>(0,2) = -6.166020e-04;
    RT.at<double>(0,3) = -4.069766e-03;

    RT.at<double>(1,0) = 1.480249e-02;
    RT.at<double>(1,1) = 7.280733e-04;
    RT.at<double>(1,2) = -9.998902e-01;
    RT.at<double>(1,3) = -7.631618e-02;

    RT.at<double>(2,0) = 9.998621e-01;
    RT.at<double>(2,1) = 7.523790e-03;
    RT.at<double>(2,2) = 1.480755e-02;
    RT.at<double>(2,3) = -2.717806e-01;

    RT.at<double>(3,0) = 0.0;
    RT.at<double>(3,1) = 0.0;
    RT.at<double>(3,2) = 0.0;
    RT.at<double>(3,3) = 1.0;

    // ------------------------------------------------------------
    // R_rect_00 : rectification matrix
    // ------------------------------------------------------------

    R_rect_00.at<double>(0,0) = 9.999239e-01;
    R_rect_00.at<double>(0,1) = 9.837760e-03;
    R_rect_00.at<double>(0,2) = -7.445048e-03;
    R_rect_00.at<double>(0,3) = 0.0;

    R_rect_00.at<double>(1,0) = -9.869795e-03;
    R_rect_00.at<double>(1,1) = 9.999421e-01;
    R_rect_00.at<double>(1,2) = -4.278459e-03;
    R_rect_00.at<double>(1,3) = 0.0;

    R_rect_00.at<double>(2,0) = 7.402527e-03;
    R_rect_00.at<double>(2,1) = 4.351614e-03;
    R_rect_00.at<double>(2,2) = 9.999631e-01;
    R_rect_00.at<double>(2,3) = 0.0;

    R_rect_00.at<double>(3,0) = 0.0;
    R_rect_00.at<double>(3,1) = 0.0;
    R_rect_00.at<double>(3,2) = 0.0;
    R_rect_00.at<double>(3,3) = 1.0;

    // ------------------------------------------------------------
    // P_rect_00 : camera projection matrix
    // ------------------------------------------------------------

    P_rect_00.at<double>(0,0) = 7.215377e+02;
    P_rect_00.at<double>(0,1) = 0.0;
    P_rect_00.at<double>(0,2) = 6.095593e+02;
    P_rect_00.at<double>(0,3) = 0.0;

    P_rect_00.at<double>(1,0) = 0.0;
    P_rect_00.at<double>(1,1) = 7.215377e+02;
    P_rect_00.at<double>(1,2) = 1.728540e+02;
    P_rect_00.at<double>(1,3) = 0.0;

    P_rect_00.at<double>(2,0) = 0.0;
    P_rect_00.at<double>(2,1) = 0.0;
    P_rect_00.at<double>(2,2) = 1.0;
    P_rect_00.at<double>(2,3) = 0.0;

    // ============================================================
    // START
    // ============================================================

    cout << endl;
    cout << "==============================================" << endl;
    cout << " STAGE 3: ALL LiDAR -> CAMERA PROJECTION" << endl;
    cout << "==============================================" << endl;

    cout << "Frames: "
         << imgStartIndex
         << " - "
         << imgEndIndex
         << endl;

    cout << "NO LiDAR filtering in this stage." << endl;
    cout << "ALL raw LiDAR points will be projected." << endl;

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
        // Camera image
        // --------------------------------------------------------

        string imgFullFilename =
            imgBasePath +
            imgPrefix +
            imgNumber.str() +
            ".png";

        cv::Mat cameraImg =
            cv::imread(imgFullFilename);

        if (cameraImg.empty())
        {
            cerr
                << "ERROR: Could not load image: "
                << imgFullFilename
                << endl;

            continue;
        }

        // --------------------------------------------------------
        // LiDAR file
        // --------------------------------------------------------

        string lidarFullFilename =
            imgBasePath +
            lidarPrefix +
            imgNumber.str() +
            ".bin";

        vector<LidarPoint> lidarPoints;

        loadLidarFromFile(
            lidarPoints,
            lidarFullFilename
        );

        if (lidarPoints.empty())
        {
            cerr
                << "ERROR: No LiDAR points loaded: "
                << lidarFullFilename
                << endl;

            continue;
        }

        size_t rawCount =
            lidarPoints.size();

        // --------------------------------------------------------
        // Create visualization
        // --------------------------------------------------------

        cv::Mat visImg =
            cameraImg.clone();

        cv::Mat overlay =
            cameraImg.clone();

        int projectedCount = 0;

        // ========================================================
        // PROJECT EVERY RAW LiDAR POINT
        // ========================================================

        for (const auto &point : lidarPoints)
        {
            // ----------------------------------------------------
            // Homogeneous LiDAR point
            // ----------------------------------------------------

            cv::Mat X(
                4,
                1,
                cv::DataType<double>::type
            );

            X.at<double>(0,0) = point.x;
            X.at<double>(1,0) = point.y;
            X.at<double>(2,0) = point.z;
            X.at<double>(3,0) = 1.0;

            // ----------------------------------------------------
            // EXACT PROJECT PIPELINE
            //
            // LiDAR
            //   ↓
            // RT
            //   ↓
            // R_rect_00
            //   ↓
            // P_rect_00
            //   ↓
            // Camera image
            // ----------------------------------------------------

            cv::Mat Y =
                P_rect_00 *
                R_rect_00 *
                RT *
                X;

            double depth =
                Y.at<double>(2,0);

            // ----------------------------------------------------
            // Safety check
            // ----------------------------------------------------

            if (!std::isfinite(depth) ||
                depth <= 0.0)
            {
                continue;
            }

            double u =
                Y.at<double>(0,0) /
                depth;

            double v =
                Y.at<double>(1,0) /
                depth;

            if (!std::isfinite(u) ||
                !std::isfinite(v))
            {
                continue;
            }

            int px =
                static_cast<int>(
                    std::round(u)
                );

            int py =
                static_cast<int>(
                    std::round(v)
                );

            // ----------------------------------------------------
            // Keep only points visible inside camera image
            // ----------------------------------------------------

            if (
                px < 0 ||
                px >= visImg.cols ||
                py < 0 ||
                py >= visImg.rows
            )
            {
                continue;
            }

            // ----------------------------------------------------
            // Distance-based color
            //
            // Near  -> red
            // Far   -> blue
            //
            // Based on forward LiDAR distance X.
            // ----------------------------------------------------

            double normalized =
                point.x / 40.0;

            normalized =
                std::max(
                    0.0,
                    std::min(
                        1.0,
                        normalized
                    )
                );

            int red =
                static_cast<int>(
                    255.0 *
                    (1.0 - normalized)
                );

            int blue =
                static_cast<int>(
                    255.0 *
                    normalized
                );

            int green =
                static_cast<int>(
                    180.0 *
                    (1.0 -
                     std::abs(
                         normalized - 0.5
                     ) * 2.0)
                );

            cv::Scalar pointColor(
                blue,
                green,
                red
            );

            // ----------------------------------------------------
            // Draw projected LiDAR point
            // ----------------------------------------------------

            cv::circle(
                overlay,
                cv::Point(px, py),
                3,
                pointColor,
                -1
            );

            projectedCount++;
        }

        // ========================================================
        // BLEND LiDAR WITH CAMERA
        // ========================================================

        cv::addWeighted(
            overlay,
            0.80,
            visImg,
            0.20,
            0.0,
            visImg
        );

        // ========================================================
        // INFORMATION PANEL
        // ========================================================

        string frameText =
            "STAGE 3 | Frame: " +
            to_string(imgIndex);

        string rawText =
            "Raw LiDAR points: " +
            to_string(rawCount);

        string projectedText =
            "Projected points in image: " +
            to_string(projectedCount);

        string infoText =
            "ALL LiDAR points projected - NO filtering";

        cv::putText(
            visImg,
            frameText,
            cv::Point(20, 35),
            cv::FONT_HERSHEY_SIMPLEX,
            0.75,
            cv::Scalar(255,255,255),
            2
        );

        cv::putText(
            visImg,
            rawText,
            cv::Point(20, 70),
            cv::FONT_HERSHEY_SIMPLEX,
            0.60,
            cv::Scalar(255,255,255),
            2
        );

        cv::putText(
            visImg,
            projectedText,
            cv::Point(20, 100),
            cv::FONT_HERSHEY_SIMPLEX,
            0.60,
            cv::Scalar(255,255,255),
            2
        );

        cv::putText(
            visImg,
            infoText,
            cv::Point(20, 130),
            cv::FONT_HERSHEY_SIMPLEX,
            0.55,
            cv::Scalar(255,255,255),
            2
        );

        // --------------------------------------------------------
        // Save frame
        // --------------------------------------------------------

        string outputFilename =
            outputPath +
            "frame_" +
            imgNumber.str() +
            ".png";

        cv::imwrite(
            outputFilename,
            visImg
        );

        cout
            << "Frame "
            << setw(2)
            << imgIndex
            << " | Raw LiDAR: "
            << setw(7)
            << rawCount
            << " | Projected: "
            << setw(7)
            << projectedCount
            << " | Saved"
            << endl;
    }

    cout << endl;
    cout << "==============================================" << endl;
    cout << " STAGE 3 COMPLETE" << endl;
    cout << "==============================================" << endl;

    return 0;
}