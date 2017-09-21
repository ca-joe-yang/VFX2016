#include "stitch.h"

void MyImage::Harris_Detector(const int local, const int threshold,  const double k) 
{
  assert(local>=0);
  _features.clear();

  // Reduce Noise
  Mat img_smooth, mask_smooth;
  GaussianBlur(_imgGray, img_smooth, Size(0,0), 0.25, 0);
 
  // Compute Gradient Ix, Iy
  Mat gradX;
  Sobel(img_smooth, gradX, CV_64F, 1, 0, 1);
  Mat gradY;
  Sobel(img_smooth, gradY, CV_64F, 0, 1, 1);

  // Compute Sx2, Sy2, Sxy
  Mat gradX2 = gradX.mul(gradX);
  Mat gradY2 = gradY.mul(gradY);
  Mat gradXY = gradX.mul(gradY);
  Mat SX2, SY2, SXY;
  GaussianBlur(gradX2, SX2, Size(0,0), 0.5, 0);
  GaussianBlur(gradY2, SY2, Size(0,0), 0.5, 0);
  GaussianBlur(gradXY, SXY, Size(0,0), 0.5, 0);

  // Compute R
  Mat RMatrix = Mat::zeros(_img.size(), CV_64F);
  for (int y = 1; y < _img.rows-1; ++y) {
    for (int x = 1; x < _img.cols-1; ++x) {
      if (_mask.at<uchar>(y,x) == 0) continue;
      if (_border.at<uchar>(y,x) == 255) continue;
      Mat mat(2, 2, CV_64F);
      mat.at<double>(0,0) = SX2.at<double>(Point(x,y));
      mat.at<double>(0,1) = SXY.at<double>(Point(x,y));
      mat.at<double>(1,0) = SXY.at<double>(Point(x,y));
      mat.at<double>(1,1) = SY2.at<double>(Point(x,y));

      double det = determinant(mat);
      double tr = trace(mat)[0];
      double R = det - k*tr*tr;

      if (R > threshold)
        RMatrix.at<double>(Point(x,y)) = R;
    }
  }
  
  // Find local maximum
  // Compute descriptor
  int block = 2;
  for (int y = 1; y < _img.rows-1; ++y) {
    for (int x = 1; x < _img.cols-1; ++x) {
      bool localMax = true;
      double R0 = RMatrix.at<double>(Point(x,y));
      if (R0 == 0) continue;
      for (int i = -1*local; i <= local; ++i) {
        for (int j = -1*local; j <= local; ++j) {
          double Rvalue = 0;
          if (x+i >= 0 && x+i < _img.cols && y+j >=0 && y+j < _img.rows)
            Rvalue = RMatrix.at<double>(Point(x+i, y+j));
          if (R0 < Rvalue)
            localMax = false;
          if (!localMax) break;
        }
        if (!localMax) break;
      }

      if (localMax) {
        MyFeature feature(x,y);
        
        int count = 0;
        Mat neighbor(1, (2*block+1)*(2*block+1), CV_64F);
        for (int i = -1*block; i <= block; ++i) {
          for (int j = -1*block; j <= block; ++j) {
            uchar value = 0;
            if (x+i >= 0 && x+i < _img.cols && y+j >=0 && y+j < _img.rows)
              value = img_smooth.at<uchar>(Point(x+i, y+j));
            neighbor.at<double>(count++) = double(value);
          }
        }
        cv::normalize(neighbor, neighbor, 1, 0);
        feature._neighbor = neighbor;
        _features.push_back(feature);
      }
    }
  }
  cout << _features.size() << " features extracted" << endl;
}


