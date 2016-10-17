#ifndef IMAGE_TRANSFORMS_H_
#define IMAGE_TRANSFORMS_H_

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <eigen3/Eigen/Dense>

typedef Eigen::Map<Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> > EigenMap;
typedef Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> EigenRowMatrix;

class ImageTransforms
{
    public:
        ImageTransforms(int rows, int cols, int logPolarrows, int logPolarcols);
        virtual ~ImageTransforms();
        void createLogPolarMap();
        void remapLogPolar(const cv::Mat &src, cv::Mat &dst);
        /*
         * Convert a row/col on IDFT of CPS of log-polar images into a scale and rotation
         */
        void getScaleRotation(double row, double col, double &scale, double &rotation);
        /*
         * Rotate and scale image
         */
        void rotateAndScale(const cv::Mat &src, cv::Mat &dst, double scale, double angle);
        /*
         * translate image
         */
        void translate(const cv::Mat &src, cv::Mat &dst, double xTrans, double yTrans);
        /*
         * smooth borders
         */
        void apodize(const cv::Mat &in, cv::Mat &out);
        cv::Mat getBorderMask();

    protected:
        Eigen::VectorXd getHanningWindow(int size);
        Eigen::MatrixXd getApodizationWindow(int rows, int cols, int radius);

    protected:
        int rows_;
        int cols_;
        int logPolarrows_;
        int logPolarcols_;
        cv::Mat cv_xMap;
        cv::Mat cv_yMap;
        EigenMap xMap;
        EigenMap yMap;
        double logBase_;
        cv::Mat border_mask_;
        cv::Mat rotated_mask_;
        cv::Mat rotated_translated_mask_;
        cv::Mat appodizationWindow;
};

#endif /* IMAGE_TRANSFORMS_H */
