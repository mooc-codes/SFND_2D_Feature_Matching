This document addresses the first point in the rubric 
>Provide a Writeup / README that includes all the rubric points and how you addressed each one. You can submit your writeup as markdown or pdf.


### MP.1
> Implement a vector for dataBuffer objects whose size does not exceed a limit (e.g. 2 elements). This can be achieved by pushing in new elements on one end and removing elements on the other end.

```C++
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
```

If the data buffer is full, the first (oldest) frame is removed and new frame is inserted, otherwise the incoming new frame is insterted without removing any older frames.

### MP.2
> Implement detectors HARRIS, FAST, BRISK, ORB, AKAZE, and SIFT and make them selectable by setting a string accordingly.

```C++

    cv::Ptr<cv::FeatureDetector> detector;

    if (detectorType.compare("FAST") == 0) 
    {
        detector = cv::FastFeatureDetector::create();
    }
    else if (detectorType.compare("BRISK") == 0)
    {
        detector = cv::BRISK::create();
    }
    else if (detectorType.compare("ORB") == 0)
    {
        detector = cv::ORB::create();
    }
    else if (detectorType.compare("SIFT") == 0)
    {
        detector = cv::xfeatures2d::SIFT::create();
    }
    else if (detectorType.compare("AKAZE") == 0)
    {
        detector = cv::AKAZE::create();
    }

```
The modern keypoint detectors are implemented as above and the HARRIS detector is implemented in `detKeypointsHarris()`

### MP.3

>Remove all keypoints outside of a pre-defined rectangle and only use the keypoints within the rectangle for further processing.

```C++
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
```
The coordinates of the keypoints is checked against the `cv::Rect` and only the points that lie inside the rectanle are retained.


### MP.4

> Implement descriptors BRIEF, ORB, FREAK, AKAZE and SIFT and make them selectable by setting a string accordingly.

```C++
    cv::Ptr<cv::DescriptorExtractor> extractor;
    if (descriptorType.compare("BRISK") == 0)
    {

        int threshold = 30;        // FAST/AGAST detection threshold score.
        int octaves = 3;           // detection octaves (use 0 to do single scale)
        float patternScale = 1.0f; // apply this scale to the pattern used for sampling the neighbourhood of a keypoint.

        extractor = cv::BRISK::create(threshold, octaves, patternScale);
    }
    else if (descriptorType.compare("ORB") == 0)
    {
        extractor = cv::ORB::create();
    }
    else if (descriptorType.compare("BRIEF") == 0)
    {
        extractor = cv::xfeatures2d::BriefDescriptorExtractor::create();
    }
    else if (descriptorType.compare("FREAK") == 0)
    {
        extractor = cv::xfeatures2d::FREAK::create();
    }
    else if (descriptorType.compare("SIFT") == 0)
    {
        extractor = cv::xfeatures2d::SIFT::create();
    }
    else if (descriptorType.compare("AKAZE") == 0)
    {
        extractor = cv::AKAZE::create();
    }
```

### MP.5
> Implement FLANN matching as well as k-nearest neighbor selection. Both methods must be selectable using the respective strings in the main function.

```C++

    else if (matcherType.compare("MAT_FLANN") == 0)
    {
        descSource.convertTo(descSource, CV_32F);
        descRef.convertTo(descRef, CV_32F);
        matcher = cv::FlannBasedMatcher::create(); 
    }
```
The images need to be converted since FLANN uses a float type.

### MP.6
> Use the K-Nearest-Neighbor matching to implement the descriptor distance ratio test, which looks at the ratio of best vs. second-best match to decide whether to keep an associated pair of keypoints.

```C++
    std::vector<std::vector<cv::DMatch>> knnMatches;
    matcher->knnMatch(descSource, descRef, knnMatches, 2);


    float ratioThreshold = 0.8f;  // Ratio threshold for filtering good matches

    for (size_t i = 0; i < knnMatches.size(); ++i)
    {
        if (knnMatches[i][0].distance < ratioThreshold * knnMatches[i][1].distance) 
        {
            matches.push_back(knnMatches[i][0]);
        }
    }
```

### MP.7 and MP.8

> Count the number of keypoints on the preceding vehicle for all 10 images and take note of the distribution of their neighborhood size. Do this for all the detectors you have implemented.

```C++
    // compute keypoint neighborhood size distribution
    double neighborhood_mean = std::accumulate(keypointSizes.begin(), keypointSizes.end(), 0.0) / keypointSizes.size();
    double neighborhood_variance = 0.0;
    for(const auto keypoint_size: keypointSizes)
    {
        neighborhood_variance += (keypoint_size - neighborhood_mean) * (keypoint_size - neighborhood_mean);
    }
    neighborhood_variance /= keypointSizes.size();
    
```

To understand the distribution of neighborhood sizes, we store the sizes of keypoints in bounded area from all 10 images and compute the mean and variance of the size distribution.

The number of keypoints per image, the number of matches and the distribution of neighborhood sizes are as following

```
SHI-TOMASI -> neighborhood size distribution (4, 0)
Keypoints : [ 125 118 123 120 120 113 114 123 111 112 ]

FAST -> neighborhood size distribution (7, 0)
Keypoints : [ 419 427 404 423 386 414 418 406 396 401 ]

BRISK -> neighborhood size distribution (21.9444, 213.688)
Keypoints : [ 264 282 282 277 297 279 289 272 266 254 ]

ORB -> neighborhood size distribution (55.9928, 630.76)
Keypoints : [ 92 102 106 113 109 125 130 129 127 128 ]

AKAZE -> neighborhood size distribution (7.69915, 12.5121)
Keypoints : [ 166 157 161 155 163 164 173 175 177 179 ]

SIFT -> neighborhood size distribution (5.03738, 35.8244)
Keypoints : [ 138 132 124 137 134 140 137 148 159 137 ]
```