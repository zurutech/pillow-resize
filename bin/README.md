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
  --width UINT                Output image width. (default: 0).
                              If you want to set the output image size you need to pass both width and height.
  --height UINT               Output image height. (default: 0)
  --fx FLOAT                  Scale factor along the horizontal axis. (default: 0).
                              Output image size will automatically computed as image_width*fx.
                              If you want to set the scale you need to pass both fx and fy.
  --fy FLOAT                  Scale factor along the vertical axis. (default: 0)
                              Output image size will automatically computed as image_height*fy.
  -m,--method ENUM:value in {bicubic->3,bilinear->2,box->4,hamming->5,lanczos->1,nearest->0} OR {3,2,4,5,1,0} REQUIRED
                              Interpolation method. 


```