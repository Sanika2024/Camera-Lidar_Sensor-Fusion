#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#include "dataStructures.h"
#include "objectDetection2D.hpp"

using namespace std;

int main()
{
    /* ============================================================
       STAGE 1 : YOLO ALL VEHICLE DETECTIONS
       ============================================================ */

    // ------------------------------------------------------------
    // 1. PATHS
    // ------------------------------------------------------------

    string dataPath = "./";

    string imgBasePath = dataPath + "images/";
    string imgPrefix =
        "KITTI/2011_09_26/image_02/data/000000";
    string imgFileType = ".png";

    int imgStartIndex = 0;
    int imgEndIndex = 18;
    int imgFillWidth = 4;

    // YOLO files
    string yoloBasePath = dataPath + "dat/yolo/";
    string yoloClassesFile =
        yoloBasePath + "coco.names";
    string yoloModelConfiguration =
        yoloBasePath + "yolov3.cfg";
    string yoloModelWeights =
        yoloBasePath + "yolov3.weights";

    // Output folder
    string outputPath =
        "./results/stages/01_yolo_all_vehicles/frames/";

    // ------------------------------------------------------------
    // 2. YOLO PARAMETERS
    // ------------------------------------------------------------

    float confThreshold = 0.2;
    float nmsThreshold = 0.4;

    bool bVis = false;

    // ------------------------------------------------------------
    // 3. PROCESS EACH CAMERA IMAGE
    // ------------------------------------------------------------

    for (int imgIndex = imgStartIndex;
         imgIndex <= imgEndIndex;
         imgIndex++)
    {
        // Create image number: 000000, 000001, ...
        ostringstream imgNumber;
        imgNumber << setfill('0')
                  << setw(imgFillWidth)
                  << imgIndex;

        // Build image filename
        string imgFullFilename =
            imgBasePath +
            imgPrefix +
            imgNumber.str() +
            imgFileType;

        cout << endl;
        cout << "========================================"
             << endl;
        cout << "Processing frame: "
             << imgNumber.str()
             << endl;

        // --------------------------------------------------------
        // Load image
        // --------------------------------------------------------

        cv::Mat img = cv::imread(imgFullFilename);

        if (img.empty())
        {
            cerr << "ERROR: Could not load image: "
                 << imgFullFilename
                 << endl;

            continue;
        }

        cout << "Image loaded successfully."
             << endl;

        // --------------------------------------------------------
        // YOLO OBJECT DETECTION
        // --------------------------------------------------------

        vector<BoundingBox> boundingBoxes;

        detectObjects(
            img,
            boundingBoxes,
            confThreshold,
            nmsThreshold,
            yoloBasePath,
            yoloClassesFile,
            yoloModelConfiguration,
            yoloModelWeights,
            bVis
        );

        cout << "Total YOLO detections: "
             << boundingBoxes.size()
             << endl;

        // --------------------------------------------------------
        // DRAW ALL DETECTIONS
        // --------------------------------------------------------

        cv::Mat stage1Img = img.clone();

        for (const auto &box : boundingBoxes)
        {
            // Draw bounding box
            cv::rectangle(
                stage1Img,
                box.roi,
                cv::Scalar(0, 255, 0),
                2
            );

            // Prepare label
            string className;
            switch (box.classID)
            {
                case 2:
                    className = "car";
                    break;

                case 3:
                    className = "motorcycle";
                    break;

                case 5:
                    className = "bus";
                    break;

                case 7:
                    className = "truck";
                    break;

                default:
                    className = "object";
                    break;
            }

            string label =
                className +
                "  Conf: " +
                to_string(box.confidence).substr(0, 4);

            // Text size
            int baseline = 0;

            cv::Size labelSize =
                cv::getTextSize(
                    label,
                    cv::FONT_HERSHEY_SIMPLEX,
                    0.5,
                    1,
                    &baseline
                );

            int labelX = box.roi.x;
            int labelY =
                max(
                    box.roi.y - 5,
                    labelSize.height + 5
                );

            // Label background
            cv::rectangle(
                stage1Img,
                cv::Point(
                    labelX,
                    labelY - labelSize.height - 5
                ),
                cv::Point(
                    labelX + labelSize.width,
                    labelY + baseline - 5
                ),
                cv::Scalar(0, 255, 0),
                cv::FILLED
            );

            // Label text
            cv::putText(
                stage1Img,
                label,
                cv::Point(
                    labelX,
                    labelY - 7
                ),
                cv::FONT_HERSHEY_SIMPLEX,
                0.5,
                cv::Scalar(0, 0, 0),
                1
            );
        }

        // --------------------------------------------------------
        // SAVE OUTPUT
        // --------------------------------------------------------

        string outputFile =
            outputPath +
            "frame_" +
            imgNumber.str() +
            ".png";

        bool saved =
            cv::imwrite(
                outputFile,
                stage1Img
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

    cout << endl;
    cout << "========================================"
         << endl;
    cout << "STAGE 1 COMPLETE"
         << endl;
    cout << "========================================"
         << endl;

    return 0;
}