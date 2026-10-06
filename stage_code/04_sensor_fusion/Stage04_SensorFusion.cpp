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
#include "objectDetection2D.hpp"
#include "dataStructures.h"

using namespace std;
using namespace cv;


// ================================================================
// PROJECTED LiDAR POINT
// ================================================================

struct ProjectedLidarPoint
{
    cv::Point pixel;
    double distance;
};


// ================================================================
// PROJECT ONE LiDAR POINT ONTO CAMERA IMAGE
//
// Same transformation used by the original project:
//
// Y = P_rect_00 * R_rect_00 * RT * X
// ================================================================

bool projectLidarPoint(
    const LidarPoint &point,
    const cv::Mat &P_rect_00,
    const cv::Mat &R_rect_00,
    const cv::Mat &RT,
    const cv::Size &imageSize,
    cv::Point &pixel)
{
    cv::Mat X(
        4,
        1,
        cv::DataType<double>::type
    );

    X.at<double>(0,0) = point.x;
    X.at<double>(1,0) = point.y;
    X.at<double>(2,0) = point.z;
    X.at<double>(3,0) = 1.0;

    // LiDAR -> camera projection
    cv::Mat Y =
        P_rect_00 *
        R_rect_00 *
        RT *
        X;

    double depth =
        Y.at<double>(2,0);

    // Point must be in front of camera
    if (!std::isfinite(depth) ||
        depth <= 0.0)
    {
        return false;
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
        return false;
    }

    int px =
        static_cast<int>(
            std::round(u)
        );

    int py =
        static_cast<int>(
            std::round(v)
        );

    // Point must lie inside camera image
    if (
        px < 0 ||
        px >= imageSize.width ||
        py < 0 ||
        py >= imageSize.height
    )
    {
        return false;
    }

    pixel =
        cv::Point(px, py);

    return true;
}


// ================================================================
// GET READABLE VEHICLE CLASS
// ================================================================

string getVehicleClassName(int classID)
{
    switch (classID)
    {
        case 2:
            return "car";

        case 3:
            return "motorcycle";

        case 5:
            return "bus";

        case 7:
            return "truck";

        default:
            return "vehicle";
    }
}


// ================================================================
// MAIN
// ================================================================

