/*
 * Zuru Tech - Strictly Confidential
 * Copyright 2021, Zuru Tech HK Limited, All rights reserved.
 */
#include <CLI/CLI.hpp>

#include <opencv2/opencv.hpp>

#include <PillowResize/PillowResize.hpp>

namespace fs = std::filesystem;

int main(int argc, char** argv)
{
    CLI::App app{"Resize an image using Pillow resample"};

    fs::path image_path, out_path;
    app.add_option("-i,--input", image_path, "Input image path.")->mandatory();
    app.add_option("-o,--output", out_path,
                   "Output image folder. \n"
                   "If empty the output image will be saved in the "
                   "input directory with the suffix '_resized'.");

    uint width = 0, height = 0;
    app.add_option("--width", width,
                   "Output image width. (default: 0).\n"
                   "If you want to set the output image size you need to pass "
                   "both width and height.");
    app.add_option("--height", height, "Output image height. (default: 0)");

    double fx = 0, fy = 0;
    app.add_option(
        "--fx", fx,
        "Scale factor along the horizontal axis. (default: 0).\n"
        "Output image size will automatically computed as image_width*fx.\n"
        "If you want to set the scale you need to pass "
        "both fx and fy.");
    app.add_option(
        "--fy", fy,
        "Scale factor along the vertical axis. (default: 0)\n"
        "Output image size will automatically computed as image_height*fy.");

    std::map<std::string, PillowResize::InterpolationMethods> map{
        {"nearest", PillowResize::InterpolationMethods::INTERPOLATION_NEAREST},
        {"box", PillowResize::InterpolationMethods::INTERPOLATION_BOX},
        {"bilinear",
         PillowResize::InterpolationMethods::INTERPOLATION_BILINEAR},
        {"hamming", PillowResize::InterpolationMethods::INTERPOLATION_HAMMING},
        {"bicubic", PillowResize::InterpolationMethods::INTERPOLATION_BICUBIC},
        {"lanczos", PillowResize::InterpolationMethods::INTERPOLATION_LANCZOS},
    };

    PillowResize::InterpolationMethods interpolation =
        PillowResize::InterpolationMethods::INTERPOLATION_BILINEAR;
    app.add_option("-m, --method", interpolation, "Interpolation method. \n")
        ->required()
        ->transform(CLI::CheckedTransformer(map, CLI::ignore_case));

    CLI11_PARSE(app, argc, argv);

    if (out_path.empty()) {
        out_path = image_path.parent_path() /
                   (image_path.stem().string() + "_resized" +
                    image_path.extension().string());
    }
    auto out_parent_path = out_path.parent_path();
    if (out_parent_path.empty()) {
        out_path = fs::current_path() / out_path;
        out_parent_path = fs::current_path();
    }
    if (!fs::exists(out_parent_path)) {
        fs::create_directories(out_parent_path);
    }

    cv::Mat input = cv::imread(image_path, cv::IMREAD_ANYCOLOR);

    cv::Size out_size;
    if (width != 0 && height != 0) {
        out_size = cv::Size(width, height);
    }
    else if (fx > 0 && fy > 0) {
        out_size = cv::Size(input.size().width * fx, input.size().height * fy);
    }
    else {
        std::cout << "You need to set the output size. \n"
                     "Set both width and height or both fx and fy."
                  << std::endl;
        return EXIT_FAILURE;
    }
    cv::Mat out = PillowResize::resize(input, out_size, interpolation);

    cv::imwrite(out_path, out);

    return EXIT_SUCCESS;
}