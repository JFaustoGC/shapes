//
// Created by fausto on 7/18/25.
//

#include "../include/Figure.h"

#include <opencv2/imgproc.hpp>
#include <utility>
#include <iostream>

Figure::Figure(const std::vector<cv::Point> &contour, std::string name) : contour(contour) {
    moments = cv::moments(contour);
    centroid = cv::Point(
        static_cast<int>(moments.m10 / moments.m00),
        static_cast<int>(moments.m01 / moments.m00)
    );
    this->name = std::move(name);
}

Figure::Figure(const std::vector<double> &bof, std::string name, cv::Point centroid) {
    this->name = std::move(name);
    const double max_dist = 100.0;
    std::vector<cv::Point> points;
    points.reserve(bof.size());

    const double angle_step = 2 * M_PI / bof.size();
    for (size_t i = 0; i < bof.size(); ++i) {
        const double angle = i * angle_step;
        const double dist = bof[i] * max_dist;

        const int x = static_cast<int>(centroid.x + dist * std::cos(angle));
        const int y = static_cast<int>(centroid.y + dist * std::sin(angle));
        points.emplace_back(x, y);
    }

    this->contour = std::move(points);
    this->moments = cv::moments(contour);
}

const Figure& Figure::find_closest(const std::vector<Figure>& figures) const {
    if (figures.empty()) {
        throw std::invalid_argument("Cannot find closest in empty vector");
    }

    const std::vector<double> this_bof = this->find_bof();

    double min_distance = std::numeric_limits<double>::max();
    size_t closest_index = 0;

    for (size_t i = 0; i < figures.size(); ++i) {
        const std::vector<double> other_bof = figures[i].find_bof();
        double current_distance = cv::norm(this_bof, other_bof, cv::NORM_L2);

        std::cout << "Figure: " << figures[i].get_name()  // Adjust if you use another method or member
                  << " | Distance: " << current_distance << '\n';

        if (current_distance < min_distance) {
            min_distance = current_distance;
            closest_index = i;
        }
    }

    return figures[closest_index];
}


const cv::Point &Figure::get_centroid() const {
    return centroid;
}

const std::vector<cv::Point> &Figure::get_contour() const {
    return contour;
}

const std::string &Figure::get_name() const {
    return name;
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
