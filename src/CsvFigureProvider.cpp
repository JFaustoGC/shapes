//
// Created by fausto on 7/20/25.
//

#include "../include/CsvFigureProvider.h"
#include <fstream>
#include <sstream>


CsvFigureProvider::CsvFigureProvider(std::string filename) {
    this->filename = filename;
    if (!file_exists()) {
        std::ofstream file(filename);
        file.close();
    }
}

std::vector<Figure> CsvFigureProvider::get_figures(const cv::Mat &ref_image, int quantity) {
    std::vector<Figure> figures;
    std::ifstream file(filename);
    std::string line;

    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file: " + filename);
    }

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string value;
        std::vector<double> bof;
        bof.reserve(bof_size);
        cv::Point centroid;
        std::string name;
        int count = 0;

        while (std::getline(ss, value, ',')) {
            if (count < bof_size) {
                bof.push_back(std::stod(value));
            } else if (count == bof_size + 1) {
                centroid.x = std::stoi(value);
            } else if (count == bof_size + 2) {
                centroid.y = std::stoi(value);
            } else if (count == bof_size + 4) {
                name = value;
            }
            count++;
        }

        if (bof.size() == bof_size && !name.empty()) {
            figures.emplace_back(bof, name, centroid);
        }
    }

    return figures;
}

void CsvFigureProvider::save_figures(const std::vector<Figure> &figures) const {
    auto file = std::ofstream(filename);

    for (const auto &figure: figures) {
        auto bof = figure.find_bof();
        for (const auto &val: bof) {
            file << val << ",";
        }

        file << ",";

        const auto &centroid = figure.get_centroid();
        file << centroid.x << "," << centroid.y;

        file << ",," << figure.get_name();

        file << std::endl;
    }
    file.close();
}

bool CsvFigureProvider::file_exists() const {
    const std::ifstream f(filename.c_str());
    return f.good();
}
