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

    double x;
    double y;
    double z;

    double distance;
};


// ================================================================
// PROJECT LiDAR POINT ONTO CAMERA IMAGE
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


    if (
        !std::isfinite(depth) ||
        depth <= 0.0
    )
    {
        return false;
    }


    double u =
        Y.at<double>(0,0) /
        depth;

    double v =
        Y.at<double>(1,0) /
        depth;


    if (
        !std::isfinite(u) ||
        !std::isfinite(v)
    )
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
        cv::Point(
            px,
            py
        );


    return true;
}


// ================================================================
// VEHICLE CLASS NAME
// ================================================================

string getVehicleClassName(
    int classID)
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
// ORIGINAL LiDAR FILTER
//
// Same limits used in the original project.
// ================================================================

bool isInsideLidarRegion(
    const LidarPoint &point)
{
    float minZ = -1.5;
    float maxZ = -0.9;

    float minX = 2.0;
    float maxX = 20.0;

    float maxY = 2.0;

    float minR = 0.1;


    double distanceXY =
        sqrt(
            point.x * point.x +
            point.y * point.y
        );


    if (point.z < minZ)
        return false;

    if (point.z > maxZ)
        return false;

    if (point.x < minX)
        return false;

    if (point.x > maxX)
        return false;

    if (point.y > maxY)
        return false;

    if (distanceXY < minR)
        return false;


    return true;
}


// ================================================================
// INTERSECTION OVER UNION
//
// Used to determine which YOLO vehicle detection corresponds
// to the original preceding-vehicle ROI.
// ================================================================

double computeIoU(
    const cv::Rect &a,
    const cv::Rect &b)
{
    int x1 =
        max(
            a.x,
            b.x
        );

    int y1 =
        max(
            a.y,
            b.y
        );

    int x2 =
        min(
            a.x + a.width,
            b.x + b.width
        );

    int y2 =
        min(
            a.y + a.height,
            b.y + b.height
        );


    int intersectionWidth =
        max(
            0,
            x2 - x1
        );

    int intersectionHeight =
        max(
            0,
            y2 - y1
        );


    double intersectionArea =
        static_cast<double>(
            intersectionWidth *
            intersectionHeight
        );


    double areaA =
        static_cast<double>(
            a.area()
        );

    double areaB =
        static_cast<double>(
            b.area()
        );


    double unionArea =
        areaA +
        areaB -
        intersectionArea;


    if (unionArea <= 0.0)
    {
        return 0.0;
    }


    return (
        intersectionArea /
        unionArea
    );
}


// ================================================================
// MAIN
// ================================================================

