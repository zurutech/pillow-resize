/*
 * Zuru Tech - Strictly Confidential
 * Copyright 2021, Zuru Tech HK Limited, All rights reserved.
 */

#ifndef PILLOWRESIZE_HPP
#define PILLOWRESIZE_HPP

#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

#include <opencv2/opencv.hpp>

/**
 * \brief PillowResize Porting of the resize methods from Pillow library
 * (https://github.com/python-pillow/Pillow).
 * The implementation depends only on OpenCV, so all Pillow code has been
 * converted to use cv::Mat and OpenCV structures.
 * Since Pillow does not support natively all cv::Mat types, this implementation
 * extends the support to almost all OpenCV pixel types.
 */
class PillowResize {
protected:
    /**
     * \brief precision_bits 8 bits for result. Filter can have negative areas.
     * In one case the sum of the coefficients will be negative,
     * in the other it will be more than 1.0. That is why we need
     * two extra bits for overflow and int type.
     */
    static constexpr int precision_bits = 32 - 8 - 2;

    /**
     * \brief Filter Abstract class to handle the filters used by
     * the different interpolation methods.
     */
    class Filter {
    protected:
        double _support; /** Support size (length of resampling filter). */

    public:
        /**
         * \brief Construct a new Filter object.
         *
         * \param[in] support Support size (length of resampling filter).
         */
        Filter(double support) : _support{support} {};

        /**
         * \brief filter Apply filter.
         *
         * \param[in] x Input value.
         *
         * \return Processed value by the filter.
         */
        [[nodiscard]] virtual double filter(double x) const = 0;

        /**
         * \brief support Get support size.
         *
         * \return support size.
         */
        [[nodiscard]] double support() const { return _support; };
    };

    class BoxFilter : public Filter {
    public:
        BoxFilter() : Filter(0.5){};
        [[nodiscard]] double filter(double x) const override;
    };

    class BilinearFilter : public Filter {
    public:
        BilinearFilter() : Filter(1.){};
        [[nodiscard]] double filter(double x) const override;
    };

    class HammingFilter : public Filter {
    public:
        HammingFilter() : Filter(1.){};
        [[nodiscard]] double filter(double x) const override;
    };

    class BicubicFilter : public Filter {
    public:
        BicubicFilter() : Filter(2.){};
        [[nodiscard]] double filter(double x) const override;
    };

    class LanczosFilter : public Filter {
    protected:
        [[nodiscard]] static double _sincFilter(double x);

    public:
        LanczosFilter() : Filter(3.){};
        [[nodiscard]] double filter(double x) const override;
    };

    /**
     * \brief _lut Generate lookup table.
     * https://joelfilho.com/blog/2020/compile_time_lookup_tables_in_cpp/
     * 
     * \tparam Length Number of table elements.  
     * \param[in] f Functor called to generate each elements in the table.
     * \return An array of length Length with type deduced from Generator output.
     */
    template <size_t Length, typename Generator>
    static constexpr auto _lut(Generator&& f)
    {
        using content_type = decltype(f(size_t{0}));
        std::array<content_type, Length> arr{};
        for (size_t i = 0; i < Length; ++i) {
            arr[i] = f(i);
        }
        return arr;
    }

    /**
     * \brief _clip8_lut Clip lookup table.
     * 
     * \tparam Length Number of table elements.
     * \tparam min_val Value of the starting element.
     */
    template <size_t Length, intmax_t min_val>
    static inline constexpr auto _clip8_lut =
        _lut<Length>([](size_t n) -> uchar {
            intmax_t saturate_val = static_cast<intmax_t>(n) + min_val;
            if (saturate_val < 0) {
                return 0;
            }
            if (saturate_val > UCHAR_MAX) {
                return UCHAR_MAX;
            }
            return static_cast<uchar>(saturate_val);
        });

    /**
     * \brief _clip8 Optimized clip function.
     *
     * \param[in] in input value.
     *
     * \return Clipped value.
     */
    [[nodiscard]] static uchar _clip8(double in)
    {
        // Lookup table to speed up clip method.
        // Handles values from -640 to 639.
        const uchar* clip8_lookups = &_clip8_lut<1280, -640>[640];
        return clip8_lookups[static_cast<int>(in) >> precision_bits];
    }

    /**
     * \brief _roundUp Round function.
     * The output value will be cast to type T.
     *
     * \param[in] f Input value.
     *
     * \return Rounded value.
     */
    template <typename T>
    [[nodiscard]] static T _roundUp(double f)
    {
        return static_cast<T>(std::round(f));
    }

