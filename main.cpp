#include <opencv2/opencv.hpp>
#include <vector>
#include <fstream>

#include "include/Figure.h"
#include "include/FigureProvider.h"
#include "include/CsvFigureProvider.h"
#include "include/ImageFigureProvider.h"


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
    const std::string image_path = "../img/shapes.jpg";
    const std::string csv_path = "../img/shapes.csv";
    const std::string square = "../img/oval.png";

    const auto image = cv::imread(image_path);

    if (image.empty()) return 1;

    ImageFigureProvider provider;
    const auto figures = provider.get_figures(image, 24);
    const auto square_image = cv::imread(square);
    const auto square_figures = provider.get_figures(square_image, 1);
    //const auto figures = read_from_csv();

    const auto match = square_figures[0].find_closest(figures);
    std::cout << match.get_name() << std::endl;


    //save_as_csv(figures);
    draw_figures(image.size(),std::vector{match});
    cv::waitKey(0);

    return 0;
}
