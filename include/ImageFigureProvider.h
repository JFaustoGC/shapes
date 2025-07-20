//
// Created by fausto on 7/20/25.
//

#ifndef IMAGE_FIGURE_PROVIDER_H
#define IMAGE_FIGURE_PROVIDER_H

#include "FigureProvider.h"

class ImageFigureProvider : public FigureProvider {
public:
    std::vector<Figure> get_figures(const cv::Mat& ref_image, int quantity) override;

private:
    static cv::Mat convert_to_grayscale(const cv::Mat& input);
    static cv::Mat apply_threshold(const cv::Mat& gray_image);
    static cv::Mat apply_morphology(const cv::Mat& binary_image);
    static std::vector<std::vector<cv::Point>> find_sorted_contours(const cv::Mat& image, int quantity);
    static std::vector<Figure> convert_to_figures(const std::vector<std::vector<cv::Point>>& contours);

    static constexpr int THRESHOLD_VALUE = 250;
    static constexpr int FILTER_SIZE = 3;
    static constexpr int ITERATIONS = 3;

    static const std::vector<std::string> names;
};

#endif
