/**
 * Copyright (c) Santosh Thoduka
 *
 * This code is licensed under the GPLv3 license, which can be found in the root directory.
 * Original repo: https://github.com/sthoduka/imreg_fmt
 */
#ifndef IMAGE_DFT_H_
#define IMAGE_DFT_H_

#include <iostream>
#include <complex>
#include <fftw3.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <eigen3/Eigen/Dense>

typedef Eigen::Matrix<std::complex<double>, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> ComplexMatrix;
class ImageDFT
{
    public:
        ImageDFT(int rows, int cols);
        virtual ~ImageDFT();

        /*
         * calculate DFT of image
         */
        ComplexMatrix fft(const cv::Mat &im);
        /*
         * calculate DFT of matrix
         */
        ComplexMatrix fft(const ComplexMatrix &in);
        /*
         * calculate inverse DFT of matrix
         */
        ComplexMatrix ifft(const ComplexMatrix &in);

        /*
         * swap 1st and 3rd quadrant, and 2nd and 4th quadrant
         */
        ComplexMatrix fftShift(const ComplexMatrix &in);

        /*
         * cross power spectrum of two fourier transforms
         */
        ComplexMatrix crossPowerSpectrum(const ComplexMatrix &f1, const ComplexMatrix &f2);

        /*
         * find translation in x and y between two images
         */
        void phaseCorrelate(const cv::Mat &im0, const cv::Mat &im1, double &row, double &col);

        /*
         * centre of mass centered at (row,col)
         */
        void getCentreOfMass(const Eigen::MatrixXd &f1, double &row, double &col);

        Eigen::VectorXd getFftFreqs(int size);
        Eigen::MatrixXd getHighPassFilter();
        void plot(const Eigen::MatrixXd &in);

    protected:
        Eigen::MatrixXd neighbourhood(const Eigen::MatrixXd &f1, int centre_row, int centre_col, int radius);

    protected:
        int cols_;
        int rows_;
        fftw_plan forward_plan_;
        fftw_plan backward_plan_;
        ComplexMatrix in_;
        ComplexMatrix out_;
        ComplexMatrix iin_;
        ComplexMatrix iout_;
};

#endif
