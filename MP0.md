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

### MP.7

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