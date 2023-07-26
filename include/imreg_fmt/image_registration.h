/**
 * Copyright (c) Santosh Thoduka
 *
 * This code is licensed under the GPLv3 license, which can be found in the root directory.
 * Original repo: https://github.com/sthoduka/imreg_fmt
 */
#ifndef IMAGE_REGISTRATION_H
#define IMAGE_REGISTRATION_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <imreg_fmt/image_dft.h>
#include <imreg_fmt/image_transforms.h>

class ImageRegistration
{
    public:
        ImageRegistration(const cv::Mat &im);
        virtual ~ImageRegistration();
        void initialize(const cv::Mat &im);
        /*
         * convert to grayscale
         * change range of pixel intensity to 0-1
         * smooth borders
         * get log-polar transform of DFT
         */
        void processImage(const cv::Mat &im, cv::Mat &gray, cv::Mat &log_polar);
        /*
         * register im to previous image (from previous call to the same function or initialize function)
         */
        void registerImage(const cv::Mat &im, cv::Mat &out, std::vector<double> &transform_params, bool display_images = false);
        /*
         * return white image warped by same amount as last registration operation
         */
        cv::Mat getBorderMask();
        /*
         * prepare for next image by copying reference image
         *
         * this is not done in registerImage since the current and previous image
         * might be required after registration.
         */
        void next();

        cv::Mat getPreviousImage();
        cv::Mat getCurrentImage();

    protected:
        int rows_;
        int cols_;
        int log_polar_size_;

        ImageDFT imdft_;
        ImageDFT imdft_logpolar_;
        ImageTransforms image_transforms_;

        Eigen::MatrixXd high_pass_filter_;

        cv::Mat im0_gray_;
        cv::Mat im1_gray_;
        cv::Mat im0_logpolar_;
        cv::Mat im1_logpolar_;
        cv::Mat im0_rotated_;

};

#endif /* IMAGE_REGISTRATION_H */
