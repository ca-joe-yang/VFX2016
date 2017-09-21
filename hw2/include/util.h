#include <opencv2/imgcodecs.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <locale>
#include <libgen.h>

#include "dirent.h"

using namespace cv;
using namespace std;

void parseFilePath(string path, string& dir, string& base);
void toLowerCase(const string& str, string& lower);

int myrandom (int i); 
bool isColorRGB(const Vec3b v3, const uchar r, const uchar g, const uchar b);

void getImagesInDir(const string dir, vector<string>& files);

