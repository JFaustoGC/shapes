//
// Created by fausto on 7/18/25.
//

#ifndef FIGURE_H
#define FIGURE_H
#include <opencv2/core/types.hpp>


class Figure {
    std::vector<cv::Point> contour;
    cv::Moments moments;
    cv::Point centroid;
    std::string name;
    std::vector <double> bof;

    static constexpr int BOF_SIZE = 180;

public:
    explicit Figure(const std::vector<cv::Point> &contour, std::string name);

    explicit Figure(const std::vector<double> &bof, std::string name, cv::Point centroid);

    const Figure& find_closest(const std::vector<Figure> &figures) const;

    const cv::Point &get_centroid() const;

    const std::vector<cv::Point> &get_contour() const;

    const std::string &get_name() const;

    std::vector<double> find_bof() const;

    bool operator<(const Figure &other) const;
};


#endif //FIGURE_H
