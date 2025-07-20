//
// Created by fausto on 7/20/25.
//

#ifndef FIGURE_PROVIDER_H
#define FIGURE_PROVIDER_H

#include <vector>
#include "Figure.h"

class FigureProvider {
public:
    virtual ~FigureProvider() = default;

    virtual std::vector<Figure> get_figures(const cv::Mat &ref_image, int quantity) = 0;
};


#endif //FIGURE_PROVIDER_H
