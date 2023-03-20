/*
 * Zuru Tech - Strictly Confidential
 * Copyright 2021, Zuru Tech HK Limited, All rights reserved.
 */

#include <filesystem>
#include <vector>

#include <gtest/gtest.h>

#include <opencv2/opencv.hpp>

#include <PillowResize/PillowResize.hpp>

namespace {

namespace fs = std::filesystem;

class ResizeTest : public testing::TestWithParam<std::tuple<fs::path, int>> {
protected:
    fs::path base_path{"tests/assets"};
};

TEST_P(ResizeTest, TestEqualColor)
{
    auto [img_path, interpolation_method] = GetParam();

    cv::Mat input =
        cv::imread((base_path / "glare.png").string(), cv::IMREAD_COLOR);

    // This image has been obtained using Pillow from python.
    // In particular, the image is loaded and saved using OpenCV but the resize
    // operation is done with Pillow (using bilinear interpolation).
    // We noticed that if the image is loaded/saved using Pillow the results are slightly different.
    cv::Mat input_dest =
        cv::imread((base_path / img_path).string(), cv::IMREAD_COLOR);

    cv::Size dst_size(input_dest.size().width, input_dest.size().height);

    cv::Mat out = PillowResize::resize(input, dst_size, interpolation_method);
    cv::Mat res_compare;
    cv::compare(out, input_dest, res_compare, cv::CMP_NE);
    cv::cvtColor(res_compare, res_compare, cv::COLOR_BGR2GRAY);
    int diff_non_zero = cv::countNonZero(res_compare);
    ASSERT_EQ(diff_non_zero, 0);
}

// TEST_P(ResizeTest, TestEqualGray)
// {
//     auto [img_path, interpolation_method] = GetParam();

//     cv::Mat input =
//         cv::imread((base_path / "glare.png").string(), cv::IMREAD_GRAYSCALE);

//     // This image has been obtained using Pillow from python.
//     // In particular, the image is loaded and saved using OpenCV but the resize
//     // operation is done with Pillow (using bilinear interpolation).
//     // We noticed that if the image is loaded/saved using Pillow the results are slightly different.
//     cv::Mat input_dest =
//         cv::imread((base_path / img_path).string(), cv::IMREAD_GRAYSCALE);

//     cv::Size dst_size(input_dest.size().width, input_dest.size().height);

//     cv::Mat out = PillowResize::resize(input, dst_size, interpolation_method);
//     cv::Mat res_compare;
//     cv::compare(out, input_dest, res_compare, cv::CMP_NE);
//     int diff_non_zero = cv::countNonZero(res_compare);
//     ASSERT_EQ(diff_non_zero, 0);
// }

INSTANTIATE_TEST_SUITE_P(
    ResizeGroup,
    ResizeTest,
    testing::Values(
        // std::make_tuple(
        //     "nearest_400_600.png",
        //     PillowResize::InterpolationMethods::INTERPOLATION_NEAREST),
        // std::make_tuple(
        //     "bilinear_400_600.png",
        //     PillowResize::InterpolationMethods::INTERPOLATION_BILINEAR),
        // std::make_tuple(
        //     "glare_resized_bilinear.png",
        //     PillowResize::InterpolationMethods::INTERPOLATION_BILINEAR),
        std::make_tuple(
            "bicubic.png",
            PillowResize::InterpolationMethods::INTERPOLATION_BICUBIC)));

}    // namespace