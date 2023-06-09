/* INCLUDES FOR THIS PROJECT */
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <limits>
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/xfeatures2d/nonfree.hpp>

#include "dataStructures.h"
#include "matching2D.hpp"

using namespace std;

/* MAIN PROGRAM */
int main(int argc, const char *argv[])
{

    /* INIT VARIABLES AND DATA STRUCTURES */

    // data location
    string dataPath = "../";

    // camera
    string imgBasePath = dataPath + "images/";
    string imgPrefix = "KITTI/2011_09_26/image_00/data/000000"; // left camera, color
    string imgFileType = ".png";
    int imgStartIndex = 0; // first file index to load (assumes Lidar and camera names have identical naming convention)
    int imgEndIndex = 9;   // last file index to load
    int imgFillWidth = 4;  // no. of digits which make up the file index (e.g. img-0001.png)

    // misc
    int dataBufferSize = 2;       // no. of images which are held in memory (ring buffer) at the same time
    vector<DataFrame> dataBuffer; // list of data frames which are held in memory at the same time
    bool bVis = false;            // visualize results

    /* MAIN LOOP OVER ALL IMAGES */
    std::vector<string>detectorTypes = {"SIFT"}; //{"SHITOMASI" , "FAST", "BRISK", "ORB", "AKAZE", "SIFT"};
    string descriptorType = "ORB"; // BRIEF, ORB, FREAK, AKAZE, SIFT

    for (string detectorType: detectorTypes)
    {
        vector<cv::KeyPoint> keypoints; // create empty feature list for current image
        vector<cv::DMatch> matches;

        std::vector<double> keypointSizes;
        std::vector<size_t> numKeypoints;
        std::vector<size_t> numMatches;

        double detection_time = 0.0;
        double description_time = 0.0;

        for (size_t imgIndex = 0; imgIndex <= imgEndIndex - imgStartIndex; imgIndex++)
        {
            /* LOAD IMAGE INTO BUFFER */

            // assemble filenames for current index
            ostringstream imgNumber;
            imgNumber << setfill('0') << setw(imgFillWidth) << imgStartIndex + imgIndex;
            string imgFullFilename = imgBasePath + imgPrefix + imgNumber.str() + imgFileType;

            // load image from file and convert to grayscale
            cv::Mat img, imgGray;
            img = cv::imread(imgFullFilename);
            cv::cvtColor(img, imgGray, cv::COLOR_BGR2GRAY);

            //// STUDENT ASSIGNMENT
            //// TASK MP.1 -> replace the following code with ring buffer of size dataBufferSize

            // push image into data frame buffer
            DataFrame frame;
            if (dataBuffer.size() == dataBufferSize)
            {
                dataBuffer.erase(dataBuffer.begin());
                frame.cameraImg = imgGray;
                dataBuffer.push_back(frame);
            }
            else
            {
                frame.cameraImg = imgGray;
                dataBuffer.push_back(frame);
            }


            //// EOF STUDENT ASSIGNMENT
            cout << "#1 : LOAD IMAGE INTO BUFFER done" << endl;

            /* DETECT IMAGE KEYPOINTS */

            // extract 2D keypoints from current image
            keypoints.clear();
            //// STUDENT ASSIGNMENT
            //// TASK MP.2 -> add the following keypoint detectors in file matching2D.cpp and enable string-based selection based on detectorType
            //// -> HARRIS, FAST, BRISK, ORB, AKAZE, SIFT

            if (detectorType.compare("SHITOMASI") == 0)
            {
                detection_time += detKeypointsShiTomasi(keypoints, imgGray, false);
            }
            else if (detectorType.compare("HARRIS") == 0)
            {
                detection_time += detKeypointsHarris(keypoints, imgGray, false);
            }
            else
            {
                detection_time += detKeypointsModern(keypoints, imgGray, detectorType, false);
            }
            //// EOF STUDENT ASSIGNMENT

            //// STUDENT ASSIGNMENT
            //// TASK MP.3 -> only keep keypoints on the preceding vehicle

            // only keep keypoints on the preceding vehicle
            bool bFocusOnVehicle = true;
            cv::Rect vehicleRect(535, 180, 180, 150);
            if (bFocusOnVehicle)
            {
            std::vector<cv::KeyPoint> tempPoints;
            for(auto &point : keypoints)
            {
                if(vehicleRect.contains(point.pt))
                {
                    tempPoints.push_back(point);
                }
            }

            keypoints = tempPoints;
            }

            // log keypoint sizes to vector for computing neighborhood distribution later
            for(const auto &keypoint: keypoints)
            {
                keypointSizes.push_back(keypoint.size);
            }
            numKeypoints.push_back(keypoints.size());

            //// EOF STUDENT ASSIGNMENT

            // optional : limit number of keypoints (helpful for debugging and learning)
            bool bLimitKpts = false;
            if (bLimitKpts)
            {
                int maxKeypoints = 50;

                if (detectorType.compare("SHITOMASI") == 0)
                { // there is no response info, so keep the first 50 as they are sorted in descending quality order
                    keypoints.erase(keypoints.begin() + maxKeypoints, keypoints.end());
                }
                cv::KeyPointsFilter::retainBest(keypoints, maxKeypoints);
                cout << " NOTE: Keypoints have been limited!" << endl;
            }

            // push keypoints and descriptor for current frame to end of data buffer
            (dataBuffer.end() - 1)->keypoints = keypoints;
            cout << "#2 : DETECT KEYPOINTS done" << endl;

            /* EXTRACT KEYPOINT DESCRIPTORS */

            //// STUDENT ASSIGNMENT
            //// TASK MP.4 -> add the following descriptors in file matching2D.cpp and enable string-based selection based on descriptorType
            //// -> BRIEF, ORB, FREAK, AKAZE, SIFT

            cv::Mat descriptors;
            description_time += descKeypoints((dataBuffer.end() - 1)->keypoints, (dataBuffer.end() - 1)->cameraImg, descriptors, descriptorType);
            //// EOF STUDENT ASSIGNMENT

            // push descriptors for current frame to end of data buffer
            (dataBuffer.end() - 1)->descriptors = descriptors;

            cout << "#3 : EXTRACT DESCRIPTORS done" << endl;

            if (dataBuffer.size() > 1) // wait until at least two images have been processed
            {

                /* MATCH KEYPOINT DESCRIPTORS */
                matches.clear();
                string matcherType = "MAT_BF";        // MAT_BF, MAT_FLANN

                string descriptorType = descriptorType.compare("SIFT") == 0  ? "DES_HOG" : "DES_BINARY"; // DES_BINARY, DES_HOG
                string selectorType = "SEL_KNN";       // SEL_NN, SEL_KNN

                auto descSource = (dataBuffer.end() - 2)->descriptors;
                auto descRef = (dataBuffer.end() - 1)->descriptors;
                if (detectorType.compare("SIFT") == 0 && descriptorType.compare("SIFT") == 0)
                {
                    descSource.convertTo(descSource, CV_8U);
                    descRef.convertTo(descRef, CV_8U); 
                }

                //// STUDENT ASSIGNMENT
                //// TASK MP.5 -> add FLANN matching in file matching2D.cpp
                //// TASK MP.6 -> add KNN match selection and perform descriptor distance ratio filtering with t=0.8 in file matching2D.cpp

                matchDescriptors((dataBuffer.end() - 2)->keypoints, (dataBuffer.end() - 1)->keypoints,
                                descSource, descRef,
                                matches, descriptorType, matcherType, selectorType);
                // Log the number of matches
                numMatches.push_back(matches.size());
                //// EOF STUDENT ASSIGNMENT

                // store matches in current data frame
                (dataBuffer.end() - 1)->kptMatches = matches;

                cout << "#4 : MATCH KEYPOINT DESCRIPTORS done" << endl;

                // visualize matches between current and previous image
                bVis = false;
                if (bVis)
                {
                    cv::Mat matchImg = ((dataBuffer.end() - 1)->cameraImg).clone();
                    cv::drawMatches((dataBuffer.end() - 2)->cameraImg, (dataBuffer.end() - 2)->keypoints,
                                    (dataBuffer.end() - 1)->cameraImg, (dataBuffer.end() - 1)->keypoints,
                                    matches, matchImg,
                                    cv::Scalar::all(-1), cv::Scalar::all(-1),
                                    vector<char>(), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

                    string windowName = "Matching keypoints between two camera images";
                    cv::namedWindow(windowName, 7);
                    cv::imshow(windowName, matchImg);
                    cout << "Press key to continue to next image" << endl;
                    cv::waitKey(0); // wait for key to be pressed
                }
                bVis = false;
            }

        } // eof loop over all images

        // Compute the average times for detection and description
        detection_time /= 10;
        description_time /= 10;

        // compute keypoint neighborhood size distribution
        double neighborhood_mean = std::accumulate(keypointSizes.begin(), keypointSizes.end(), 0.0) / keypointSizes.size();
        double neighborhood_variance = 0.0;
        for(const auto keypoint_size: keypointSizes)
        {
            neighborhood_variance += (keypoint_size - neighborhood_mean) * (keypoint_size - neighborhood_mean);
        }
        neighborhood_variance /= keypointSizes.size();

        std::stringstream logData;
        logData << detectorType << ", " << descriptorType << ", ";
        logData << detection_time << ", " << description_time << ", ";
        logData <<neighborhood_mean<<", "<< neighborhood_variance << ", ";
        logData << "[ ";
        for (const size_t nKpts: numKeypoints)
        {
            logData << nKpts<< " ";
        } 
        logData << "], ";
        logData << "[ ";
        for (const size_t nMatches: numMatches)
        {
            logData << nMatches<< " ";
        }
        logData << "]" << std::endl;
        std::cout << logData.str();
    }

    return 0;
}
