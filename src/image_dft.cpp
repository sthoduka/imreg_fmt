#include <imreg_fmt/image_dft.h>
#include <math.h>

ImageDFT::ImageDFT(int rows, int cols)
{
    rows_ = rows;
    cols_ = cols;
    in_.resize(rows_, cols_);
    out_.resize(rows_, cols_);
    iin_.resize(rows_, cols_);
    iout_.resize(rows_, cols_);
    forward_plan_ = fftw_plan_dft_2d(rows_, cols_, const_cast<fftw_complex*>(reinterpret_cast<fftw_complex*>(in_.data())),
                                           const_cast<fftw_complex*>(reinterpret_cast<fftw_complex*>(out_.data())),
                                           FFTW_FORWARD, FFTW_MEASURE);
    backward_plan_ = fftw_plan_dft_2d(rows_, cols_, const_cast<fftw_complex*>(reinterpret_cast<fftw_complex*>(iin_.data())),
                                           const_cast<fftw_complex*>(reinterpret_cast<fftw_complex*>(iout_.data())),
                                           FFTW_BACKWARD, FFTW_MEASURE);

}

ImageDFT::~ImageDFT()
{
    fftw_destroy_plan(forward_plan_);
    fftw_destroy_plan(backward_plan_);
}

ComplexMatrix ImageDFT::fft(const cv::Mat &im)
{
    ComplexMatrix in(im.rows, im.cols);
    // TODO: find a better way to copy the data over
    for (int i = 0; i < im.rows; i++)
    {
       for (int j = 0; j < im.cols; j++)
       {
           in(i, j) = std::complex<double>(im.at<float>(i,j), 0.0);
       }
    }
    return fft(in);
}

ComplexMatrix ImageDFT::fft(const ComplexMatrix &in)
{
    in_ = in;
    fftw_execute(forward_plan_);
    return out_;
}

ComplexMatrix ImageDFT::ifft(const ComplexMatrix &in)
{
    iin_ = in;
    fftw_execute(backward_plan_);
    // output is scaled by size
    iout_ = iout_ / (iout_.size());
    return iout_;
}
//
// TODO: use templating so just one function is needed
// TODO: check if row/col size is even or odd
ComplexMatrix ImageDFT::fftShift(const ComplexMatrix &in)
{
    ComplexMatrix out(in.rows(), in.cols());
    int block_rows = in.rows()/2;
    int block_cols = in.cols()/2;
    // swap first and third quadrant
    out.block(0, 0, block_rows, block_cols) = in.block(block_rows, block_cols, block_rows, block_cols);
    out.block(block_rows, block_cols, block_rows, block_cols) = in.block(0, 0, block_rows, block_cols);
    // swap second and fourth quadrant
    out.block(block_rows, 0, block_rows, block_cols) = in.block(0, block_cols, block_rows, block_cols);
    out.block(0, block_cols, block_rows, block_cols) = in.block(block_rows, 0, block_rows, block_cols);
    return out;
}

Eigen::MatrixXd ImageDFT::fftShift(const Eigen::MatrixXd &in)
{
    Eigen::MatrixXd out(in.rows(), in.cols());
    int block_rows = in.rows()/2;
    int block_cols = in.cols()/2;
    // swap first and third quadrant
    out.block(0, 0, block_rows, block_cols) = in.block(block_rows, block_cols, block_rows, block_cols);
    out.block(block_rows, block_cols, block_rows, block_cols) = in.block(0, 0, block_rows, block_cols);
    // swap second and fourth quadrant
    out.block(block_rows, 0, block_rows, block_cols) = in.block(0, block_cols, block_rows, block_cols);
    out.block(0, block_cols, block_rows, block_cols) = in.block(block_rows, 0, block_rows, block_cols);
    return out;
}

Eigen::VectorXd ImageDFT::fftShift(const Eigen::VectorXd &in)
{
    // [1, 2, 3, 4, 5] -> [4, 5, 1, 2, 3]
    // [1, 2, 3, 4, 5, 6] -> [4, 5, 6, 1, 2, 3]
    Eigen::VectorXd out(in.size());
    int block_size = in.size() / 2;
    int first_block_size = block_size;
    int second_start_point = block_size;
    if (in.size() % 2 != 0)
    {
        first_block_size++;
        second_start_point++;
    }
    // swap first half and second half
    out.segment(block_size, first_block_size) = in.segment(0, first_block_size);
    out.segment(0, block_size) = in.segment(first_block_size, block_size);
    return out;
}

