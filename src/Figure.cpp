//
// Created by fausto on 7/18/25.
//

#include "../include/Figure.h"

#include <opencv2/imgproc.hpp>

Figure::Figure(const std::vector<cv::Point> &contour) : contour(contour) {
    moments = cv::moments(contour);
    centroid = cv::Point(moments.m10 / moments.m00, moments.m01 / moments.m00);
}

const cv::Point &Figure::get_centroid() const {
    return centroid;
}

const std::vector<cv::Point> & Figure::get_contour() const {
    return contour;
}


bool Figure::operator<(const Figure &other) const {
    if (constexpr int row_tolerance = 10; std::abs(centroid.y - other.centroid.y) > row_tolerance) {
        return centroid.y < other.centroid.y;
    }
    return centroid.x < other.centroid.x;
}


std::vector<double> Figure::find_bof() const {
    std::vector<cv::Point> sorted = contour;
    std::sort(sorted.begin(), sorted.end(), [&](const cv::Point &a, const cv::Point &b) {
        const double angle_a = std::atan2(a.y - centroid.y, a.x - centroid.x);
        const double angle_b = std::atan2(b.y - centroid.y, b.x - centroid.x);
        return angle_a < angle_b;
    });

    std::vector<double> bof;
    bof.reserve(sorted.size());
    for (const auto &pt: sorted) {
        const double dist = cv::norm(centroid - pt);
        bof.push_back(dist);
    }

    const double max = *std::max_element(bof.begin(), bof.end());
    for (auto &val: bof) {
        val /= max;
    }

    std::vector<double> n_bof;
    cv::resize(bof, n_bof, cv::Size(newSize, 1), 0, 0, cv::INTER_CUBIC);
    return n_bof;
}
