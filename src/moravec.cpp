#include "stitch.h"

void MyImage::Moravec_Detector(const int local, const int threshold) 
{
  assert(local>=0);
  _features.clear();

  // Compute E
  Mat Emap = Mat::zeros(size(), CV_64F);
  int u[4] = {1, 1, 0, -1};
  int v[4] = {0, 1, 1, 1};
  for (int y = 0; y < _img.rows; ++y) {
    for (int x = 0; x < _img.cols; ++x) {
      if (_mask.at<uchar>(y,x) == 0) continue;
      if (_border.at<uchar>(y,x) == 255) continue;
      double Emin;
      for (int k = 0; k < 4; ++k) {
        int sum = 0;
        for (int dx = -1; dx <= 1; ++dx) {
          for (int dy = -1; dy <= 1; ++dy) {
            int xdx = x+dx;
            int ydy = y+dy;
            int value = 0;
            if (xdx >= 0 && xdx < _img.cols && ydy >= 0 && ydy < _img.rows)
              value = _imgGray.at<uchar>(ydy, xdx);
            int xudx = x+u[k]+dx;
            int yvdy = y+v[k]+dy;
            int valueUV = 0;
            if (xudx >= 0 && xudx < _img.cols && yvdy >= 0 && yvdy < _img.rows)
              valueUV = _imgGray.at<uchar>(yvdy, xudx);
            double diff2 = std::pow(valueUV-value, 2);
            sum += diff2;
          }
        }
        if (k == 0 || sum < Emin)
          Emin = sum;
      }
      if (Emin < threshold)
        Emin = 0;
      Emap.at<double>(y,x) = Emin;
    }
  }
  
  // Find local maximum
  // Compute descriptor
  int block = 2;
  for (int y = 1; y < _img.rows-1; ++y) {
    for (int x = 1; x < _img.cols-1; ++x) {
      bool localMax = true;
      double E0 = Emap.at<double>(y,x);
      if (E0 == 0) continue;
      for (int i = -1*local; i <= local; ++i) {
        for (int j = -1*local; j <= local; ++j) {
          double Evalue = 0;
          if (x+i >= 0 && x+i < _img.cols && y+j >=0 && y+j < _img.rows)
            Evalue = Emap.at<double>(y+j, x+i);
          if (E0 < Evalue)
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
              value = _imgGray.at<uchar>(Point(x+i, y+j));
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