    /**
     * \brief _getPixelType Return the type of a matrix element.
     * If the matrix has multiple channels, the function returns the
     * type of the element without the channels.
     * For instance, if the type is CV_16SC3 the function return CV_16S.
     *
     * \param[in] img Input image.
     *
     * \return Matrix element type.
     */
    [[nodiscard]] static int _getPixelType(const cv::Mat& img)
    {
        return img.type() & CV_MAT_DEPTH_MASK;
    }

    /**
     * \brief _precomputeCoeffs Compute 1D interpolation coefficients.
     * If you have an image (or a 2D matrix), call the method twice to compute
     * the coefficients for row and column either.
     * The coefficients are computed for each element in range [0, out_size).
     *
     * \param[in] in_size Input size (e.g. image width or height).
     * \param[in] in0 Input starting index.
     * \param[in] in1 Input last index.
     * \param[in] out_size Output size.
     * \param[in] filterp Pointer to a Filter object.
     * \param[out] bounds Bounds vector. A bound is a pair of xmin and xmax.
     * \param[out] kk Coefficients vector. To each elements corresponds a number of
     * coefficients returned by the function.
     *
     * \return Size of the filter coefficients.
     */
    [[nodiscard]] static int _precomputeCoeffs(
        int in_size,
        double in0,
        double in1,
        int out_size,
        const std::shared_ptr<Filter>& filterp,
        std::vector<int>& bounds,
        std::vector<double>& kk);

    /**
     * \brief _normalizeCoeffs8bpc Normalize coefficients for 8 bit per pixel matrix.
     *
     * \param[in] prekk Filter coefficients.
     *
     * \return Filter coefficients normalized.
     */
    [[nodiscard]] static std::vector<double> _normalizeCoeffs8bpc(
        const std::vector<double>& prekk);

    /**
     * \brief _resampleHorizontal Apply resample along the horizontal axis.
     * It calls the _resampleHorizontal with the correct pixel type using
     * the value returned by cv::Mat::type().
     *
     * \param[in, out] im_out Output resized matrix.
     *                        The matrix has to be previously initialized with right size.
     * \param[in] im_in Input matrix.
     * \param[in] offset Vertical offset (first used row in the source image).
     * \param[in] ksize Interpolation filter size.
     * \param[in] bounds Interpolation filter bounds (value of the min and max column
     *                   to be considered by the filter).
     * \param[in] prekk Interpolation filter coefficients.
     */
    static void _resampleHorizontal(cv::Mat& im_out,
                                    const cv::Mat& im_in,
                                    int offset,
                                    int ksize,
                                    const std::vector<int>& bounds,
                                    const std::vector<double>& prekk);

    /**
     * \brief _resampleVertical Apply resample along the vertical axis.
     * It calls the _resampleVertical with the correct pixel type using
     * the value returned by cv::Mat::type().
     *
     * \param[in, out] im_out Output resized matrix.
     *                        The matrix has to be previously initialized with right size.
     * \param[in] im_in Input matrix.
     * \param[in] ksize Interpolation filter size.
     * \param[in] bounds Interpolation filter bounds (value of the min and max row
     *                   to be considered by the filter).
     * \param[in] prekk Interpolation filter coefficients.
     */
    static void _resampleVertical(cv::Mat& im_out,
                                  const cv::Mat& im_in,
                                  int ksize,
                                  const std::vector<int>& bounds,
                                  const std::vector<double>& prekk);

    typedef std::vector<double> (*preprocessCoefficientsFn)(
        const std::vector<double>&);

    template <typename T>
    using outMapFn = T (*)(double);

    /**
     * \brief _resampleHorizontal Apply resample along the horizontal axis.
     *
     * \param[in, out] im_out Output resized matrix.
     *                       The matrix has to be previously initialized with right size.
     * \param[in] im_in Input matrix.
     * \param[in] offset Vertical offset (first used row in the source image).
     * \param[in] ksize Interpolation filter size.
     * \param[in] bounds Interpolation filter bounds (index of min and max pixel
     *                   to be considered by the filter).
     * \param[in] prekk Interpolation filter coefficients.
     * \param[in] preprocessCoefficients Function used to process the filter coefficients.
     * \param[in] init_buffer Initial value of pixel buffer (default: 0.0).
     * \param[in] outMap Function used to convert the value of the pixel after
     *                   the interpolation into the output pixel.
     */
    template <typename T>
    static void _resampleHorizontal(
        cv::Mat& im_out,
        const cv::Mat& im_in,
        int offset,
        int ksize,
        const std::vector<int>& bounds,
        const std::vector<double>& prekk,
        preprocessCoefficientsFn preprocessCoefficients = nullptr,
        double init_buffer = 0.,
        outMapFn<T> outMap = nullptr);

