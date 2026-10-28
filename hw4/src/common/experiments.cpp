#include "hw4/common.hpp"

#include <iostream>

namespace hw4 {
    Images denoiseSingles(const Image& noisy) {
        return {
            {"mean3", meanFilter(noisy, 3)},
            {"mean5", meanFilter(noisy, 5)},
            {"gaussian5_s0.8", gaussianFilter(noisy, 5, 0.8)},
            {"gaussian5_s1.5", gaussianFilter(noisy, 5, 1.5)},
        };
    }

    Images denoiseMedians(const Image& noisy) {
        return {
            {"median3", medianFilter(noisy, 3)},
            {"median5", medianFilter(noisy, 5)},
        };
    }

    Images denoiseCascades(const Image& noisy) {
        const Image median = medianFilter(noisy, 3), mean = meanFilter(noisy, 5);
        const Image gaussian = gaussianFilter(noisy, 5, 0.8);
        return {
            {"median_then_gaussian", gaussianFilter(median, 5, 0.8)},
            {"gaussian_then_median", medianFilter(gaussian, 3)},
            {"mean_then_gaussian", gaussianFilter(mean, 5, 0.8)},
            {"gaussian_then_mean", meanFilter(gaussian, 5)},
        };
    }

    std::vector<EdgeResult> standardEdges(const Image& image) {
        return {laplacian(image, false), laplacian(image, true), sobel(image, 0), sobel(image, 90),
                sobel(image, -1)};
    }

    void saveImages(const fs::path& dir, const Images& images) {
        for (const auto& [name, image] : images) {
            saveImage(dir / name, image);
            std::cout << "  Saved " << (dir / name).string() << ".raw / .png\n";
        }
    }

    void saveEdge(const fs::path& dir, const std::string& prefix, const EdgeResult& edge) {
        saveImage(dir / (prefix + "_" + edge.name + "_response_preview"),
                  edge.name == "sobel_combined" ? magnitudePreview(edge.response)
                                                : signedPreview(edge.response));
        saveImage(dir / (prefix + "_" + edge.name + "_strength_preview"),
                  magnitudePreview(edge.strength));
    }

    void writeParameters(const Paths& paths) {
        writeText(paths.output / "parameters.txt", R"(ADIP HW4 experiment parameters
Input: )" + paths.input.string() + "\nOutput: " + paths.output.string() +
                                                       R"(
All processing uses standard C++20; PNG encoding uses stb_image_write. No OpenCV calls or linkage.
Input/output: headerless 8-bit grayscale RAW; PNG is the same byte image.
Working data: double; clamp and round to [0,255] only when saving and computing reported PSNR.
Boundary: replicate nearest pixel. Gaussian kernels normalized to unit sum.
Cascade: floating-point intermediate images; no intermediate byte rounding.
Q1 median: only 3x3 and 5x5 are specified; PDF says four outputs but lists two methods.
Q1 cascade: median3 / mean5 / gaussian5 sigma=0.8.
Resize: pixel-center (half-pixel) mapping, replicated endpoints.
IBP: bilinear initialization; Gaussian5 sigma=0.8 then 2x2 cell-center average decimation.
IBP correction: bilinear upsampling then Gaussian5 sigma=0.8, gain=0.8.
IBP: clamp updated HR to [0,255], keep fractional values; snapshots at 1,3,5,10.
IBP model is an implementation choice; more iterations need not improve reference PSNR.
Laplacian: kernels [0,1,0;1,-4,1;0,1,0] and [1,1,1;1,-8,1;1,1,1].
Laplacian binary strength: absolute response, without display normalization.
Sobel0=Gx (left-to-right derivative, vertical contours), Sobel90=Gy (top-to-bottom derivative).
Combined Sobel: hypot(Gx,Gy). No gradient normalization before thresholding.
LoG: Gaussian5 sigma=1.0 then 4-neighbor Laplacian.
LoG binary strength: 8-neighbor zero-crossing contrast; mark smaller-magnitude endpoint.
LoG clean response is computed internally only to choose a reference threshold.
Display previews: 99th-percentile scaling; signed response uses gray=128 for zero.
Binary: strength > threshold. CLEAN GT uses laplacian4/laplacian8 p88; other methods p85.
Noisy threshold is selected from CLEAN 70/80/85/90/95 percentiles using reference FOM.
Matrix methods maximize mean FOM over the three fixed GT columns; Sobel0/90 use their own GT.
One threshold per noisy method, shared by all cells in its row; ties choose lower percentile.
This is best among five tested candidates on this image, not a global or unseen-image optimum.
Default noisy_*_binary images and fom_matrix.csv use selected thresholds.
Noisy p85 baseline (current GT): baseline_p85/ and fom_matrix_p85.csv; see threshold_selection.txt.
FOM matrix: noisy lap4/lap8/combined/LoG rows vs clean lap4/lap8/combined columns.
Pratt FOM: alpha=1/9, squared Euclidean distance to closest GT edge; exact separable EDT.
FOM empty-set convention: both empty=1, only one empty=0.
Threshold sweep: noisy thresholds from CLEAN 70/80/85/90/95 percentiles;
GT remains fixed throughout the sweep: CLEAN laplacian4/laplacian8 p88, other methods p85.
The sweep also includes Sobel0 and Sobel90 against their own fixed clean GT.
)");
    }
} // namespace hw4
