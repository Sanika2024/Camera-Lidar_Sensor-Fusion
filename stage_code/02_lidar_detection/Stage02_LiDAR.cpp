#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>
#include <cmath>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#include "dataStructures.h"
#include "lidarData.hpp"

using namespace std;

int main()
{
    /* ============================================================
       STAGE 2 : RAW LiDAR POINT CLOUD
       ============================================================ */

    // ------------------------------------------------------------
    // 1. PATHS
    // ------------------------------------------------------------
    string dataPath = "./";
    string imgBasePath = dataPath + "images/";

    string lidarPrefix =
        "KITTI/2011_09_26/velodyne_points/data/000000";

    string lidarFileType = ".bin";

    string outputPath =
        "./results/stages/02_lidar_detection/frames/";

    // ------------------------------------------------------------
    // 2. FRAME RANGE
    // ------------------------------------------------------------

    int imgStartIndex = 0;
    int imgEndIndex = 18;
    int imgFillWidth = 4;

    // ------------------------------------------------------------
    // 3. ORIGINAL PIPELINE FILTER PARAMETERS
    //
    // These are NOT used to remove points from the Stage 2
    // visualization.
    //
    // We calculate the filtered count separately so that Stage 2
    // documents what the original pipeline does.
    // ------------------------------------------------------------

    float minZ = -1.5;
    float maxZ = -0.9;

    float minX = 2.0;
    float maxX = 20.0;

    float maxY = 2.0;

    float minR = 0.1;

    // ------------------------------------------------------------
    // 4. LiDAR TOP-VIEW VISUALIZATION
    // ------------------------------------------------------------

    int imageWidth = 1000;
    int imageHeight = 900;

    // Raw point-cloud visualization range
    //
    // X = forward direction
    // Y = lateral direction

    double viewMinX = 0.0;
    double viewMaxX = 40.0;

    double viewMinY = -20.0;
    double viewMaxY = 20.0;

    // ------------------------------------------------------------
    // 5. PROCESS ALL LiDAR FRAMES
    // ------------------------------------------------------------

    for (int imgIndex = imgStartIndex;
         imgIndex <= imgEndIndex;
         imgIndex++)
    {
        // --------------------------------------------------------
        // Create frame number
        // --------------------------------------------------------

        ostringstream imgNumber;

        imgNumber << setfill('0')
                  << setw(imgFillWidth)
                  << imgIndex;

        cout << endl;
        cout << "========================================"
             << endl;

        cout << "Processing LiDAR frame: "
             << imgNumber.str()
             << endl;

        // --------------------------------------------------------
        // Build LiDAR filename
        // --------------------------------------------------------

        string lidarFullFilename =
            imgBasePath +
            lidarPrefix +
            imgNumber.str() +
            lidarFileType;

        cout << "File: "
             << lidarFullFilename
             << endl;

        // --------------------------------------------------------
        // 6. LOAD RAW LiDAR POINTS
        // --------------------------------------------------------

        vector<LidarPoint> rawLidarPoints;

        loadLidarFromFile(
            rawLidarPoints,
            lidarFullFilename
        );

        size_t rawPointCount =
            rawLidarPoints.size();

        cout << "Raw LiDAR points: "
             << rawPointCount
             << endl;

        // --------------------------------------------------------
        // 7. CREATE FILTERED COPY
        //
        // The raw points remain untouched.
        // This is only used to calculate the number of points
        // that survive the original pipeline's spatial filter.
        // --------------------------------------------------------

        vector<LidarPoint> filteredLidarPoints =
            rawLidarPoints;

        cropLidarPoints(
            filteredLidarPoints,
            minX,
            maxX,
            maxY,
            minZ,
            maxZ,
            minR
        );

        size_t filteredPointCount =
            filteredLidarPoints.size();

        cout << "Points after original pipeline filter: "
             << filteredPointCount
             << endl;

        // --------------------------------------------------------
        // 8. CREATE BLACK LiDAR VISUALIZATION
        // --------------------------------------------------------

        cv::Mat topView(
            imageHeight,
            imageWidth,
            CV_8UC3,
            cv::Scalar(0, 0, 0)
        );

        // --------------------------------------------------------
        // 9. SUBTLE DISTANCE GRID
        // --------------------------------------------------------

        // Forward-distance lines
        for (int x = 0; x <= 40; x += 5)
        {
            int py =
                imageHeight -
                static_cast<int>(
                    (x - viewMinX) /
                    (viewMaxX - viewMinX) *
                    imageHeight
                );

            cv::line(
                topView,
                cv::Point(0, py),
                cv::Point(imageWidth, py),
                cv::Scalar(35, 35, 35),
                1
            );

            string label =
                to_string(x) + "m";

            cv::putText(
                topView,
                label,
                cv::Point(8, py - 5),
                cv::FONT_HERSHEY_SIMPLEX,
                0.45,
                cv::Scalar(130, 130, 130),
                1
            );
        }

        // Lateral-position lines
        for (int y = -20; y <= 20; y += 5)
        {
            int px =
                static_cast<int>(
                    (y - viewMinY) /
                    (viewMaxY - viewMinY) *
                    imageWidth
                );

            cv::line(
                topView,
                cv::Point(px, 0),
                cv::Point(px, imageHeight),
                cv::Scalar(25, 25, 25),
                1
            );
        }

        // --------------------------------------------------------
        // 10. DRAW EGO VEHICLE
        // --------------------------------------------------------

        int egoX =
            imageWidth / 2;

        int egoY =
            imageHeight - 20;

        cv::rectangle(
            topView,
            cv::Point(
                egoX - 18,
                egoY - 35
            ),
            cv::Point(
                egoX + 18,
                egoY
            ),
            cv::Scalar(255, 255, 255),
            cv::FILLED
        );

        cv::putText(
            topView,
            "EGO",
            cv::Point(
                egoX - 18,
                egoY + 20
            ),
            cv::FONT_HERSHEY_SIMPLEX,
            0.45,
            cv::Scalar(255, 255, 255),
            1
        );

        // --------------------------------------------------------
        // 11. PLOT ALL RAW LiDAR POINTS
        //
        // Point color represents forward distance:
        //
        // Near  -> red/orange
        // Far   -> blue
        //
        // This is only visualization.
        // No points are removed because of this coloring.
        // --------------------------------------------------------

        size_t plottedPointCount = 0;

        for (const auto &point : rawLidarPoints)
        {
            // ----------------------------------------------------
            // Only exclude points outside the DISPLAY window.
            //
            // This is NOT the sensor filtering used by the
            // original pipeline.
            // ----------------------------------------------------

            if (point.x < viewMinX ||
                point.x > viewMaxX ||
                point.y < viewMinY ||
                point.y > viewMaxY)
            {
                continue;
            }

            // ----------------------------------------------------
            // Convert LiDAR coordinates to image coordinates
            // ----------------------------------------------------

            int px =
                static_cast<int>(
                    (point.y - viewMinY) /
                    (viewMaxY - viewMinY) *
                    imageWidth
                );

            int py =
                imageHeight -
                static_cast<int>(
                    (point.x - viewMinX) /
                    (viewMaxX - viewMinX) *
                    imageHeight
                );

            // ----------------------------------------------------
            // Calculate normalized distance
            // ----------------------------------------------------

            double normalizedDistance =
                (point.x - viewMinX) /
                (viewMaxX - viewMinX);

            normalizedDistance =
                std::max(
                    0.0,
                    std::min(
                        1.0,
                        normalizedDistance
                    )
                );

            // ----------------------------------------------------
            // Distance-based color
            //
            // Near = red
            // Middle = green/yellow
            // Far = blue
            // ----------------------------------------------------

            int hue =
                static_cast<int>(
                    (1.0 - normalizedDistance) * 120.0
                );

            cv::Mat hsvPixel(
                1,
                1,
                CV_8UC3
            );

            hsvPixel.at<cv::Vec3b>(0, 0) =
                cv::Vec3b(
                    static_cast<unsigned char>(hue),
                    255,
                    255
                );

            cv::Mat bgrPixel;

            cv::cvtColor(
                hsvPixel,
                bgrPixel,
                cv::COLOR_HSV2BGR
            );

            cv::Vec3b color =
                bgrPixel.at<cv::Vec3b>(0, 0);

            // ----------------------------------------------------
            // Draw point
            // ----------------------------------------------------

            cv::circle(
                topView,
                cv::Point(px, py),
                1,
                cv::Scalar(
                    color[0],
                    color[1],
                    color[2]
                ),
                cv::FILLED
            );

            plottedPointCount++;
        }

        // --------------------------------------------------------
        // 12. TITLE
        // --------------------------------------------------------

        string title =
            "Stage 2 - Raw LiDAR Point Cloud | Frame " +
            imgNumber.str();

        cv::putText(
            topView,
            title,
            cv::Point(20, 30),
            cv::FONT_HERSHEY_SIMPLEX,
            0.8,
            cv::Scalar(255, 255, 255),
            2
        );

        // --------------------------------------------------------
        // 13. POINT INFORMATION
        // --------------------------------------------------------

        string pointInfo =
            "Raw points: " +
            to_string(rawPointCount) +
            " | Displayed: " +
            to_string(plottedPointCount);

        cv::putText(
            topView,
            pointInfo,
            cv::Point(20, 60),
            cv::FONT_HERSHEY_SIMPLEX,
            0.55,
            cv::Scalar(220, 220, 220),
            1
        );

        // --------------------------------------------------------
        // 14. ORIGINAL PIPELINE FILTER INFORMATION
        // --------------------------------------------------------

        string filterInfo =
            "Original filter: X 2-20m | Y <= 2m | Z -1.5 to -0.9m | R > 0.1";

        cv::putText(
            topView,
            filterInfo,
            cv::Point(20, 85),
            cv::FONT_HERSHEY_SIMPLEX,
            0.45,
            cv::Scalar(170, 170, 170),
            1
        );

        string filteredInfo =
            "After original filter: " +
            to_string(filteredPointCount) +
            " points";

        cv::putText(
            topView,
            filteredInfo,
            cv::Point(20, 108),
            cv::FONT_HERSHEY_SIMPLEX,
            0.5,
            cv::Scalar(170, 170, 170),
            1
        );

        // --------------------------------------------------------
        // 15. COLOR LEGEND
        // --------------------------------------------------------

        cv::putText(
            topView,
            "Near",
            cv::Point(
                imageWidth - 180,
                30
            ),
            cv::FONT_HERSHEY_SIMPLEX,
            0.45,
            cv::Scalar(0, 80, 255),
            1
        );

        cv::putText(
            topView,
            "Far",
            cv::Point(
                imageWidth - 70,
                30
            ),
            cv::FONT_HERSHEY_SIMPLEX,
            0.45,
            cv::Scalar(255, 100, 0),
            1
        );

        // --------------------------------------------------------
        // 16. SAVE OUTPUT
        // --------------------------------------------------------

        string outputFile =
            outputPath +
            "frame_" +
            imgNumber.str() +
            ".png";

        bool saved =
            cv::imwrite(
                outputFile,
                topView
            );

        if (saved)
        {
            cout << "Saved: "
                 << outputFile
                 << endl;
        }
        else
        {
            cerr << "ERROR: Could not save: "
                 << outputFile
                 << endl;
        }
    }

    // ------------------------------------------------------------
    // 17. COMPLETE
    // ------------------------------------------------------------

    cout << endl;
    cout << "========================================"
         << endl;
    cout << "STAGE 2 COMPLETE"
         << endl;
    cout << "========================================"
         << endl;

    return 0;
}