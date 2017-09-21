#include "stitch.h"

//#define SHOW_FEATHER

void MyImage::feathering() 
{
  _imgFeather = _img.clone();  
  for (int x = 0; x < _img.cols; ++x) {
    double weight = 1.0;
    if (x >= _leftBorder-1 && x <= _leftFeatherBorder)
      weight = _alpha*double(x-_leftBorder+1);
    else if (x >= _rightFeatherBorder-1 && x <= _rightBorder)
      weight = 1.0 + _beta*double(x-_rightFeatherBorder+1);
    else 
      continue;

    if (weight < 0.0) weight = 0.0;
    if (weight > 1.0) weight = 1.0;
    for (int y = 0; y < _img.rows; ++y) {
      Vec3b v3_feather;
      Vec3b v3 = _img.at<Vec3b>(y,x);
      for (int ch = 0; ch < 3; ++ch)
        v3_feather[ch] = weight*double(v3[ch]);
      _imgFeather.at<Vec3b>(y,x) = v3_feather;
    }
  }
    
  #ifdef SHOW_FEATHER
  namedWindow("feather_"+_name, CV_WINDOW_AUTOSIZE);
  imshow("feather_"+_name, _imgFeather);
  #endif
}


void MyCanvas::featherBlending()
{
  if(_imgs.size() == 0) return;

  vector<int> keycols;
  
  _imgs[0]._alpha = 1.0;
  _imgs[0]._leftFeatherBorder = _imgs[0]._leftBorder;
    
  _imgs[_imgs.size()-1]._beta = 1.0;
  _imgs[_imgs.size()-1]._rightFeatherBorder = _imgs[_imgs.size()-1]._rightBorder;
 
  for (int i = 1; i < _imgs.size(); ++i) {
    _imgs[i]._alpha = 1.0/double(_imgs[i-1]._rightBorder-_imgs[i]._leftBorder);
    _imgs[i-1]._beta = -1*_imgs[i]._alpha;
    _imgs[i]._leftFeatherBorder = _imgs[i-1]._rightBorder;
    _imgs[i-1]._rightFeatherBorder = _imgs[i]._leftBorder;
  }
  for (int i = 0; i < num(); ++i)
    _imgs[i].feathering();

  for (int x = 0; x < paranoma.cols; ++x) {
    for (int y = 0; y < paranoma.rows; ++y) {
      Vec3b v3_paranoma(0,0,0);
      for (int i = 0; i < num(); ++i) {  
        Vec3b v3 = _imgs[i]._imgFeather.at<Vec3b>(Point(x,y)); 
        for (int ch = 0; ch < 3; ++ch) 
          v3_paranoma[ch] += v3[ch];
      }
      paranoma.at<Vec3b>(y,x) = v3_paranoma;
    }
    
  }
}
void MyCanvas::averageBlending()
{
  for (int y = 0; y < paranoma.rows; ++y) {
    for (int x = 0; x < paranoma.cols; ++x) {
      Vec3b v3_paranoma;
      vector<Vec3b> v3_imgs;
      for (int i = 0; i < num(); ++i) {
        Vec3b v3 = _imgs[i]._img.at<Vec3b>(Point(x,y));
        if (v3[0] && v3[1] && v3[2])
          v3_imgs.push_back(v3);
      }
      for (int ch = 0; ch < 3; ++ch) {
        if (v3_imgs.size() == 0) {
          v3_paranoma[ch] = 0;
        } else {
          int sum = 0;
          for (int i = 0; i < v3_imgs.size(); ++i) {
            sum += v3_imgs[i][ch];
          }
          sum /= v3_imgs.size();
          v3_paranoma[ch] = int(sum);
        }
      }
      paranoma.at<Vec3b>(Point(x,y)) = v3_paranoma;
    }
  }
}

void MyCanvas::weakCropping()
{
  int top_y = 0;
  int bottom_y = paranoma.rows-1;
  int left_x = 0;
  int right_x = paranoma.cols-1;
  
  for (; top_y < paranoma.rows-1; ++top_y) {
    bool black = true;
    for (int x = 0; x < paranoma.cols; ++x) {
      Vec3b v3 = paranoma.at<Vec3b>(Point(x,top_y));
      if (!isColorRGB(v3,0,0,0)) {
        black = false;
        break;
      }
    }
    if (!black) break;
  }
  for (; bottom_y > 0; --bottom_y) {
    bool black = true; 
    for (int x = 0; x < paranoma.cols; ++x) {
      Vec3b v3 = paranoma.at<Vec3b>(Point(x,bottom_y));
      if (!isColorRGB(v3,0,0,0)) {
        black = false;
        break;
      }
    }
    if (!black) break;
  }
  for (; left_x < paranoma.cols; ++left_x) {
    bool black = true;
    for (int y = 0; y < paranoma.rows; ++y) {
      Vec3b v3 = paranoma.at<Vec3b>(Point(left_x,y));
      if (!isColorRGB(v3,0,0,0)) {
        black = false;
        break;
      }
    }
    if (!black) break;
  }
  for (; right_x > 0; --right_x) {
    bool black = true;
    for (int y = 0; y < paranoma.rows; ++y) {
      Vec3b v3 = paranoma.at<Vec3b>(Point(right_x,y));
      if (!isColorRGB(v3,0,0,0)) {
        black = false;
        break;
      }
    }
    if (!black) break;
  }
 
  paranoma = Mat(paranoma, Rect(left_x, top_y, right_x-left_x, bottom_y-top_y)).clone();
}

void MyCanvas::strictCropping()
{
  int top_y = 0;
  int bottom_y = paranoma.rows-1;
  
  for (; top_y < paranoma.rows-1; ++top_y) {
    bool black = false;
    int count = 0;
    for (int x = 0; x < paranoma.cols; ++x) {
      Vec3b v3 = paranoma.at<Vec3b>(Point(x,top_y));
      if (isColorRGB(v3,0,0,0)) {
        count ++;
      }
      if (count >= 50) {
        black = true;
        break;
      }
    }
    if (!black) break;
  }
  for (; bottom_y > 0; --bottom_y) {
    bool black = false;
    int count = 0;
    for (int x = 0; x < paranoma.cols; ++x) {
      Vec3b v3 = paranoma.at<Vec3b>(Point(x,bottom_y));
      if (isColorRGB(v3,0,0,0)) {
        count ++;
      }
      if (count >= 50) {
        black = true;
        break;
      }
    }
    if (!black) break;
  }

  paranoma = Mat(paranoma, Rect(0, top_y, paranoma.cols-1, bottom_y-top_y)).clone();
}
