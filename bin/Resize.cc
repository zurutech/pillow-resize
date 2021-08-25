/*
* Copyright 2021 Zuru Tech HK Limited
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* istributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <sstream>

#include <CLI/CLI.hpp>
#include <magic_enum.hpp>

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
    app.add_option("--ow,--width", width,
                   "Output image width. (default: 0).\n"
                   "If you want to set the output image size you need to pass "
                   "both width and height.");
    app.add_option("--oh,--height", height,
                   "Output image height. (default: 0)");

    double fx = 0., fy = 0.;
    app.add_option("--horizontal_scale_factor,--fx", fx,
                   "Scale factor along the horizontal axis. (default: 0).\n"
                   "Output image size will automatically computed as "
                   "image_width*horizontal_scale_factor.\n"
                   "If you want to set the scale you need to pass "
                   "both horizontal_scale_factor and vertical_scale_factor.");
    app.add_option("--vertical_scale_factor,--fy", fy,
                   "Scale factor along the vertical axis. (default: 0)\n"
                   "Output image size will automatically computed as "
                   "image_height*vertical_scale_factor.");

    // enum_names return list of name sorted by enum values.
    constexpr auto& interpolation_names =
        magic_enum::enum_names<PillowResize::InterpolationMethods>();
    std::stringstream interpolation_help;
    interpolation_help << "Interpolation method. \nValid values in:\n";
    for (size_t i = 0; i < interpolation_names.size(); ++i) {
        interpolation_help << interpolation_names[i] << " -> " << i
                           << std::endl;
    }

    auto transformArgToEnum = [](const std::string& s) -> std::string {
        auto isInt = [](const std::string& s, int& value) -> bool {
            try {
                value = std::stoi(s);
                return true;
            }
            catch (const std::exception& e) {
                return false;
            }
        };

        std::optional<PillowResize::InterpolationMethods> interpolation;
        int arg_converted;
        if (isInt(s, arg_converted)) {
            interpolation =
                magic_enum::enum_cast<PillowResize::InterpolationMethods>(
                    arg_converted);
        }
        else {
            interpolation =
                magic_enum::enum_cast<PillowResize::InterpolationMethods>(s);
        }
        if (!interpolation.has_value()) {
            throw CLI::ValidationError("Interpolation method not valid.");
        }
        return std::to_string(magic_enum::enum_integer(interpolation.value()));
    };

    PillowResize::InterpolationMethods interpolation =
        PillowResize::InterpolationMethods::INTERPOLATION_BILINEAR;
    app.add_option("-m, --method", interpolation, interpolation_help.str())
        ->required()
        ->transform(transformArgToEnum);

    CLI11_PARSE(app, argc, argv);

    // Check if out_path is empty.
    // If empty, create out_path from image_path with the suffix _resized.
    if (out_path.empty()) {
        out_path = image_path.parent_path() /
                   (image_path.stem().string() + "_" +
                    std::string(magic_enum::enum_name(interpolation)) +
                    image_path.extension().string());
    }

    auto out_parent_path = out_path.parent_path();
    // If image_path is a relative path to the current directory,
    // its parent path will be empty.
    // This is necessary in case the library is installed and the binary
    // is run using its full path (installation folder) from a folder with images.
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
