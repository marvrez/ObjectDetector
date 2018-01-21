#ifndef OBJECT_H
#define  OBJECT_H

#include <string>

#include <opencv/cv.h>
#include <opencv/highgui.h>

class Object {
public:
    Object() : type_("Object"), color_(0,0,0) { }
    explicit Object(std::string name);

    ~Object() = default;

    //getters
    int             getXPos() const { return x_pos_; }
    int             getYPos() const { return y_pos_; }
    cv::Scalar      getHSVmin() const { return hsv_min_; }
    cv::Scalar      getHSVmax() const { return hsv_max_; }
    std::string     getType() const { return type_; }
    cv::Scalar      getColor() const { return color_; }

    //setters
    void setXPos(int x) { x_pos_ = x; }
    void setYPos(int y) { y_pos_ = y; }
    void setHSVmin(cv::Scalar min) { hsv_min_ = min; }
    void setHSVmax(cv::Scalar max) { hsv_max_ = max; }
    void setType(std::string t) { type_ = t; }
    void setColor(cv::Scalar c) { color_ = c; }
private:
    int x_pos_, y_pos_;
    std::string type_;
    cv::Scalar hsv_min_, hsv_max_;
    cv::Scalar color_;
};

#endif
