# NTU CSIE DigiVFX 2016
- Author
  - B02901072 楊喬諳
  - B02901065 李洺曦

## HW1 HDR Imaging

### Description
- 我們整個程式都是使用MATLAB所撰寫，使用到的程式庫只包含MATLAB所內建的hdrwrite()函式用以儲存hdr file，以及教授所介紹的Debevec’s gsolve.m

### File Format
  - myHDR.m
    - 使用 hdr = myHDR(ImageDir, format, algorithm, arg);
      - ImageDir 為圖片檔資料夾位置
      - format 為圖片格式，如’.jpg’, ‘.png’, ‘.tif’等
      - algorithm可為1或2
        - 若為1，則使用Debevec’s method
          - arg 為exposure time的vector
        - 若為2，則使用Mitsunaga and Nayar’s method
    - hdr 回傳一組 圖片長x圖片寬x3的 hdr radiance matrix
    - 會在程式碼中呼叫到gsolve.m gsolve2.m myMTB.m函式
  - gsolve.m
    - Debevec’s method 同講義中所介紹的一樣
  - gsolve2.m
    - 使用 g = gsolve2.m(Z,M);
    - Mitsunaga and Nayar’s method 假設為M次多項式並解之
    - reference: <High Dynamic Range Imaging> p.175開始
  - myMTB.m
    - 使用 X = myMTB(X);
    - 輸入一個 圖片數x圖片長x圖片寬x3 的pixel value matrix
    - 回傳一個 已aligned的的pixel value matrix

  - myTonemap.m
    - 我們使用的是講義上的Photographic Tone Reproduction
    - 使用方式有兩種
    1. rgb = myTonemap(hdr, ’global’, key, Lwhite);
      - hdr 為一組 圖片長x圖片寬x3的 hdr radiance matrix
        - 可以是myHDR所回傳的值，或是內建的hdrread所回傳的值
      - rgb 為tonemap出來的結果 可用imshow(rgb)加以顯示
      - key 為下圖中所使用的”a”
      - Lwhite 為下圖中所使用”Lwhite”，可填可不填
        - default值為整張圖片中原有最高的Lm值

  - Image alignment -> myNTB.m
    - 我們使用的是講義中所介紹的MTB alignment technique
    - 先找到一張圖的中位數median，將threshold設為此，以上為1，以下為0
    - 然後再進行八個方向（加上原本不動共九個）可能進行微調，得出變化最小的
    - 先將圖片縮小到1/32，九個方向進行比較，然後是1/16,1/8...以加快速度
    - 實作中，我們已曝光量中間程度那張（假設有九張，就是用第五張）作為標準圖
    - 將其他影像依據標準圖做移動

  - High Dynamic Range -> myHDR.m

  - Debevic’s method -> gsolve.m

  - Mitsunaga and Nayar’s method -> gsolve2.m

  - Ghost Removal -> myGhost.m & myImageVariance.m
    - 這部分我們有嘗試做做看，但只有做到Image的Weighted Variance這一步而已


  - Tone Mapping -> myTonemap.m


## HW2 Image Stitching
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
$ cd ./build
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

