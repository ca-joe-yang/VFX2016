#include "stitch.h"

Point2d cylindrical2Plain(const Point2d pt_cyl, double f, double r, int x0, int y0)
{
  double x_cyl = pt_cyl.x-x0;
  double y_cyl = pt_cyl.y-y0;
  double x_pln, y_pln;
  if (x_cyl == 0) {
    x_pln = 0;
    y_pln = y_cyl * f / r;
  } else {
    x_pln = f * x_cyl / sqrt(r*r - x_cyl*x_cyl);
    y_pln = x_pln * y_cyl / x_cyl;
  }
  return Point2d(x_pln+x0, y_pln+y0);
}

Point2d plain2Cylindrical(const Point2d pt_pln, double f, double r, int x0, int y0)
{
  double x_pln = pt_pln.x-x0;
  double y_pln = pt_pln.y-y0;
  double x_cyl, y_cyl;
  if (x_pln == 0) {
    x_cyl = 0;
    y_cyl = y_pln * r / f;
  } else {
    x_cyl = r * x_pln / sqrt(x_pln*x_pln + f*f);
    y_cyl = x_cyl * y_pln / x_pln;
  }
  return Point2d(x_cyl+x0, y_cyl+y0);
}

void MyImage::warpCylindrical(const double r)
{
  const double f = _focalLength;

  Mat img_cyl = Mat::zeros(_img.size(), CV_8UC3);
  Mat border_cyl = Mat::zeros(_img.size(), CV_8U);
 
  Point2i center(_img.cols/2, _img.rows/2);
  Point2i leftBorderPt = plain2Cylindrical(Point2f(0,0), f, r, center.x, center.y);
  _leftBorder = leftBorderPt.x;
  Point2i rightBorderPt = plain2Cylindrical(Point2f(_img.cols-1,0), f, r, center.x, center.y);
  _rightBorder = rightBorderPt.x;
 
  for(int y = 0; y < _img.rows; y++) {
    for(int x = 0; x < _img.cols; x++) {
      Point2f pt_pln = cylindrical2Plain(Point2f(x,y), f, r, _img.cols/2, _img.rows/2);

      Point2i top_left((int)pt_pln.x,(int)pt_pln.y); 

      //make sure the point is actually inside the original image
      if(top_left.x < 0 || top_left.x > _img.cols-2 || 
         top_left.y < 0 || top_left.y > _img.rows-2) {
        _mask.at<uchar>(y,x) = 0;
        continue;
      }
       //bilinear interpolation
      double dx = pt_pln.x-top_left.x;
      double dy = pt_pln.y-top_left.y;

      double weight_tl = (1.0 - dx) * (1.0 - dy);
      double weight_tr = (dx)       * (1.0 - dy);
      double weight_bl = (1.0 - dx) * (dy);
      double weight_br = (dx)       * (dy);

      Vec3b value;
      Vec3b vec_tl = _img.at<Vec3b>(top_left.y, top_left.x);
      Vec3b vec_tr = _img.at<Vec3b>(top_left.y, top_left.x+1);
      Vec3b vec_bl = _img.at<Vec3b>(top_left.y+1, top_left.x);
      Vec3b vec_br = _img.at<Vec3b>(top_left.y+1, top_left.x+1);

      for (int ch = 0; ch < 3; ++ch) 
        value[ch] = weight_tl*vec_tl[ch] + weight_tr*vec_tr[ch] +
                    weight_bl*vec_bl[ch] + weight_br*vec_br[ch];

      img_cyl.at<Vec3b>(y,x) = value;
    }
  }
  _img = img_cyl.clone();
  computeGrayScale();

  for(int y = 0; y < _img.rows; y++) {
    for(int x = 0; x < _img.cols; x++) {
      if (int(_border.at<uchar>(y,x)) == 255) {
        Point2i border_pt_cyl = plain2Cylindrical(Point2f(x,y), f, r, _img.cols/2, _img.rows/2);
        for (int i = -1; i < 1; ++i) {
          for (int j = -1; j < 1; ++j) {
            if (border_pt_cyl.x+i < 0 || border_pt_cyl.x+i >= _img.cols ||
                border_pt_cyl.y+j < 0 || border_pt_cyl.y+j >= _img.rows)
              continue;
            border_cyl.at<uchar>(border_pt_cyl.y+j, border_pt_cyl.x+i) = 255;
          }
        }
      }
    }
  }
  _border = border_cyl.clone();

  return;
}

