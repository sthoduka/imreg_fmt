#include <imreg_fmt/image_transforms.h>
#include <math.h>
#include <iostream>
#include <sstream>
#include <opencv2/core/eigen.hpp>

ImageTransforms::ImageTransforms(int rows, int cols, int logPolarrows, int logPolarcols) :
                                rows_(rows), cols_(cols), logPolarrows_(logPolarrows), logPolarcols_(logPolarcols),
                                cv_xMap(logPolarrows, logPolarcols, CV_32FC1), cv_yMap(logPolarrows, logPolarcols, CV_32FC1),
                                xMap(cv_xMap.ptr<float>(), cv_xMap.rows, cv_xMap.cols),
                                yMap(cv_yMap.ptr<float>(), cv_yMap.rows, cv_yMap.cols)
{
    createLogPolarMap();
    border_mask_ = cv::Mat(rows_, cols_, CV_8UC1, cv::Scalar(255));

    Eigen::MatrixXd win = getApodizationWindow(rows_, cols_, (int)((0.12)*std::min(rows_, cols_)));
    cv::eigen2cv(win, appodizationWindow);
    appodizationWindow.convertTo(appodizationWindow, CV_32F);
}

ImageTransforms::~ImageTransforms()
{

}

void ImageTransforms::createLogPolarMap()
{
    logBase_ = std::exp(std::log(rows_ * 1.1 / 2.0) / std::max(rows_, cols_));
    Eigen::VectorXf scales(logPolarcols_);
    float ellipse_coefficient = (float)(rows_) / cols_;
    for (int i = 0; i < logPolarcols_; i++)
    {
        scales(i) = std::pow(logBase_, i);
    }
    Eigen::VectorXf ones_rows = Eigen::VectorXf::Ones(logPolarrows_);
    Eigen::MatrixXf scales_matrix = ones_rows * scales.transpose();// identical rows, each col is logBase^i

    Eigen::VectorXf angles = Eigen::VectorXf::LinSpaced(logPolarrows_, 0.0, M_PI);
    angles *= -1.0;
    Eigen::MatrixXf angles_matrix = angles * Eigen::VectorXf::Ones(logPolarcols_).transpose();// identical columns, each row is linspace 0-pi

    float centre[2] = {rows_/2.0f, cols_/2.0f};
    EigenRowMatrix cos = angles_matrix.array().cos() / ellipse_coefficient;
    EigenRowMatrix sin = angles_matrix.array().sin();
    xMap = scales_matrix.cwiseProduct(cos).array() + centre[1];
    yMap = scales_matrix.cwiseProduct(sin).array() + centre[0];
}


void ImageTransforms::remapLogPolar(const cv::Mat &src, cv::Mat &dst)
{
    cv::remap(src, dst, cv_xMap, cv_yMap, CV_INTER_CUBIC & cv::INTER_MAX, cv::BORDER_CONSTANT, cv::Scalar());
}

void ImageTransforms::getScaleRotation(double row, double col, double &scale, double &rotation)
{
    rotation = -M_PI * row  / cv_xMap.rows;
    rotation = rotation * 180.0 / M_PI;
    rotation += 360.0 / 2.0;
    rotation = fmod(rotation, 360.0);
    rotation -=  360.0 / 2.0;
    rotation = -rotation;
    scale = std::pow(logBase_, col);
    scale = 1.0 / scale;
}

void ImageTransforms::rotateAndScale(const cv::Mat &src, cv::Mat &dst, double scale, double angle)
{
    cv::Mat warped_image;
    cv::Mat rotationMatrix = cv::getRotationMatrix2D(cv::Point(src.cols/2, src.rows/2), angle, scale);
    cv::warpAffine(src, dst, rotationMatrix, src.size());
    cv::warpAffine(border_mask_, rotated_mask_, rotationMatrix, src.size());
}

void ImageTransforms::translate(const cv::Mat &src, cv::Mat &dst, double xTrans, double yTrans)
{
    cv::Mat warped_image;
    cv::Mat translateMatrix = (cv::Mat_<float>(2, 3) << 1.0, 0.0, xTrans, 0.0, 1.0, yTrans);
    cv::warpAffine(src, dst, translateMatrix, src.size());
    cv::warpAffine(rotated_mask_, rotated_translated_mask_, translateMatrix, src.size());
}

cv::Mat ImageTransforms::getBorderMask()
{
    return rotated_translated_mask_;
}

void ImageTransforms::apodize(const cv::Mat &in, cv::Mat &out)
{
    out = in.mul(appodizationWindow);
}

Eigen::VectorXd ImageTransforms::getHanningWindow(int size)
{
    Eigen::VectorXd window(size);
    for (int i = 0; i < size; i++)
    {
        window(i) = 0.5 - 0.5 * std::cos((2 * M_PI * i)/(size - 1));
    }
    return window;
}

Eigen::MatrixXd ImageTransforms::getApodizationWindow(int rows, int cols, int radius)
{
    Eigen::VectorXd hanning_window = getHanningWindow(radius * 2);

    Eigen::VectorXd row = Eigen::VectorXd::Ones(cols);
    row.segment(0, radius) = hanning_window.segment(0, radius);
    row.segment(cols - radius, radius) = hanning_window.segment(radius, radius);

    Eigen::VectorXd col = Eigen::VectorXd::Ones(rows);
    col.segment(0, radius) = hanning_window.segment(0, radius);
    col.segment(rows - radius, radius) = hanning_window.segment(radius, radius);

    return col * row.transpose();
}

