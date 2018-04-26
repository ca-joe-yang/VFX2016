# NTU CSIE DigiVFX 2016
## HW2 Image Stitching
- Author
  - B02901072 楊喬諳
  - B02901065 李洺曦

### g++ version
- 4.2.1
### Cmake version 
- 2.8
### OpenCV version
- 3.1.0

### Files included:
- HW2/
  - README.txt
  - CmakeList.txt
  - src/
    - main.cpp
    - feature.cpp
    - harris.cpp
    - moravec.cpp
    - sift.cpp
    - blend.cpp
    - warp.cpp
    - util.cpp
    - tm_usage.cpp
  - include/
    - stitch.h
    - util.h
    - tm_usage.h
  - input/
    - [where_mostly_the_input_images_are]
  - output/
    - [where_mostly_the_result_paranoma_are_saved]
  - build/
    - main
    - Makefile
    - [and_some_other_cmake_stuffs]

### How to compile:

```bash
$ mkdir build
$ cd build
$ cmake ..
$ make
(which should generate a executable file ./main in the ./build/ directory)
$ ./main <INPUT_IMAGES_DIR> <OUTPUT_PATH> [DETECT_METHOD] [BLEND_METHOD]
```

- WARNING: Since we did not put much effort in parsing the arguments, the arguments must be put in this order and this order only, or else the program will return error message(MAYBE) and crash(CERTAINLY).

---
<INPUT_IMAGES_DIR> --- MUST
For example: 
  ../input/parrington
  ../input/parrington/

WARNING: The images in the directory MUST be in order, whether it's 
         left to right or right to left.

WARNING: Inside the directory must contain the txt file "pano.txt"
         which specify the estimated focal lengths of each images.
         The format is quite simple. Please check the report for 
         details.
         If not exists, the program will assign a default one with 
         focal length equals to 500.

---
<OUTPUT_PATH> --- MUST
For example:
  ../output/result.jpg

---
[DETECT_METHOD] --- OPTIONAL
Only accept three kinds of flags(can either be in upper case or lower case)
  moravec
  harris
  sift

representing the different feature detection methods separately, 
  Moravec method
  Harris method
  SIFT

If not specified, using Harris method as default.

---
[BLEND_METHOD] --- OPTIONAL
Only accept two kinds of flags(can either be in upper case or lower case)
  average
  feather

representing the different blending methods separately, 
  Average Blending
  Feather Blending

If not specified, using feather blending as default.

---
Overall Example
  CORRECT
  $ ./main ../input/parrington ./output/result.jpg HarrIS feATher
  $ ./main ../input/parrington/ ./output/aaaaa.jpg sift average
  $ ./main ../input/parrington ./output/result.jpg MORAVEC feATher
  
  ERROR
  $ ./main [DIR_THAT_DOESNT_EXIST] ./output/result.jpg MORAVEC feATher
  $ ./main ../input/parrington ./output/result.jpg hlakhflkSD feATher

