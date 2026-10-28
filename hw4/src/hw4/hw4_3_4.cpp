#include "hw4/hw4_3_4.hpp"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace hw4 {
    namespace {
        const std::vector<std::string> matrixColumns{
            "laplacian4", "laplacian8", "sobel_combined"};

        std::vector<std::string> evaluationColumns(const std::string& method) {
            if (method == "sobel0" || method == "sobel90") {
                return {method};
            }
            return matrixColumns;
        }

        std::string fomMatrix(const Images& gt, const Images& detected) {
            std::ostringstream table;
            table << std::fixed << std::setprecision(8)
                  << "noisy_method,GT_laplacian4,GT_laplacian8,GT_sobel_combined\n";
            for (const std::string method : {"laplacian4", "laplacian8", "sobel_combined", "log"}) {
                table << method;
                for (const auto& column : matrixColumns) {
                    table << ',' << prattFom(gt.at(column), detected.at(method));
                }
                table << '\n';
            }
            return table.str();
        }

        int gtPercentile(const std::string& method) {
            return (method == "laplacian4" || method == "laplacian8") ? 88 : 85;
        }

        auto edgeCount(const Image& image) {
            return std::count_if(image.pixels.begin(), image.pixels.end(),
                                 [](double p) { return p > 0; });
        }
    } // namespace

    void runHw4_3_4(const Paths& paths) {
        const Image clean = readRaw(paths.input / "cameraman256.raw", 256, 256);
        const Image noisy = readRaw(paths.input / "cameraman256_noise.raw", 256, 256);
        auto cleanEdges = standardEdges(clean), noisyEdges = standardEdges(noisy);
        cleanEdges.push_back(logEdges(clean)); // Threshold calibration only, not a matrix GT.
        noisyEdges.push_back(logEdges(noisy));
        const fs::path dir = paths.output / "hw4_3_4";
        Images gt, baseline, selected;
        std::map<std::string, double> gtThresholds;
        for (const auto& edge : cleanEdges) {
            const double threshold = percentile(edge.strength, gtPercentile(edge.name) / 100.0);
            gtThresholds[edge.name] = threshold;
            gt[edge.name] = binarize(edge.strength, threshold);
            if (edge.name != "log") {
                saveImage(dir / ("clean_" + edge.name + "_binary"), gt.at(edge.name));
            }
        }

        std::ostringstream sweep, thresholds;
        sweep << std::fixed << std::setprecision(8)
              << "noisy_method,clean_threshold_percentile,threshold,gt_method,gt_fixed_threshold,"
                 "fom\n";
        thresholds << std::fixed << std::setprecision(8)
                   << "method,gt_percentile,gt_threshold,selected_percentile,selected_threshold,"
                      "baseline_objective,selected_objective,clean_reference_edge_pixels,"
                      "selected_noisy_edge_pixels\n";

        for (std::size_t i = 0; i < noisyEdges.size(); ++i) {
            const std::string& name = noisyEdges[i].name;
            const auto columns = evaluationColumns(name);
            int bestPercentile = 0;
            double bestThreshold = 0, bestScore = -1, baselineScore = 0;
            Image bestBinary;
            // Same candidate budget for every method; a single threshold is used for each row.
            for (int p : {70, 80, 85, 90, 95}) {
                const double threshold = percentile(cleanEdges[i].strength, p / 100.0);
                const Image binary = binarize(noisyEdges[i].strength, threshold);
                saveImage(dir / "threshold_sweep" / ("noisy_" + name + "_p" + std::to_string(p)),
                          binary);
                double score = 0;
                for (const auto& col : columns) {
                    const double fom = prattFom(gt.at(col), binary);
                    score += fom;
                    sweep << name << ',' << p << ',' << threshold << ',' << col << ','
                          << gtThresholds.at(col) << ',' << fom << '\n';
                }
                score /= static_cast<double>(columns.size());
                if (p == 85) {
                    baseline[name] = binary;
                    baselineScore = score;
                    saveImage(dir / "baseline_p85" / ("noisy_" + name + "_binary"), binary);
                }
                // Ties retain the lower candidate percentile (first in ascending order).
                if (score > bestScore) {
                    bestScore = score;
                    bestPercentile = p;
                    bestThreshold = threshold;
                    bestBinary = binary;
                }
            }
            selected[name] = bestBinary;
            saveImage(dir / ("noisy_" + name + "_binary"), bestBinary);
            thresholds << name << ',' << gtPercentile(name) << ',' << gtThresholds.at(name) << ',' << bestPercentile << ','
                       << bestThreshold << ',' << baselineScore << ',' << bestScore << ','
                       << edgeCount(gt.at(name)) << ',' << edgeCount(bestBinary) << '\n';
        }
        writeText(dir / "threshold_sweep.csv", sweep.str());
        writeText(dir / "thresholds.csv", thresholds.str());
        writeText(dir / "fom_matrix_p85.csv", fomMatrix(gt, baseline));
        const std::string matrix = fomMatrix(gt, selected);
        writeText(dir / "fom_matrix.csv", matrix);
        writeText(dir / "threshold_selection.txt", R"(Threshold selection for HW4 3-4
GT is fixed at CLEAN laplacian4/laplacian8 p88, all other methods p85; not optimized in the sweep.
Candidates for noisy thresholds: CLEAN method's 70/80/85/90/95 percentiles.
For each matrix row, maximize mean FOM over the three fixed matrix GT columns.
For Sobel0/90 (outside the matrix), maximize FOM against its own fixed clean GT.
One selected threshold and one noisy binary image are used for the whole row.
Ties choose the lower percentile. No per-cell threshold selection.
This selects the best of five tested thresholds on this image, not a global optimum.
GT is used to tune the threshold: these are reference-tuned experimental results,
not performance on unseen images or a method for choosing thresholds without a clean reference.

Report images: clean_*_binary.png (fixed GT), noisy_*_binary.png (selected result).
Report table: fom_matrix.csv (selected thresholds).
Selection details: thresholds.csv (GT and noisy thresholds recorded separately).
Baseline: fom_matrix_p85.csv and baseline_p85/noisy_*_binary.png use NOISY p85 and current fixed GT.
All candidates and scores: threshold_sweep/ and threshold_sweep.csv.
)");
        std::cout << "Best tested thresholds (GT fixed; selection uses reference FOM):\n"
                  << thresholds.str() << '\n' << matrix;
    }
} // namespace hw4
