#include <fstream>
#include <sstream>
#include <iostream>

#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "objectDetection2D.hpp"

using namespace std;

// detects objects in an image using the YOLO library and a set of pre-trained objects from the COCO database;
// a set of 80 classes is listed in "coco.names" and pre-trained weights are stored in "yolov3.weights"

void detectObjects(cv::Mat& img,
                   std::vector<BoundingBox>& bBoxes,
                   float confThreshold,
                   float nmsThreshold,
                   std::string basePath,
                   std::string classesFile,
                   std::string modelConfiguration,
                   std::string modelWeights,
                   bool bVis)
{
    // Load COCO class names
    std::vector<std::string> classes;
    std::ifstream ifs(classesFile);
    std::string line;

    while (std::getline(ifs, line))
        classes.push_back(line);

    if (!ifs.eof() && ifs.fail())
        throw std::runtime_error("Failed to read COCO class names");

    // modelConfiguration now points to the ONNX model.
    // modelWeights is retained to preserve the existing function signature.
    (void)basePath;
    (void)modelWeights;

    cv::dnn::Net net = cv::dnn::readNetFromONNX(modelConfiguration);
    net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
    net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);

    // The converted YOLOv3 model expects a 608 x 608 input.
    constexpr int inputSize = 608;

    cv::Mat blob;
    cv::dnn::blobFromImage(
        img, blob, 1.0 / 255.0,
        cv::Size(inputSize, inputSize),
        cv::Scalar(0, 0, 0),
        false, false
    );

    net.setInput(blob);
    cv::Mat output = net.forward();

    // Expected output: [1, 22743, 85]
    // Each row: cx, cy, width, height, objectness, 80 class scores.
    if (output.dims != 3 || output.size[2] != 85)
        throw std::runtime_error("Unexpected YOLOv3 ONNX output shape");

    const int numPredictions = output.size[1];
    const int numValues = output.size[2];

    cv::Mat predictions(
        numPredictions,
        numValues,
        CV_32F,
        output.ptr<float>()
    );

    std::vector<int> classIds;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;

    const cv::Rect imageBounds(0, 0, img.cols, img.rows);

    for (int i = 0; i < numPredictions; ++i)
    {
        const float* data = predictions.ptr<float>(i);

        const float objectness = data[4];

        if (objectness <= 0.0f)
            continue;

        // Find the class with the highest class probability.
        int classId = 0;
        float classScore = data[5];

        for (int c = 1; c < 80; ++c)
        {
            if (data[5 + c] > classScore)
            {
                classScore = data[5 + c];
                classId = c;
            }
        }

        // Standard YOLO confidence = objectness * class probability.
        const float confidence = objectness * classScore;

        if (confidence < confThreshold)
            continue;

        // Output coordinates are in 608 x 608 model-input space.
        // Map them back to the original image dimensions.
        const float scaleX = static_cast<float>(img.cols) / inputSize;
        const float scaleY = static_cast<float>(img.rows) / inputSize;

        const float cx = data[0] * scaleX;
        const float cy = data[1] * scaleY;
        const float width = data[2] * scaleX;
        const float height = data[3] * scaleY;

        const int left = static_cast<int>(cx - width / 2.0f);
        const int top = static_cast<int>(cy - height / 2.0f);
        const int boxWidth = static_cast<int>(width);
        const int boxHeight = static_cast<int>(height);

        cv::Rect box(left, top, boxWidth, boxHeight);
        box &= imageBounds;

        if (box.width <= 0 || box.height <= 0)
            continue;

        boxes.push_back(box);
        classIds.push_back(classId);
        confidences.push_back(confidence);
    }

    // Preserve the original pipeline's NMS behavior.
    std::vector<int> indices;
    cv::dnn::NMSBoxes(
        boxes, confidences,
        confThreshold, nmsThreshold, indices
    );

    for (int index : indices)
    {
        BoundingBox bBox;
        bBox.roi = boxes[index];
        bBox.classID = classIds[index];
        bBox.confidence = confidences[index];
        bBox.boxID = static_cast<int>(bBoxes.size());

        bBoxes.push_back(bBox);
    }

    // Optional visualization
    if (bVis)
    {
        cv::Mat visImg = img.clone();

        for (const auto& box : bBoxes)
        {
            cv::rectangle(
                visImg, box.roi,
                cv::Scalar(0, 255, 0), 2
            );

            std::string label;
            if (box.classID >= 0 &&
                box.classID < static_cast<int>(classes.size()))
            {
                label = classes[box.classID];
            }
            else
            {
                label = "unknown";
            }

            label += ":" + cv::format("%.2f", box.confidence);

            cv::putText(
                visImg, label,
                cv::Point(box.roi.x, std::max(15, box.roi.y - 5)),
                cv::FONT_HERSHEY_SIMPLEX, 0.5,
                cv::Scalar(0, 255, 0), 1
            );
        }

        cv::imshow("Object classification", visImg);
        cv::waitKey(0);
    }
}