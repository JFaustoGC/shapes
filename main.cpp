#include <opencv2/opencv.hpp>
#include <vector>
#include <algorithm>
#include <iostream>

cv::Mat read_image_gray(const std::string &image_path) {
    cv::Mat image = cv::imread(image_path, cv::IMREAD_GRAYSCALE);
    if (image.empty()) {
        std::cerr << "Could not load image!" << std::endl;
        return {};
    }
    return image;
}

cv::Mat preprocess_binary(const cv::Mat &image, const int threshold_value = 250, const int filter_size = 3,
                          const int iterations = 2) {
    cv::Mat binarized;
    cv::threshold(image, binarized, threshold_value, 255, cv::THRESH_BINARY_INV);

    const cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(filter_size, filter_size));
    cv::morphologyEx(binarized, binarized, cv::MORPH_OPEN, kernel, cv::Point(-1, -1), iterations);

    return binarized;
}

cv::Mat filter_largest_blobs(const cv::Mat &binary_image, const int max_blobs = 24) {
    cv::Mat labels, stats, centroids;
    const int numLabels = cv::connectedComponentsWithStats(binary_image, labels, stats, centroids);

    std::vector<std::pair<int, int> > blobs;
    for (int i = 1; i < numLabels; ++i) {
        // skip background
        int area = stats.at<int>(i, cv::CC_STAT_AREA);
        blobs.emplace_back(i, area);
    }

    std::sort(blobs.begin(), blobs.end(),
              [](const auto &a, const auto &b) {
                  return a.second > b.second;
              });

    cv::Mat filtered = cv::Mat::zeros(binary_image.size(), CV_8UC1);
    const int keepCount = std::min(max_blobs, static_cast<int>(blobs.size()));
    for (int i = 0; i < keepCount; ++i) {
        const int label = blobs[i].first;
        filtered.setTo(255, labels == label);
    }

    return filtered;
}


std::vector<std::pair<cv::Rect, std::vector<cv::Point> > > find_contours(const cv::Mat &image) {
    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(image, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    std::vector<std::pair<cv::Rect, std::vector<cv::Point> > > sortedContours;
    for (const auto &contour: contours) {
        cv::Rect bbox = cv::boundingRect(contour);
        sortedContours.emplace_back(bbox, contour);
    }

    std::sort(sortedContours.begin(), sortedContours.end(),
              [](const auto &a, const auto &b) {
                  constexpr int row_tolerance = 10;
                  if (std::abs(a.first.y - b.first.y) > row_tolerance)
                      return a.first.y < b.first.y;
                  return a.first.x < b.first.x;
              });

    return sortedContours;
}

std::vector<cv::Point> centroids(const std::vector<std::pair<cv::Rect, std::vector<cv::Point> > > &contours) {
    std::vector<cv::Point> centroids;
    for (const auto &[boundingBox, contour]: contours) {
        const cv::Moments m = cv::moments(contour);
        int cx = static_cast<int>(m.m10 / m.m00);
        int cy = static_cast<int>(m.m01 / m.m00);
        centroids.emplace_back(cx, cy);
    }
    return centroids;
}

std::vector<std::vector<double> > get_bofs(const std::vector<std::pair<cv::Rect, std::vector<cv::Point> > > &contours,
                                           const std::vector<cv::Point> &centroids) {
    std::vector<std::vector<double> > vectors;

    for (size_t i = 0; i < contours.size(); ++i) {
        const auto &contour = contours[i].second;
        const auto &centroid = centroids[i];

        std::vector<cv::Point> sorted = contour;
        // std::sort(sorted.begin(), sorted.end(), [&](const cv::Point &a, const cv::Point &b) {
        //     const double angle_a = std::atan2(a.y - centroid.y, a.x - centroid.x);
        //     const double angle_b = std::atan2(b.y - centroid.y, b.x - centroid.x);
        //     return angle_a < angle_b;
        // });

        // must resize here


        std::vector<double> bof;
        bof.reserve(sorted.size());
        for (const auto &pt: sorted) {
            const double dist = cv::norm(centroid - pt);
            bof.push_back(dist);
        }

        vectors.emplace_back(std::move(bof));
    }

    return vectors;
}

std::vector<std::vector<double>> resize_normalize_bofs(std::vector<std::vector<double> > &vectors,
                                                       const int newSize = 180) {
    std::vector<std::vector<double> > bofs;
    for (auto &vector: vectors) {
        const double max = *std::max_element(vector.begin(), vector.end());
        for (auto &val: vector) {
            val /= max;
        }

        cv::Mat bof(1, newSize, CV_32FC1);
        auto vecMat = cv::Mat(vector);
        cv::resize(vecMat, bof, cv::Size(newSize, 1), 0, 0, cv::INTER_NEAREST);
        bofs.emplace_back(std::move(bof));
    }
    return bofs;
}


void draw_contours_with_centroids(const cv::Size size,
                                  const std::vector<std::pair<cv::Rect, std::vector<cv::Point> > > &contours,
                                  const std::vector<cv::Point> &centroids) {
    cv::Mat contourImage = cv::Mat::zeros(size, CV_8UC3);
    cv::RNG rng(12345);

    for (size_t i = 0; i < contours.size(); ++i) {
        auto color = cv::Scalar(rng.uniform(100, 255), rng.uniform(100, 255), rng.uniform(100, 255));
        cv::drawContours(contourImage,
                         std::vector<std::vector<cv::Point> >{contours[i].second},
                         -1, color, 2);

        if (i < centroids.size()) {
            cv::circle(contourImage, centroids[i], 4, color, -1);
        }

        const cv::Rect bbox = contours[i].first;
        const cv::Point textOrg(bbox.x, bbox.y + bbox.height + 15);


        cv::putText(contourImage,
                    std::to_string(i + 1),
                    textOrg,
                    cv::FONT_HERSHEY_SIMPLEX,
                    0.7,
                    color,
                    2);
    }


    cv::imshow("Sorted Contours", contourImage);
}

int main() {
    const std::string image_path = "../shapes.jpg";
    const auto image = read_image_gray(image_path);

    if (image.empty()) return 1;

    const auto binarized = preprocess_binary(image);
    const auto filtered = filter_largest_blobs(binarized);
    const auto sortedContours = find_contours(filtered);
    const auto sortedCentroids = centroids(sortedContours);
    auto bofs = get_bofs(sortedContours, sortedCentroids);
    auto normal_bofs = resize_normalize_bofs(bofs);
    draw_contours_with_centroids(image.size(), sortedContours, sortedCentroids);
    cv::waitKey(0);

    return 0;
}
