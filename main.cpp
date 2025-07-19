#include <opencv2/opencv.hpp>
#include <vector>
#include <algorithm>
#include <fstream>
#include <iostream>

#include "include/Figure.h"

namespace FigureFinder {
    namespace Constants {
        constexpr int THRESHOLD_VALUE = 250;
        constexpr int FILTER_SIZE = 3;
        constexpr int ITERATIONS = 2;
    }

    static cv::Mat convert_to_grayscale(const cv::Mat &input) {
        if (input.channels() > 1) {
            cv::Mat gray;
            cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
            return gray;
        }
        return input.clone();
    }

    static cv::Mat apply_threshold(const cv::Mat &gray_image) {
        cv::Mat binary;
        cv::threshold(gray_image, binary, Constants::THRESHOLD_VALUE, 255, cv::THRESH_BINARY_INV);
        return binary;
    }

    static cv::Mat apply_morphology(const cv::Mat &binary_image) {
        const cv::Mat kernel = cv::getStructuringElement(
            cv::MORPH_RECT,
            cv::Size(Constants::FILTER_SIZE, Constants::FILTER_SIZE)
        );
        cv::Mat morphed;
        cv::morphologyEx(binary_image, morphed, cv::MORPH_OPEN, kernel,
                         cv::Point(-1, -1), Constants::ITERATIONS);
        return morphed;
    }

    static std::vector<std::vector<cv::Point> > find_sorted_contours(const cv::Mat &image) {
        std::vector<std::vector<cv::Point> > contours;
        std::vector<cv::Vec4i> hierarchy;
        cv::findContours(image, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
        std::sort(contours.begin(), contours.end(),
                  [](const auto &a, const auto &b) {
                      return a.size() > b.size();
                  });

        return contours;
    }


    static std::vector<Figure> find_figures(const cv::Mat &ref_image, const int quantity) {
        const auto gray_image = convert_to_grayscale(ref_image);
        const auto binary_image = apply_threshold(gray_image);
        const auto morphed_image = apply_morphology(binary_image);
        auto contours = find_sorted_contours(morphed_image);
        std::vector<Figure> figures;
        figures.reserve(quantity);
        const int keepCount = std::min(quantity, static_cast<int>(contours.size()));
        for (int i = 0; i < keepCount; ++i) {
            figures.emplace_back(contours[i]);
        }

        return figures;
    }
};


void save_as_csv(const std::vector<Figure> &figures,
                 const std::string &filename = "/home/fausto/CLionProjects/figuras/results.csv") {
    std::vector<std::vector<double> > vectors;

    for (auto &figure: figures) {
        auto bof = figure.find_bof();
        vectors.emplace_back(std::move(bof));
    }


    auto file = std::ofstream(filename);
    for (const auto &vector: vectors) {
        for (const auto &val: vector) {
            file << val << ",";
        }

        file << std::endl;
    }
    file.close();
}


void draw_figures(const cv::Size &size, const std::vector<Figure> &figures) {
    cv::Mat display = cv::Mat::zeros(size, CV_8UC3);
    cv::RNG rng(12345);

    for (size_t i = 0; i < figures.size(); ++i) {
        // Generate random color for each figure
        auto color = cv::Scalar(rng.uniform(100, 255), rng.uniform(100, 255), rng.uniform(100, 255));

        // Draw contour
        cv::drawContours(display, std::vector<std::vector<cv::Point> >{figures[i].get_contour()}, -1, color, 2);

        // Draw centroid
        cv::circle(display, figures[i].get_centroid(), 4, color, -1);

        // Calculate text position (below the figure)
        cv::Rect bbox = cv::boundingRect(figures[i].get_contour());
        cv::Point textOrg(bbox.x, bbox.y + bbox.height + 15);

        // Draw figure number
        cv::putText(display,
                    std::to_string(i + 1),
                    textOrg,
                    cv::FONT_HERSHEY_SIMPLEX,
                    0.7,
                    color,
                    2);
    }

    cv::imshow("Figures", display);
}

int main() {
    const std::string image_path = "../shapes.jpg";
    const auto image = read_image_gray(image_path);

    if (image.empty()) return 1;

    const auto figures = FigureFinder::find_figures(image, 24);


    save_as_csv(figures);
    draw_figures(image.size(), figures);
    cv::waitKey(0);

    return 0;
}
