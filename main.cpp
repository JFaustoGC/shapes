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
    for (const auto &[fst, snd]: contours) {
        centroids.emplace_back(fst.x + fst.width / 2, fst.y + fst.height / 2);
    }
    return centroids;
}

void draw_contours_with_centroids(const cv::Size size, const std::vector<std::pair<cv::Rect, std::vector<cv::Point> > > &contours, std::vector<cv::Point> centroids ) {
    cv::Mat contourImage = cv::Mat::zeros(size, CV_8UC3);
    cv::RNG rng(12345);

    for (size_t i = 0; i < contours.size(); ++i) {
        auto color = cv::Scalar(rng.uniform(100, 255), rng.uniform(100, 255), rng.uniform(100, 255));
        cv::drawContours(contourImage,
                         std::vector<std::vector<cv::Point> >{contours[i].second},
                         -1, color, 2);


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
    auto sortedContours = find_contours(filtered);
    auto sortedCentroids = centroids(sortedContours);
    draw_contours_with_centroids(sortedContours[0].first.size(), sortedContours, sortedCentroids);
    cv::waitKey(0);

    return 0;
}
