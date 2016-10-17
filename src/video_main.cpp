#include <imreg_fmt/image_registration.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

int main(int argc, char **argv)
{
    cv::VideoCapture vidCap;
    cv::Mat im;

    if (argc < 2)
    {
        std::cout << "specify video file" << std::endl;
        return 1;
    }
    vidCap = cv::VideoCapture(argv[1]);
    if (!vidCap.isOpened())
    {
        std::cout << "Cannot open file " << argv[1] << std::endl;
        return 1;
    }

    vidCap >> im;

    ImageRegistration image_registration(im);

    int frame_number = 0;
    cv::Mat output;

    // x, y, rotation, scale
    std::vector<double> transform_params(4, 0.0);
    cv::Mat registered_image;
    cv::Mat previous_image;
    im.copyTo(previous_image);

    while(true)
    {
        frame_number++;
        vidCap >> im;

        if (im.empty())
        {
            std::cout << "done" << std::endl;
            break;
        }

        image_registration.registerImage(im, registered_image, transform_params, true);
        image_registration.next();

        cv::imshow("im0", previous_image);
        cv::imshow("im1", im);
        im.copyTo(previous_image);

        std::cout << "Frame " << frame_number << ", x: " << transform_params[0] << ", y: "
                  << transform_params[1] << ", rotation: " << transform_params[2]
                  << ", scale: " << transform_params[3] << std::endl;

        cv::Mat overlay_image;
        cv::addWeighted(image_registration.getCurrentImage(), 0.5, registered_image, 0.5, 0.0, overlay_image);
        cv::imshow("overlay_image", overlay_image);

        int c = cv::waitKey(1);
        if ((char)c == 'q')
        {
            exit(0);
        }
    }
    return 0;
}
