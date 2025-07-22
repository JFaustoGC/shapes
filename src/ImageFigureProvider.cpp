//
// Created by fausto on 7/20/25.
//

#include "../include/ImageFigureProvider.h"

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

// const std::vector<std::string> ImageFigureProvider::names = {
//     "circle", "ellipse", "oval", "square", "rectangle", "trapezium",
//     "parallelogram", "rhombus", "kite", "triangle", "right triangle", "scalene triangle",
//     "pentagon", "hexagon", "heptagon", "octagon", "nonagon", "decagon",
//     "star", "heart", "crescent", "cross", "pie", "arrow"
// };

const std::vector<std::string> ImageFigureProvider::names = {
    "circle", "ellipse", "oval", "square", "rectangle", "trapezium",
    "rhombus", "kite", "triangle", "parallelogram", "right triangle", "scalene triangle",
    "pentagon", "hexagon", "heptagon", "octagon", "nonagon", "decagon",
    "star", "heart", "crescent", "cross", "pie", "arrow"
};

cv::Mat ImageFigureProvider::convert_to_grayscale(const cv::Mat &input) {
    if (input.channels() > 1) {
        cv::Mat gray;
        cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
        return gray;
    }
    return input.clone();
}

cv::Mat ImageFigureProvider::apply_threshold(const cv::Mat &gray_image) {
    cv::Mat binary;
    cv::threshold(gray_image, binary, THRESHOLD_VALUE, 255, cv::THRESH_BINARY_INV);
    return binary;
}

cv::Mat ImageFigureProvider::apply_morphology(const cv::Mat &binary_image) {
    const cv::Mat kernel = cv::getStructuringElement(
        cv::MORPH_RECT,
        cv::Size(FILTER_SIZE, FILTER_SIZE)
    );
    cv::Mat morphed;
    cv::morphologyEx(binary_image, morphed, cv::MORPH_OPEN, kernel,
                     cv::Point(-1, -1), ITERATIONS);
    return morphed;
}

std::vector<std::vector<cv::Point> >
ImageFigureProvider::find_sorted_contours(const cv::Mat &image, const int quantity) {
    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(image, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);

    std::sort(contours.begin(), contours.end(),
              [](const auto &a, const auto &b) {
                  return a.size() > b.size();
              });

    if (contours.size() > quantity) {
        contours.resize(quantity);
    }

    std::sort(contours.begin(), contours.end(),
              [](const auto &a, const auto &b) {
                  const cv::Rect ra = cv::boundingRect(a);
                  const cv::Rect rb = cv::boundingRect(b);
                  constexpr int row_tolerance = 10;
                  if (std::abs(ra.y - rb.y) > row_tolerance)
                      return ra.y < rb.y;
                  return ra.x < rb.x;
              });


    return contours;
}


std::vector<Figure> ImageFigureProvider::convert_to_figures(const std::vector<std::vector<cv::Point> > &contours) {
    const int size = static_cast<int>(contours.size());
    std::vector<Figure> figures;
    figures.reserve(size);
    const int keepCount = std::min(size, static_cast<int>(contours.size()));
    for (int i = 0; i < keepCount; ++i) {
        figures.emplace_back(contours[i], names[i]);
    }

    return figures;
}


std::vector<Figure> ImageFigureProvider::get_figures(const cv::Mat &ref_image, const int quantity) {
    const auto gray_image = convert_to_grayscale(ref_image);
    const auto binary_image = apply_threshold(gray_image);
    const auto morphed_image = apply_morphology(binary_image);
    const auto contours = find_sorted_contours(morphed_image, quantity);
    return convert_to_figures(contours);
}