Eigen::VectorXd ImageDFT::ifftShift(const Eigen::VectorXd &in)
{
    // [4, 5, 1, 2, 3] -> [1, 2, 3, 4, 5]
    // [4, 5, 6, 1, 2, 3] -> [1, 2, 3, 4, 5, 6]
    Eigen::VectorXd out(in.size());
    int block_size = in.size() / 2;
    int second_block_size = block_size;
    int first_start_point = block_size;
    if (in.size() % 2 != 0)
    {
        second_block_size++;
        first_start_point++;
    }
    // swap first half and second half
    out.segment(first_start_point, block_size) = in.segment(0, block_size);
    out.segment(0, second_block_size) = in.segment(block_size, second_block_size);
    return out;
}

ComplexMatrix ImageDFT::crossPowerSpectrum(const ComplexMatrix &f1, const ComplexMatrix &f2)
{
    // (f1 x f2*) / (abs(f1) x abs(f2))
    double eps = 1e-15; // add eps in case denominator is zero
    Eigen::MatrixXd denom = f1.cwiseAbs().cwiseProduct(f2.cwiseAbs()).array() + eps;
    ComplexMatrix out = (f1.cwiseProduct(f2.conjugate())).cwiseQuotient(denom);
    return out;
}

void ImageDFT::phaseCorrelate(const cv::Mat &im0, const cv::Mat &im1, double &row, double &col)
{
    ComplexMatrix m0 = fft(im0);
    ComplexMatrix m1 = fft(im1);
    ComplexMatrix cross_power = crossPowerSpectrum(m0, m1);
    ComplexMatrix inversefft = ifft(cross_power);
    ComplexMatrix shifted_cps = fftShift(inversefft);
    Eigen::MatrixXd abs = shifted_cps.cwiseAbs();

    int approx_row, approx_col;
    abs.maxCoeff(&approx_row, &approx_col); // argmax

    int radius = 2;
    Eigen::MatrixXd subArray = neighbourhood(abs, approx_row, approx_col, radius);
    getCentreOfMass(subArray, row, col);

    row = approx_row + (row - radius);
    col = approx_col + (col - radius);

    row = row - (shifted_cps.rows() / 2);
    col = col - (shifted_cps.cols() / 2);
}

void ImageDFT::getCentreOfMass(const Eigen::MatrixXd &f1, double &row, double &col)
{
    Eigen::VectorXd cols = Eigen::VectorXd::LinSpaced(f1.cols(), 0, f1.cols() - 1);
    Eigen::VectorXd rows = Eigen::VectorXd::LinSpaced(f1.rows(), 0, f1.rows() - 1);

    Eigen::MatrixXd colMat = Eigen::VectorXd::Ones(f1.rows()) * cols.transpose();
    Eigen::MatrixXd rowMat = rows * Eigen::VectorXd::Ones(f1.cols()).transpose();

    double f1sum = f1.sum();
    col = colMat.cwiseProduct(f1).sum() / f1sum;
    row = rowMat.cwiseProduct(f1).sum() / f1sum;
}

Eigen::MatrixXd ImageDFT::getHighPassFilter()
{
    Eigen::VectorXd yy = Eigen::VectorXd::LinSpaced(rows_, -M_PI / 2.0, M_PI / 2.0);
    Eigen::VectorXd yy_vec = Eigen::VectorXd::Ones(cols_);
    Eigen::MatrixXd yy_matrix = yy * yy_vec.transpose(); // identical cols, each row is linspace

    Eigen::VectorXd xx = Eigen::VectorXd::LinSpaced(cols_, -M_PI / 2.0, M_PI / 2.0);
    Eigen::VectorXd xx_vec = Eigen::VectorXd::Ones(rows_);
    Eigen::MatrixXd xx_matrix = xx_vec * xx.transpose();

    Eigen::MatrixXd filter = (yy_matrix.cwiseProduct(yy_matrix) + xx_matrix.cwiseProduct(xx_matrix)).cwiseSqrt().array().cos();
    filter = filter.cwiseProduct(filter);
    filter = -filter;
    filter = filter.array() + 1.0;
    return filter;
}

Eigen::MatrixXd ImageDFT::neighbourhood(const Eigen::MatrixXd &f1, int centre_row, int centre_col, int radius)
{
    int size = 1 + radius * 2;
    Eigen::MatrixXd subArray(size, size);
    int row_start = centre_row - radius;
    int row_end   = centre_row + radius;
    int col_start = centre_col - radius;
    int col_end   = centre_col + radius;
    // neighbourhood falls within original size
    if (row_start > 0 && row_end < f1.rows() && col_start > 0 && col_start < f1.cols())
    {
        subArray = f1.block(row_start, col_start, size, size);
    }
    // need to wrap around
    else
    {
        for (int i = 0; i < size; i++)
        {
            int ii = (i + row_start) % f1.rows();
            for(int j = 0; j < size; j++)
             {
                 int jj = (j + col_start) % f1.cols();
                 subArray(i, j) = f1(ii, jj);
             }
        }
    }
    return subArray;
}