int main()
{
    // ============================================================
    // STAGE 5
    // FINAL PRECEDING VEHICLE VISUALIZATION
    // ============================================================

    string dataPath =
        "../";


    // ============================================================
    // CAMERA DATA
    // ============================================================

    string imgBasePath =
        dataPath + "images/";

    string imgPrefix =
        "KITTI/2011_09_26/image_02/data/000000";


    // ============================================================
    // LiDAR DATA
    // ============================================================

    string lidarPrefix =
        "KITTI/2011_09_26/velodyne_points/data/000000";


    // ============================================================
    // OUTPUT
    // ============================================================

    string outputPath =
        "../results/stages/05_target_vehicle/frames/";


    // ============================================================
    // YOLO
    // ============================================================

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
    // ORIGINAL PRECEDING VEHICLE ROI
    //
    // This is the same ROI used in the original project when
    // focusing keypoints on the preceding vehicle.
    // ============================================================

    cv::Rect vehicleRect(
        535,
        180,
        180,
        150
    );


    // ============================================================
    // CALIBRATION MATRICES
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


    // ============================================================
    // RT
    // ============================================================

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


    // ============================================================
    // R_RECT_00
    // ============================================================

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


    // ============================================================
    // P_RECT_00
    // ============================================================

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
    cout << " STAGE 5: PRECEDING VEHICLE" << endl;
    cout << "==============================================" << endl;

    cout << "Using original preceding-vehicle ROI: "
         << vehicleRect
         << endl;

    cout << endl;


    // ============================================================
    // FRAME LOOP
    // ============================================================

    for (
        int imgIndex = imgStartIndex;
        imgIndex <= imgEndIndex;
        imgIndex += imgStepWidth
    )
    {
        // ========================================================
        // FRAME NUMBER
        // ========================================================

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
        // LOAD LiDAR
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
        // ORIGINAL LiDAR FILTER
        // ========================================================

        vector<LidarPoint>
            filteredLidarPoints;


        for (
            const auto &point :
            lidarPoints
        )
        {
            if (
                isInsideLidarRegion(
                    point
                )
            )
            {
                filteredLidarPoints.push_back(
                    point
                );
            }
        }


        // ========================================================
        // PROJECT FILTERED LiDAR POINTS
        // ========================================================

        vector<ProjectedLidarPoint>
            projectedPoints;


        projectedPoints.reserve(
            filteredLidarPoints.size()
        );


        for (
            const auto &point :
            filteredLidarPoints
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


            ProjectedLidarPoint projected;


            projected.pixel =
                pixel;


            projected.x =
                point.x;


            projected.y =
                point.y;


            projected.z =
                point.z;


            projected.distance =
                sqrt(
                    point.x * point.x +
                    point.y * point.y +
                    point.z * point.z
                );


            projectedPoints.push_back(
                projected
            );
        }


        // ========================================================
        // FIND THE YOLO DETECTION CORRESPONDING TO THE
        // ORIGINAL PRECEDING-VEHICLE ROI
        // ========================================================

        int targetVehicleIndex =
            -1;


        double bestIoU =
            0.0;


        for (
            size_t i = 0;
            i < boundingBoxes.size();
            i++
        )
        {
            BoundingBox &box =
                boundingBoxes[i];


            // Only vehicle classes
            if (
                box.classID != 2 &&
                box.classID != 3 &&
                box.classID != 5 &&
                box.classID != 7
            )
            {
                continue;
            }


            double iou =
                computeIoU(
                    box.roi,
                    vehicleRect
                );


            if (
                iou > bestIoU
            )
            {
                bestIoU =
                    iou;

                targetVehicleIndex =
                    static_cast<int>(
                        i
                    );
            }
        }


        // ========================================================
        // FINAL VISUALIZATION
        //
        // Only the preceding vehicle and its LiDAR points
        // are shown.
        // ========================================================

        cv::Mat visImg =
            cameraImg.clone();


        // ========================================================
        // TARGET VEHICLE FOUND
        // ========================================================

        if (
            targetVehicleIndex >= 0
        )
        {
            BoundingBox &target =
                boundingBoxes[
                    targetVehicleIndex
                ];


            // ----------------------------------------------------
            // Associate projected LiDAR points with target ROI
            // ----------------------------------------------------

            cv::Rect targetROI =
                target.roi;


            // Same 10% shrink concept as original association
            double shrinkFactor =
                0.10;


            int shrinkX =
                static_cast<int>(
                    targetROI.width *
                    shrinkFactor
                );


            int shrinkY =
                static_cast<int>(
                    targetROI.height *
                    shrinkFactor
                );


            cv::Rect shrunkROI(
                targetROI.x + shrinkX,
                targetROI.y + shrinkY,
                targetROI.width -
                    2 * shrinkX,
                targetROI.height -
                    2 * shrinkY
            );


            vector<ProjectedLidarPoint>
                targetLidarPoints;


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
                    targetLidarPoints.push_back(
                        point
                    );
                }
            }


            // ----------------------------------------------------
            // Draw target LiDAR points
            //
            // Green, like the original project output.
            // ----------------------------------------------------

            for (
                const auto &point :
                targetLidarPoints
            )
            {
                cv::circle(
                    visImg,
                    point.pixel,
                    4,
                    cv::Scalar(
                        0,
                        255,
                        0
                    ),
                    -1
                );
            }


            // ----------------------------------------------------
            // Draw target bounding box
            //
            // Green, matching original output.
            // ----------------------------------------------------

            cv::rectangle(
                visImg,
                targetROI,
                cv::Scalar(
                    0,
                    255,
                    0
                ),
                3
            );


            // ----------------------------------------------------
            // Target label
            // ----------------------------------------------------

            string label =
                "PRECEDING VEHICLE";


            cv::putText(
                visImg,
                label,
                cv::Point(
                    targetROI.x,
                    max(
                        25,
                        targetROI.y - 10
                    )
                ),
                cv::FONT_HERSHEY_SIMPLEX,
                0.65,
                cv::Scalar(
                    0,
                    255,
                    0
                ),
                2
            );


            // ----------------------------------------------------
            // LiDAR point count
            // ----------------------------------------------------

            string lidarLabel =
                "LiDAR points: " +
                to_string(
                    targetLidarPoints.size()
                );


            cv::putText(
                visImg,
                lidarLabel,
                cv::Point(
                    targetROI.x,
                    targetROI.y +
                    targetROI.height +
                    25
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
        }


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

        if (
            targetVehicleIndex >= 0
        )
        {
            cout
                << "Frame "
                << setw(2)
                << imgIndex
                << " | Target: "
                << getVehicleClassName(
                    boundingBoxes[
                        targetVehicleIndex
                    ].classID
                )
                << " | IoU: "
                << fixed
                << setprecision(3)
                << bestIoU
                << " | Saved"
                << endl;
        }
        else
        {
            cout
                << "Frame "
                << setw(2)
                << imgIndex
                << " | Target: NONE"
                << " | Saved"
                << endl;
        }
    }


    // ============================================================
    // COMPLETE
    // ============================================================

    cout << endl;

    cout << "==============================================" << endl;
    cout << " STAGE 5 COMPLETE" << endl;
    cout << "==============================================" << endl;

    return 0;
}