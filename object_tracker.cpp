#include <sstream>
#include <string>
#include <iostream>
#include <vector>

#include "object.h"

/////////////// GLOBAL VALUES /////////////////////////

//initial min and max HSV filter values.
//these will be changed using trackbars
int H_MIN = 0;
int H_MAX = 256;
int S_MIN = 0;
int S_MAX = 256;
int V_MIN = 0;
int V_MAX = 256;
//default capture width and height
constexpr int FRAME_WIDTH = 640;
constexpr int FRAME_HEIGHT = 480;
//max number of objects to be detected in frame
constexpr int MAX_NUM_OBJECTS = 50;
//minimum and maximum object area
constexpr int MIN_OBJECT_AREA = 20*20;
constexpr int MAX_OBJECT_AREA = FRAME_HEIGHT*FRAME_WIDTH/1.5;

//For canny edge detection
cv::Mat dst, detected_edges;
cv::Mat src, src_gray;
int edge_thresh = 1;
int low_thresh;
int max_low_thresh = 100;
int ratio = 3;
int kernel_size = 3;

const std::string window_name = "Edge Map";
const std::string windowName         = "Original Image";
const std::string windowName1        = "HSV Image";
const std::string windowName2        = "Thresholded Image";
const std::string windowName3        = "After Morphological Operations";
const std::string trackbarWindowName = "Trackbars";

//////////// END GLOBAL VALUES ////////////////////////


//This function gets called whenever a trackbar position is changed
void on_trackbar( int, void* ) {

}

void createTrackbars() { 
    cv::namedWindow(trackbarWindowName, 0);
    //create memory to store trackbar name on window
    char TrackbarName[50];
    sprintf(TrackbarName, "H_MIN", H_MIN);
    sprintf(TrackbarName, "H_MAX", H_MAX);
    sprintf(TrackbarName, "S_MIN", S_MIN);
    sprintf(TrackbarName, "S_MAX", S_MAX);
    sprintf(TrackbarName, "V_MIN", V_MIN);
    sprintf(TrackbarName, "V_MAX", V_MAX);
    //create trackbars and insert them into window
    cv::createTrackbar("H_MIN", trackbarWindowName, &H_MIN, H_MAX, on_trackbar);
    cv::createTrackbar("H_MAX", trackbarWindowName, &H_MAX, H_MAX, on_trackbar);
    cv::createTrackbar("S_MIN", trackbarWindowName, &S_MIN, S_MAX, on_trackbar);
    cv::createTrackbar("S_MAX", trackbarWindowName, &S_MAX, S_MAX, on_trackbar);
    cv::createTrackbar("V_MIN", trackbarWindowName, &V_MIN, V_MAX, on_trackbar);
    cv::createTrackbar("V_MAX", trackbarWindowName, &V_MAX, V_MAX, on_trackbar);
}

void drawObject(std::vector<Object> objects , cv::Mat &frame, cv::Mat &temp, 
                std::vector<std::vector<cv::Point> > contours, 
                std::vector<cv::Vec4i> hierarchy) 
{
	for(int i = 0; i < objects.size(); ++i){
        drawContours(frame, contours, i, objects.at(i).getColor(), 3, 8, hierarchy);
        circle(frame, cv::Point(objects.at(i).getXPos(), objects.at(i).getYPos()), 5, objects.at(i).getColor());

        putText(frame, std::to_string(objects.at(i).getXPos()) + " , " + std::to_string(objects.at(i).getYPos()),
                cv::Point(objects.at(i).getXPos(), objects.at(i).getYPos() + 20),
                1, 1, objects.at(i).getColor());

        putText(frame, objects.at(i).getType(), cv::Point(objects.at(i).getXPos(), objects.at(i).getYPos()-20), 1, 2, objects.at(i).getColor());
	}
}

void drawObject(std::vector<Object> objects, cv::Mat &frame){
    for(int i = 0; i< objects.size(); ++i) {
        cv::circle(frame,cv::Point(objects.at(i).getXPos(), objects.at(i).getYPos()), 10, cv::Scalar(0,0,255));

        cv::putText(frame, std::to_string(objects.at(i).getXPos()) + " , " + std::to_string(objects.at(i).getYPos()),
                    cv::Point(objects.at(i).getXPos(), objects.at(i).getYPos() + 20),
                    1, 1, cv::Scalar(0,255,0));

        cv::putText(frame, objects.at(i).getType(), cv::Point(objects.at(i).getXPos(),
                    objects.at(i).getYPos() - 30),
                    1, 2, objects.at(i).getColor());
    }
}

void morphOps(cv::Mat &thresh) {
    //create structuring element that will be used to "dilate" and "erode" image.
    //the element chosen here is a 3px by 3px rectangle
    cv::Mat erode_element = getStructuringElement(cv::MORPH_RECT, cv::Size(3,3));
    //dilate with larger element so make sure object is nicely visible
    cv::Mat dilate_element = getStructuringElement(cv::MORPH_RECT, cv::Size(8,8));

    erode(thresh, thresh, erode_element);
    erode(thresh, thresh, erode_element);

    dilate(thresh, thresh, dilate_element);
    dilate(thresh, thresh, dilate_element);
}

