#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <utility>
#include <algorithm>
#include <ctime>
#include <cassert>
#include <locale>
#include <libgen.h>

#include "util.h"

using namespace std;
using namespace cv;

#define DEFAULT_FOCAL_LENGTH 500.0

typedef pair<Point2i, Point2i> Match;

class MyCanvas;
class MyImage;
class MyFeature;

Mat getGaussianKernel2D(Size s, double sigma, int x = 0, int y = 0);

class MyFeature
{
  public:
    MyFeature(double x, double y, double s_ = 1, int o_ = 0)
      : pt(x,y), s(s_), o(o_) 
    {
      _angle = 0.0;
      _scale = 1.0;
      _color = Scalar(0, 255, 255);
    }
    ~MyFeature() {}

    // Friend classes
    friend class MyImage;

    // Set functions
    void setAngle(const double angle) { _angle = angle; }
    void setScale(const double scale) { _scale = scale; }

    // public data member
    Point2f pt;
    // For SIFT
    int     m;
    int     n;
    int     o;
    int     s;
  private:
    // private data member
    double _scale;
    double _angle;
    Scalar _color;
    Mat    _neighbor;
};

class MyImage
{
  public:
    MyImage() {}
    MyImage(const string name) : _name(name) {}
    ~MyImage() {}
    
    // Friend Classes
    friend class MyCanvas;

    // Public Member Functions
    string getName() const { return _name; }
    cv::Size size() const { return _img.size(); }
    
    void Moravec_Detector(const int local, const int threshold);
    void Harris_Detector(const int local, const int threshold = 1e4, const double k = 0.005);
    void SIFT_Detector();
    void showFeatures() const;
    int matchFeatures(MyImage* another_img, Mat& mat);

    // Warping Functions
    void warpCylindrical(const double r);
    void warpTranslation(const int moveX, const int moveY)
    {
      Mat mat = (Mat_<int>(2,3) << 1, 0, moveX, 0, 1, moveY);
      cv::warpAffine(_img, _img, mat, _img.size());
    }

    void feathering(); 

    bool load(const string path) 
    {
      string dir, base;
      parseFilePath(path, dir, base);
      _name = base;
      _dir = dir;
      _img = imread(path, CV_LOAD_IMAGE_UNCHANGED);
      if (_img.empty()) {
        cerr << "[ERROR] Image cannot be loaded..!!" << endl;
        return false;
      }
      computeGrayScale();
      
      _leftBorder = 0;
      _rightBorder = _img.cols-1;

      _mask = Mat(_img.size(), CV_8U);
      _mask.setTo(Scalar(255,255,255));
      
      _border = Mat::zeros(_img.size(), CV_8U);
      int borderXNum = 5;
      int borderYNum = 10;
      for (int y = 0; y < _img.rows; ++y) {
        for (int x = 0; x < borderXNum; ++x)
          _border.at<uchar>(y,x) = 255;
        for (int x = _img.cols-1; x > _img.cols-1-borderXNum; --x)
          _border.at<uchar>(y,x) = 255;
      }
      for (int x = 0; x < _img.cols; ++x) {
        for (int y = 0; y < borderYNum; ++y)
          _border.at<uchar>(y,x) = 255;
        for (int y = _img.rows-1; y > _img.rows-1-borderYNum; --y)
          _border.at<uchar>(y,x) = 255;
      }
      return true;
    }
    void showBorder() const 
    {
      namedWindow("border_"+_name, CV_WINDOW_AUTOSIZE);
      imshow("border_"+_name, _border);
    }
    void showImg() const 
    {
      namedWindow(_name, CV_WINDOW_AUTOSIZE); 
      imshow(_name, _img); 
    }
    void computeGrayScale() 
    {
      if (_img.type() == CV_8U)
        _imgGray = _img.clone();
      else {
        cvtColor(_img, _imgGray, CV_RGB2GRAY);
        _imgGray.convertTo(_imgGray, CV_8U);
      }
    }

  private:
    string            _name;
    string            _dir;
    string            _format;
    Mat               _img;
    int               _leftBorder;
    int               _rightBorder;
    Mat               _mask;
    Mat               _border;
    Mat               _imgGray;
    vector<MyFeature> _features;
    double            _focalLength;

    // For feathering
    Mat               _imgFeather;
    double            _alpha;
    double            _beta;
    int               _leftFeatherBorder;
    int               _rightFeatherBorder;
};

class MyCanvas
{
  public:
    MyCanvas(string directory, int num = -1) 
    {
      if (directory[directory.size() - 1] != '/') directory = directory + "/";
      vector<string> files;
      getImagesInDir(directory, files);
  
      if (num == -1) num = files.size();
      for (int i = 0; i < num; ++i) {
        string path = directory + files[i];
        MyImage img;
        img.load(path);
        _imgs.push_back(img);
      }

      string f_file = directory+"pano.txt";
      ifstream focalList(f_file.c_str());
      
      if (focalList == false) {
        cerr << "[WARNING] Cannot find file \"" << f_file  << "\"!!" << endl;
        cerr << "[WARNING] Using DEFAULT_FOCAL_LENGTH=500" << endl;
        double f = DEFAULT_FOCAL_LENGTH;
        for (int i = 0; i < _imgs.size(); ++i) 
          _imgs[i]._focalLength = f;
        _cylR = f;
        return;
      }
   
      string line;
      int count = 0;
      while(getline(focalList, line)) {
        remove(line.begin(), line.end(), ' ');
        remove(line.begin(), line.end(), '\n');
        if (line.size() == 0) continue;
        double f = std::stod(line)*5;
        _imgs[count]._focalLength = f;
        cout << f << endl;
        if (count == 0 || f < _cylR)
          _cylR = f;
        count++;
      }
    }
    ~MyCanvas() {}

    int num() const { return _imgs.size(); }
    MyImage* getImg(const int i) { return &_imgs[i]; }

    bool detecting(const string m)
    {
      for (int i = 0; i < _imgs.size(); ++i) {
        MyImage* img = &(_imgs[i]);
        cout << "Processing " << img->_name << endl;
        
        cout << "Cylindrical Warping..." << endl;
        img->warpCylindrical(_cylR);
        
        string method = "";
        toLowerCase(m, method);
        
        cout << "Extracing features..." << endl;
        if (method == "moravec")
          img->Moravec_Detector(10, 1e3);
        else if (method == "harris")
          img->Harris_Detector(10, 1e5);
        else if (method == "sift")
          img->SIFT_Detector();
        else {
          cerr << "[ERROR] Feature detecting method not defined" << endl;
          return false;
        }
        img->showFeatures();
        cout << endl;
      }
      return true;

    }
    void ordering();
    void matching(int& y_dev);
    bool blending(const string m)
    { 
      locale loc;
      string method = "";
      for (int i = 0; i < m.length(); ++i)
        method += tolower(m[i], loc);
      if (method == "average")
        averageBlending();
      else if (method == "feather")
        featherBlending();
      else {
        cerr << "[ERROR] " << endl;
        return false;
      }
      return true;
    }

    // blend.cpp
    void drifting(int y_dev);
    void featherBlending();
    void averageBlending();
    void weakCropping();
    void strictCropping();
    
    void show() const
    {
      namedWindow("result", CV_WINDOW_AUTOSIZE); 
      imshow("result", paranoma); 
    }
    void save(const string output) const
    {
      imwrite(output, paranoma);
    }

    Mat             paranoma;
  private:
    vector<MyImage> _imgs;
    double          _cylR;
};