    /**
     * \brief _resample Resize a matrix using the specified interpolation method.
     *
     * \param[in] im_in Input matrix.
     * \param[in] x_size Desidered output width.
     * \param[in] y_size Desidered output height.
     * \param[in] filter_p Pointer to the interpolation filter.
     * \param[in] rect Input region that has to be resized.
     *                 Region is defined as a vector of 4 point x0,y0,x1,y1.
     *
     * \return Resized matrix. The type of the matrix will be the same of im_in.
     */
    [[nodiscard]] static cv::Mat _resample(
        const cv::Mat& im_in,
        int x_size,
        int y_size,
        const std::shared_ptr<Filter>& filter_p,
        const cv::Vec4f& rect);

    /**
     * \brief _nearestResample Resize a matrix using nearest neighbor interpolation.
     *
     * \param[in] im_in Input matrix.
     * \param[in] x_size Desidered output width.
     * \param[in] y_size Desidered output height.
     * \param[in] rect Input region that has to be resized.
     *                 Region is defined as a vector of 4 point x0,y0,x1,y1.
     *
     * \return Resized matrix. The type of the matrix will be the same of im_in.
     * \throws std::runtime_error If the input matrix type is not supported.
     */
    [[nodiscard]] static cv::Mat _nearestResample(const cv::Mat& im_in,
                                                  int x_size,
                                                  int y_size,
                                                  const cv::Vec4f& rect);

public:
    /**
     * \brief InterpolationMethods Interpolation methods.
     * See https://pillow.readthedocs.io/en/stable/handbook/concepts.html#concept-filters.
     */
    enum InterpolationMethods {
        INTERPOLATION_NEAREST = 0,
        INTERPOLATION_BOX = 1,
        INTERPOLATION_BILINEAR = 2,
        INTERPOLATION_HAMMING = 3,
        INTERPOLATION_BICUBIC = 4,
        INTERPOLATION_LANCZOS = 5,
    };

    /**
     * \brief resize Porting of Pillow resize method.
     *
     * \param[in] src Input matrix that has to be processed.
     * \param[in] out_size Output matrix size.
     * \param[in] filter Interpolation method code, see InterpolationMethods.
     * \param[in] box Input roi. Only the elements inside the box will be resized.
     *
     * \return Resized matrix.
     * \throw std::runtime_error In case the box is invalid, the interpolation filter
     *        or the input matrix type are not supported.
     */
    [[nodiscard]] static cv::Mat resize(const cv::Mat& src,
                                        cv::Size out_size,
                                        int filter,
                                        const cv::Rect2f& box);

    /**
     * \brief resize Porting of Pillow resize method.
     *
     * \param[in] src Input matrix that has to be processed.
     * \param[in] out_size Output matrix size.
     * \param[in] filter Interpolation method code, see interpolation enum.
     *
     * \return Resized matrix.
     * \throw std::runtime_error In case the box is invalid, the interpolation filter
     *        or the input matrix type are not supported.
     */
    [[nodiscard]] static cv::Mat resize(const cv::Mat& src,
                                        cv::Size out_size,
                                        int filter);
};

template <typename T>
void PillowResize::_resampleHorizontal(
    cv::Mat& im_out,
    const cv::Mat& im_in,
    int offset,
    int ksize,
    const std::vector<int>& bounds,
    const std::vector<double>& prekk,
    preprocessCoefficientsFn preprocessCoefficients,
    double init_buffer,
    outMapFn<T> outMap)
{
    std::vector<double> kk(prekk.begin(), prekk.end());
    // Preprocess coefficients if needed.
    if (preprocessCoefficients != nullptr) {
        kk = preprocessCoefficients(kk);
    }

    for (size_t yy = 0; yy < im_out.size().height; ++yy) {
        for (size_t xx = 0; xx < im_out.size().width; ++xx) {
            int xmin = bounds[xx * 2 + 0];
            int xmax = bounds[xx * 2 + 1];
            double* k = &kk[xx * ksize];
            for (size_t c = 0; c < im_in.channels(); ++c) {
                double ss = init_buffer;
                for (size_t x = 0; x < xmax; ++x) {
                    ss += im_in.ptr<T>(yy + offset, x + xmin)[c] * k[x];
                }
                im_out.ptr<T>(yy, xx)[c] =
                    (outMap == nullptr ? ss : outMap(ss));
            }
        }
    }
}

#endif