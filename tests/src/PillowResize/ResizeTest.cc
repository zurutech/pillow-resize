/*
* Copyright 2023 Zuru Tech HK Limited
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

#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <magic_enum.hpp>

#include <opencv2/opencv.hpp>

#include <PillowResize/PillowResize.hpp>

namespace {

namespace fs = std::filesystem;

class ResizeTest : public testing::TestWithParam<std::tuple<cv::Size, int>> {
protected:
    fs::path base_path{"assets"};

    static constexpr auto interpolation_names =
        magic_enum::enum_names<PillowResize::InterpolationMethods>();
};

TEST_P(ResizeTest, TestEqualColor)
{
    auto [size, interpolation_method] = GetParam();

    cv::Mat input = cv::imread(base_path / "test.png", cv::IMREAD_COLOR);

    const auto interpolation_name = interpolation_names[interpolation_method];
    std::ostringstream oss;
    oss << interpolation_name << "_" << size.width << "_" << size.height
        << ".png";
    std::string img_name = oss.str();

    // These images have been obtained using Pillow from python.
    // In particular, each image is loaded and saved using OpenCV but the resize
    // operation is done with Pillow (using bilinear interpolation).
    // We noticed that if the image is loaded/saved using Pillow the results are slightly different.
    cv::Mat input_dest = cv::imread(base_path / img_name, cv::IMREAD_COLOR);

    cv::Size dst_size(input_dest.size().width, input_dest.size().height);

    cv::Mat out = PillowResize::resize(input, dst_size, interpolation_method);
    cv::Mat res_compare;
    cv::compare(out, input_dest, res_compare, cv::CMP_NE);
    cv::cvtColor(res_compare, res_compare, cv::COLOR_BGR2GRAY);
    int diff_non_zero = cv::countNonZero(res_compare);
    ASSERT_EQ(diff_non_zero, 0);
}

TEST_P(ResizeTest, TestEqualGray)
{
    auto [size, interpolation_method] = GetParam();

    cv::Mat input = cv::imread(base_path / "test.png", cv::IMREAD_COLOR);
    cv::cvtColor(input, input, cv::COLOR_BGR2GRAY);

    const auto interpolation_name = interpolation_names[interpolation_method];
    std::ostringstream oss;
    oss << interpolation_name << "_" << size.width << "_" << size.height
        << "_gray.png";
    std::string img_name = oss.str();

    cv::Mat input_dest = cv::imread(base_path / img_name, cv::IMREAD_GRAYSCALE);

    cv::Size dst_size(input_dest.size().width, input_dest.size().height);

    cv::Mat out = PillowResize::resize(input, dst_size, interpolation_method);
    cv::Mat res_compare;
    cv::compare(out, input_dest, res_compare, cv::CMP_NE);
    int diff_non_zero = cv::countNonZero(res_compare);
    ASSERT_EQ(diff_non_zero, 0);
}

INSTANTIATE_TEST_SUITE_P(ResizeGroup,
                         ResizeTest,
                         testing::Combine(testing::Values(cv::Size(600, 400),
                                                          cv::Size(1200, 1000)),
                                          testing::Values(0, 1, 2, 3, 4, 5)));

}    // namespace