void trackFilteredObject(cv::Mat threshold, cv::Mat HSV, cv::Mat& camera_feed) {
    std::vector<Object> objects;
    cv::Mat temp;
    threshold.copyTo(temp);
    //these two vectors needed for output of findContours
    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;
    //find contours of filtered image using openCV findContours function
    findContours(temp,contours,hierarchy,CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE );
    //use moments method to find our filtered object
    double refArea = 0;
    bool objectFound = false;
    if (hierarchy.size() > 0) {
        int numObjects = hierarchy.size();
        //if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
		if(numObjects < MAX_NUM_OBJECTS) {
			for (int index = 0; index >= 0; index = hierarchy[index][0]) {
                cv::Moments moment = moments((cv::Mat)contours[index]);
                double area = moment.m00;
                //if the area is less than 20 px by 20px then it is probably just noise
                //if the area is the same as the 3/2 of the image size, probably just a bad filter
                //we only want the object with the largest area so we safe a reference area each
                //iteration and compare it to the area in the next iteration.
                if(area > MIN_OBJECT_AREA) {
                    Object object;

                    object.setXPos(moment.m10/area);
                    object.setYPos(moment.m01/area);

                    objects.push_back(object);

                    objectFound = true;
				}
                else objectFound = false;
			}
			//let user know you found an object
            if(objectFound) {
                drawObject(objects, camera_feed);
            }
        }
        else putText(camera_feed,"TOO MUCH NOISE! ADJUST FILTER", cv::Point(0,50), 1, 2, cv::Scalar(0,0,255), 2);
    }
}

void trackFilteredObject(Object theObject, cv::Mat threshold, cv::Mat HSV, cv::Mat& camera_feed){
    std::vector<Object> objects;
    cv::Mat temp;
    threshold.copyTo(temp);
    //these two vectors needed for output of findContours
    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;
    //find contours of filtered image using openCV findContours function
    findContours(temp, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
    //use moments method to find our filtered object
    double refArea = 0;
    bool objectFound = false;
    if (hierarchy.size() > 0) {
        int numObjects = hierarchy.size();
        //if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
        if(numObjects < MAX_NUM_OBJECTS){
            for (int index = 0; index >= 0; index = hierarchy[index][0]) {
                cv::Moments moment = moments((cv::Mat)contours[index]);
                double area = moment.m00;
                //if the area is less than 20 px by 20px then it is probably just noise
                //if the area is the same as the 3/2 of the image size, probably just a bad filter
                //we only want the object with the largest area so we safe a reference area each
                //iteration and compare it to the area in the next iteration.
                if(area > MIN_OBJECT_AREA) {
                    Object object;

                    object.setXPos(moment.m10/area);
                    object.setYPos(moment.m01/area);
                    object.setType(theObject.getType());
                    object.setColor(theObject.getColor());

                    objects.push_back(object);

                    objectFound = true;
                }
                else objectFound = false;
            }
            if(objectFound) {
                drawObject(objects, camera_feed, temp, contours, hierarchy);
            }
        }
        else cv::putText(camera_feed, "Too much noise! Adjust filter", cv::Point(0,50), 1, 2, cv::Scalar(0,0,255),2);
    }
}

int main(int argc, char* argv[]) {
    //if we would like to calibrate our filter values, set to true.
    bool calibration_mode = false;

    cv::Mat camera_feed, threshold, hsv;

    //create slider bars for HSV filtering
    if(calibration_mode) createTrackbars();

    cv::VideoCapture cap(0);
    cv::waitKey(1000);

    while (true) {
        cap >> camera_feed;

        src = camera_feed;
        if (!src.data) { 
            std::cout << "Could not capture frame from camera, exiting program." << std::endl;
            return -1; 
        }

        //convert frame from BGR to HSV colorspace
        cvtColor(camera_feed, hsv, cv::COLOR_BGR2HSV);

        if (calibration_mode) { 
            //need to find the appropriate color range values
            // calibration_mode must be false

            //if in calibration mode, we track objects based on the HSV slider values.
            cvtColor(camera_feed, hsv, cv::COLOR_BGR2HSV);
            inRange(hsv, cv::Scalar(H_MIN,S_MIN,V_MIN), cv::Scalar(H_MAX,S_MAX,V_MAX), threshold);
            morphOps(threshold);
            cv::imshow(windowName2, threshold);

            //Canny edge detecton
            dst.create(src.size(), src.type());
            // Convert the image to grayscale
            cvtColor(src, src_gray, CV_BGR2GRAY);
            // Create a window
            cv::namedWindow(window_name.c_str(), CV_WINDOW_AUTOSIZE);
            // Create a Trackbar for user to enter threshold
            cv::createTrackbar("Min Threshold:", window_name.c_str(), &low_thresh, max_low_thresh);
            // Show the image
            trackFilteredObject(threshold, hsv, camera_feed);
		}
        else {
            //create some temp objects so that
            //we can use their member functions/information
            Object blue("blue"), yellow("yellow"), red("red"), green("green");

            //first find blue objects
            cv::cvtColor(camera_feed, hsv, cv::COLOR_BGR2HSV);
            inRange(hsv, blue.getHSVmin(), blue.getHSVmax(), threshold);
            morphOps(threshold);
            trackFilteredObject(blue,threshold, hsv, camera_feed);
            //then yellows
            cvtColor( camera_feed, hsv, cv::COLOR_BGR2HSV);
            inRange(hsv, yellow.getHSVmin(), yellow.getHSVmax(), threshold);
            morphOps(threshold);
            trackFilteredObject(yellow,threshold, hsv, camera_feed);
            //then reds
            cvtColor(camera_feed, hsv, cv::COLOR_BGR2HSV);
            inRange(hsv, red.getHSVmin(), red.getHSVmax(), threshold);
            morphOps(threshold);
            trackFilteredObject(red,threshold, hsv, camera_feed);
            //then greens
            cvtColor(camera_feed, hsv, cv::COLOR_BGR2HSV);
            inRange(hsv, green.getHSVmin(), green.getHSVmax(), threshold);
            morphOps(threshold);
            trackFilteredObject(green, threshold, hsv, camera_feed);
        }
        //imshow(windowName2,threshold);
        imshow(windowName, camera_feed);
        //imshow(windowName1, hsv);

        cv::waitKey(30);
    }
    return 0;
}
