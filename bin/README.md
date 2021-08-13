# PillowResize executable
`PillowResize` is shipped with an executable `resize`.

If you installed the package, you can find the executable in the installation folder in the directory `bin/PillowResize`.

## Resize

```bash
./bin/resize --help
```

```
Resize an image using Pillow resample
Usage: ./bin/resize [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  -i,--input TEXT REQUIRED    Input image path.
  -o,--output TEXT            Output image folder. 
                              If empty the output image will be saved in the input directory with the suffix '_resized'.
  --ow,--width UINT           Output image width. (default: 0).
                              If you want to set the output image size you need to pass both width and height.
  --oh,--height UINT          Output image height. (default: 0)
  --horizontal_scale_factor,--fx FLOAT
                              Scale factor along the horizontal axis. (default: 0).
                              Output image size will automatically computed as image_width*horizontal_scale_factor.
                              If you want to set the scale you need to pass both horizontal_scale_factor and vertical_scale_factor.
  --vertical_scale_factor,--fy FLOAT
                              Scale factor along the vertical axis. (default: 0)
                              Output image size will automatically computed as image_height*vertical_scale_factor.
  -m,--method ENUM REQUIRED   Interpolation method. 
                              Valid values in:
                              INTERPOLATION_NEAREST -> 0
                              INTERPOLATION_BOX -> 1
                              INTERPOLATION_BILINEAR -> 2
                              INTERPOLATION_HAMMING -> 3
                              INTERPOLATION_BICUBIC -> 4
                              INTERPOLATION_LANCZOS -> 5
```