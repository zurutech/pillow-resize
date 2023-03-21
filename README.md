# Pillow Resize

## Table of Contents 

- [Description](#description)
- [Compilation](#compilation)
- [Installation](#installation)
- [Usage](#usage)


## Description

`PillowResize` library is a C++ porting of the resize method from the [Pillow](https://github.com/python-pillow/Pillow) python library.

It is written in C++ using [OpenCV](https://opencv.org/) for matrix support

The main difference with respect to the [`resize`](https://docs.opencv.org/4.5.2/da/d54/group__imgproc__transform.html#ga47a974309e9102f5f08231edc7e7529d) method of OpenCV is the use of an anti aliasing filter, that is missing in OpenCV and could introduce some artifacts, in particular with strong down-sampling.

See also blog.


### Requirements

The only requirement is `OpenCV`, so install it from your package manager.


## Compilation

```bash
git clone --recurse https://github.com/zurutech/pillow-resize.git
cd pillow-resize
git submodule update --init --recursive
mkdir build && cd build
cmake ..
make
```

### Test
If you want to run also the tests, you need to install [`gtest`](https://github.com/google/googletest).

```bash
cmake -DBUILD_TESTS=ON ..
make
make test
```

The tests are based on a pixelwise comparison between the images resized using Pillow from Python and the images created with this library.

## Installation

To install `PillowResize` as a package ensure that you have `OpenCV` installed system-wide, then simply compile and install.

```bash
mkdir build && cd build
cmake ..
sudo make install
```

### Uninstall

To uninstall `PillowResize` and all the things it brings with it simply run

```bash
cd build
sudo make uninstall
```


## Usage

The library uses `cmake`, hence this library can be added as a subdirectory and built together with your project or use the installed package as a normal CMake package.

### Installed package

```cmake
find_package(PillowResize REQUIRED)
```

### Subdirectory

```cmake
add_subdirectory(path-to-subdir)
```

### Link

```cmake
target_link_libraries(your-target PUBLIC PillowResize)
```

The library defines also the variable `PILLOWRESIZE_LIBS`, so you can replace `PillowResize` in the command above with `"${PILLOWRESIZE_LIBS}"`.