int main()
{
    // ============================================================
    // STAGE 4
    // CAMERA + LiDAR SENSOR FUSION
    // ============================================================

    // Program is executed from build/
    string dataPath = "../";

    // ------------------------------------------------------------
    // Camera
    // ------------------------------------------------------------

    string imgBasePath =
        dataPath + "images/";

    string imgPrefix =
        "KITTI/2011_09_26/image_02/data/000000";

    // ------------------------------------------------------------
    // LiDAR
    // ------------------------------------------------------------

    string lidarPrefix =
        "KITTI/2011_09_26/velodyne_points/data/000000";

    // ------------------------------------------------------------
    // Output
    // ------------------------------------------------------------

    string outputPath =
        "../results/stages/04_sensor_fusion/frames/";

    // ------------------------------------------------------------
    // YOLO
    // ------------------------------------------------------------

    string yoloBasePath =
        dataPath + "dat/yolo/";

    string yoloClassesFile =
        yoloBasePath + "coco.names";

    string yoloModelConfiguration =
        yoloBasePath + "yolov3.cfg";

    string yoloModelWeights =
        yoloBasePath + "yolov3.weights";


    // ============================================================
    // FRAME SETTINGS
    // ============================================================

    int imgStartIndex = 0;
    int imgEndIndex = 18;
    int imgStepWidth = 1;
    int imgFillWidth = 4;


    // ============================================================
    // CAMERA-LiDAR CALIBRATION
    // Same calibration as original project
    // ============================================================

    cv::Mat P_rect_00(
        3,
        4,
        cv::DataType<double>::type
    );

    cv::Mat R_rect_00(
        4,
        4,
        cv::DataType<double>::type
    );

    cv::Mat RT(
        4,
        4,
        cv::DataType<double>::type
    );


    // ------------------------------------------------------------
    // RT
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
    // R_rect_00
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
    // P_rect_00
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
    cout << " STAGE 4: CAMERA + LiDAR SENSOR FUSION" << endl;
    cout << "==============================================" << endl;

    cout << "Frames: "
         << imgStartIndex
         << " - "
         << imgEndIndex
         << endl;

    cout << endl;

    cout << "YOLO:" << endl;
    cout << "  Confidence threshold: 0.20" << endl;
    cout << "  NMS threshold: 0.40" << endl;

    cout << endl;

    cout << "Sensor fusion:" << endl;
    cout << "  1. Detect vehicles using camera" << endl;
    cout << "  2. Load raw LiDAR points" << endl;
    cout << "  3. Project LiDAR onto camera image" << endl;
    cout << "  4. Associate LiDAR points with each vehicle ROI" << endl;

    cout << endl;

    cout << "NO target-vehicle filtering in Stage 4." << endl;


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


        // ========================================================
        // LOAD CAMERA IMAGE
        // ========================================================

        string imgFullFilename =
            imgBasePath +
            imgPrefix +
            imgNumber.str() +
            ".png";

        cv::Mat cameraImg =
            cv::imread(
                imgFullFilename
            );

        if (cameraImg.empty())
        {
            cerr
                << "ERROR: Could not load image: "
                << imgFullFilename
                << endl;

            continue;
        }


        // ========================================================
        // YOLO DETECTION
        //
        // IMPORTANT:
        // This matches the actual detectObjects() API
        // in this repository.
        // ========================================================

        vector<BoundingBox> boundingBoxes;

        detectObjects(
            cameraImg,
            boundingBoxes,
            0.20,
            0.40,
            yoloBasePath,
            yoloClassesFile,
            yoloModelConfiguration,
            yoloModelWeights,
            false
        );


        // ========================================================
        // LOAD RAW LiDAR
        // ========================================================

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


        // ========================================================
        // PROJECT ALL LiDAR POINTS
        // ========================================================

        vector<ProjectedLidarPoint>
            projectedPoints;

        projectedPoints.reserve(
            lidarPoints.size()
        );


        for (
            const auto &point :
            lidarPoints
        )
        {
            cv::Point pixel;

            bool valid =
                projectLidarPoint(
                    point,
                    P_rect_00,
                    R_rect_00,
                    RT,
                    cameraImg.size(),
                    pixel
                );

            if (!valid)
            {
                continue;
            }

            double distance =
                sqrt(
                    point.x * point.x +
                    point.y * point.y +
                    point.z * point.z
                );

            ProjectedLidarPoint projected;

            projected.pixel =
                pixel;

            projected.distance =
                distance;

            projectedPoints.push_back(
                projected
            );
        }


        // ========================================================
        // CREATE VISUALIZATION
        // ========================================================

        cv::Mat visImg =
            cameraImg.clone();


        // ========================================================
        // DRAW ALL PROJECTED LiDAR POINTS
        //
        // These are the points that Stage 3 produced.
        // ========================================================

        for (
            const auto &point :
            projectedPoints
        )
        {
            double normalized =
                point.distance / 40.0;

            normalized =
                max(
                    0.0,
                    min(
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

            cv::Scalar color(
                blue,
                120,
                red
            );

            cv::circle(
                visImg,
                point.pixel,
                2,
                color,
                -1
            );
        }


        // ========================================================
        // SENSOR FUSION
        //
        // Associate projected LiDAR points with EACH YOLO
        // vehicle bounding box.
        // ========================================================

        int fusedVehicleCount = 0;


        for (
            size_t boxIndex = 0;
            boxIndex < boundingBoxes.size();
            boxIndex++
        )
        {
            BoundingBox &box =
                boundingBoxes[boxIndex];


            // ----------------------------------------------------
            // Only visualize vehicle classes
            // ----------------------------------------------------

            if (
                box.classID != 2 &&
                box.classID != 3 &&
                box.classID != 5 &&
                box.classID != 7
            )
            {
                continue;
            }


            // ----------------------------------------------------
            // Original YOLO ROI
            // ----------------------------------------------------

            cv::Rect roi =
                box.roi;


            // ----------------------------------------------------
            // Shrink ROI by 10%
            //
            // Same idea used in original project to reduce
            // association with points near ROI boundaries.
            // ----------------------------------------------------

            double shrinkFactor =
                0.10;

            int shrinkX =
                static_cast<int>(
                    roi.width *
                    shrinkFactor
                );

            int shrinkY =
                static_cast<int>(
                    roi.height *
                    shrinkFactor
                );

            int shrunkWidth =
                roi.width -
                2 * shrinkX;

            int shrunkHeight =
                roi.height -
                2 * shrinkY;


            // Safety check
            if (
                shrunkWidth <= 0 ||
                shrunkHeight <= 0
            )
            {
                continue;
            }


            cv::Rect shrunkROI(
                roi.x + shrinkX,
                roi.y + shrinkY,
                shrunkWidth,
                shrunkHeight
            );


            // ----------------------------------------------------
            // Find LiDAR points inside this vehicle ROI
            // ----------------------------------------------------

            vector<ProjectedLidarPoint>
                associatedPoints;


            for (
                const auto &point :
                projectedPoints
            )
            {
                if (
                    shrunkROI.contains(
                        point.pixel
                    )
                )
                {
                    associatedPoints.push_back(
                        point
                    );
                }
            }


            // ----------------------------------------------------
            // Draw YOLO bounding box
            // ----------------------------------------------------

            cv::rectangle(
                visImg,
                roi,
                cv::Scalar(
                    0,
                    255,
                    0
                ),
                2
            );


            // ----------------------------------------------------
            // Draw associated LiDAR points
            // ----------------------------------------------------

            for (
                const auto &point :
                associatedPoints
            )
            {
                cv::circle(
                    visImg,
                    point.pixel,
                    4,
                    cv::Scalar(
                        0,
                        0,
                        255
                    ),
                    -1
                );
            }


            // ----------------------------------------------------
            // Vehicle class
            // ----------------------------------------------------

            string className =
                getVehicleClassName(
                    box.classID
                );


            // ----------------------------------------------------
            // Label
            // ----------------------------------------------------

            string label =
                className +
                " | LiDAR: " +
                to_string(
                    associatedPoints.size()
                );


            int labelY =
                max(
                    20,
                    roi.y - 8
                );


            cv::putText(
                visImg,
                label,
                cv::Point(
                    roi.x,
                    labelY
                ),
                cv::FONT_HERSHEY_SIMPLEX,
                0.55,
                cv::Scalar(
                    0,
                    255,
                    0
                ),
                2
            );


            if (
                !associatedPoints.empty()
            )
            {
                fusedVehicleCount++;
            }
        }


        // ========================================================
        // INFORMATION PANEL
        // ========================================================

        string frameText =
            "STAGE 4 | SENSOR FUSION | Frame: " +
            to_string(imgIndex);

        string yoloText =
            "YOLO vehicle detections: " +
            to_string(boundingBoxes.size());

        string lidarText =
            "Raw LiDAR points: " +
            to_string(lidarPoints.size());

        string projectedText =
            "Projected LiDAR points: " +
            to_string(projectedPoints.size());

        string fusedText =
            "Vehicles with LiDAR association: " +
            to_string(fusedVehicleCount);

        string legendText =
            "Green = YOLO ROI | Red = associated LiDAR";


        cv::putText(
            visImg,
            frameText,
            cv::Point(20, 35),
            cv::FONT_HERSHEY_SIMPLEX,
            0.65,
            cv::Scalar(
                255,
                255,
                255
            ),
            2
        );

        cv::putText(
            visImg,
            yoloText,
            cv::Point(20, 65),
            cv::FONT_HERSHEY_SIMPLEX,
            0.55,
            cv::Scalar(
                255,
                255,
                255
            ),
            2
        );

        cv::putText(
            visImg,
            lidarText,
            cv::Point(20, 95),
            cv::FONT_HERSHEY_SIMPLEX,
            0.55,
            cv::Scalar(
                255,
                255,
                255
            ),
            2
        );

        cv::putText(
            visImg,
            projectedText,
            cv::Point(20, 125),
            cv::FONT_HERSHEY_SIMPLEX,
            0.55,
            cv::Scalar(
                255,
                255,
                255
            ),
            2
        );

        cv::putText(
            visImg,
            fusedText,
            cv::Point(20, 155),
            cv::FONT_HERSHEY_SIMPLEX,
            0.55,
            cv::Scalar(
                255,
                255,
                255
            ),
            2
        );

        cv::putText(
            visImg,
            legendText,
            cv::Point(20, 185),
            cv::FONT_HERSHEY_SIMPLEX,
            0.50,
            cv::Scalar(
                255,
                255,
                255
            ),
            2
        );


        // ========================================================
        // SAVE FRAME
        // ========================================================

        string outputFilename =
            outputPath +
            "frame_" +
            imgNumber.str() +
            ".png";

        cv::imwrite(
            outputFilename,
            visImg
        );


        // ========================================================
        // TERMINAL OUTPUT
        // ========================================================

        cout
            << "Frame "
            << setw(2)
            << imgIndex
            << " | YOLO vehicles: "
            << setw(2)
            << boundingBoxes.size()
            << " | Raw LiDAR: "
            << setw(7)
            << lidarPoints.size()
            << " | Projected: "
            << setw(7)
            << projectedPoints.size()
            << " | Fused vehicles: "
            << setw(2)
            << fusedVehicleCount
            << " | Saved"
            << endl;
    }


    cout << endl;

    cout << "==============================================" << endl;
    cout << " STAGE 4 COMPLETE" << endl;
    cout << "==============================================" << endl;

    return 0;
}