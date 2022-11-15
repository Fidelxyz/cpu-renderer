#ifndef GAMMA_CORRECTION
#define GAMMA_CORRECTION

#include <opencv2/opencv.hpp>

class GammaCorrection {
   public:
    static void init_luts();

    static void from_linear(cv::Mat *src);
    static void to_linear(cv::Mat *src);

   private:
    GammaCorrection() = delete;

   public:
    static const float gamma;

    static cv::Mat lut_from_linear;
    static cv::Mat lut_to_linear;
};

#endif