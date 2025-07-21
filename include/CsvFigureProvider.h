//
// Created by fausto on 7/20/25.
//

#ifndef CSV_FIGURE_PROVIDER_H
#define CSV_FIGURE_PROVIDER_H

#include <string>

#include "FigureProvider.h"

class CsvFigureProvider : public FigureProvider {

    static constexpr int bof_size = 180;
public:
    explicit CsvFigureProvider(std::string filename );

    std::vector<Figure> get_figures(const cv::Mat& ref_image, int quantity) override;
    void save_figures(const std::vector<Figure>& figures) const;
    bool file_exists() const;

private:
    std::string filename;
    static constexpr int BOF_SIZE = 180;
};

#endif
