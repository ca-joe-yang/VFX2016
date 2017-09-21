#include "stitch.h"

Mat computeTransformationMatrix(vector<Match>& matches) 
{
  if (matches.size() == 0)
    return (Mat_<double>(2,3) << 1, 0, 0, 0, 1, 0);
  double m1 = 0, m2 = 0;
  // RANSAC
  int kTimes = 50000;
  int nSamples = 3;
  double threshold = 50;
  int countMax = 0;
  srand(time(0));
  for (int k = 0; k < kTimes; ++k) {
    double m1Guess = 0, m2Guess = 0;
    random_shuffle(matches.begin(), matches.end(), myrandom);
    int i = 0;
    // Choose random n samples
    // Compute M
    for (; i < nSamples; ++i) {
      int diffx = -(matches[i].first.x - matches[i].second.x);
      int diffy = -(matches[i].first.y - matches[i].second.y);
      m1Guess += diffx;
      m2Guess += diffy;
      //cout << diffx << " " << diffy << endl;
    }
    m1Guess /= double(nSamples);
    m2Guess /= double(nSamples);
    //cout << m1Guess << " " << m2Guess << endl;
    int count = 0;
    for (; i < matches.size(); ++i) {
      int diffx = m1Guess + matches[i].first.x - matches[i].second.x;
      int diffy = m2Guess + matches[i].first.y - matches[i].second.y;
      double err = pow(diffx,2) + pow(diffy,2);
      if (err < threshold) count ++; 
    }
    if (count > countMax) {
      m1 = m1Guess;
      m2 = m2Guess;
      countMax = count;
    }
  }
  Mat mat = (Mat_<double>(2,3) << 1, 0, int(m1), 0, 1, int(m2));
  return mat;
}

void MyImage::showFeatures() const
{
  Mat img_features = _img.clone();
  for (int i = 0; i < _features.size(); ++i) { 
    Point2f pt_start = _features[i].pt;
    double length = _features[i]._scale;
    Scalar color = _features[i]._color;
    //myDrawCross(img_features, pt_start, 10, color);
    circle(img_features, pt_start, length, color, 1, 8);
    Point2f pt_end;
    double angle = _features[i]._angle;
    pt_end.x = pt_start.x + length*cos( angle * M_PI / 180.0 );
    pt_end.y = pt_start.y + length*sin( angle * M_PI / 180.0 );
    line(img_features, pt_start, pt_end, color, 1, 8);
  }
  string window = "features_" + _name;
  namedWindow(window, CV_WINDOW_AUTOSIZE); 
  imshow(window, img_features);
}

int MyImage::matchFeatures(MyImage* another_img, Mat& mat)
{
  cout << "Matching " << another_img->_name << " to " << _name << endl;
  vector<Match> matches;

  int dim = 1;
  for (int i = 0; i < _features.size(); ++i) {
    MyFeature feature1 = _features[i];
    vector<double> err;
    if (i == 0) dim = feature1._neighbor.rows*feature1._neighbor.cols;
    for (int j = 0; j < another_img->_features.size(); ++j) {
      MyFeature feature2 = another_img->_features[j];
      Mat diff;
      cv::subtract(feature1._neighbor, feature2._neighbor, diff);
      Mat diff2;
      cv::pow(diff, 2.0, diff2);
      double e = cv::sum(diff2)[0];
      //e = std::sqrt(e);
      err.push_back(e);
    }
    int minJ = min_element(err.begin(), err.end()) - err.begin();
    double min1 = err[minJ];
    err.erase(err.begin() + minJ);
    double min2 = *min_element(err.begin(), err.end());
    if (min1/min2 >= 0.8) continue;
    //if (min1 >= dim*0.0005) continue;
    //cout << min1 << endl;
    MyFeature feature2 = another_img->_features[minJ];
    _features[i]._color = Scalar(255, 100, 0);
    another_img->_features[minJ]._color = Scalar(100, 0, 100);
    Match match(feature1.pt, feature2.pt);
    matches.push_back(match);
  }
  cout << matches.size() << " matches found" << endl;
 
  mat = computeTransformationMatrix(matches).clone();
  return matches.size();
}

/*
void MyCanvas::ordering()
{
  Mat relation = Mat::zeros(_imgs.size(), _imgs.size(), CV_32S);
  for (int i = 0; i < num(); ++i) {
    for (int j = 0; j < num(); ++j) {
      if (i != j) {
        Mat mat;
        int num = _imgs[i].matchFeatures(&_imgs[j], mat);
        relation.at<int>(j,i) = -num;
      }
    }
  }
  for (int i = 0; i < num(); ++i) {
    for (int j = 0; j < num(); ++j) {
      cout << i << " " << j << " " << dijk(i, j, relation) << endl;
    }
  }
}
*/

void MyCanvas::matching(int& y_dev) 
{
  if (num() <= 1) return;
  Mat test(2, 3, CV_64F);
  _imgs[1].matchFeatures(&_imgs[0], test);
  int m1 = test.at<double>(2);
  if (m1 < 0) {
    std::reverse(_imgs.begin(), _imgs.end());
  }
 
  vector<Mat> transformation_mats;
  int left(0), right(0), bottom(0), top(0);
  for (int i = 0; i < num(); ++i) {
    Mat mat(2, 3, CV_64F);
    if (i == 0) 
      mat = (Mat_<double>(2,3) << 1, 0, 0, 0, 1, 0);
    else
      getImg(i)->matchFeatures(&_imgs[i-1], mat);
    Mat temp = (Mat_<double>(1,3) << 0, 0, 1);
    vconcat(mat, temp, mat);
    if (i >= 1) 
      mat = mat*transformation_mats[i-1];    
    transformation_mats.push_back(mat);
    int m1 = mat.at<double>(2);
    _imgs[i]._leftBorder += m1;
    _imgs[i]._rightBorder += m1;
    int m2 = mat.at<double>(5);
    if (m1 > 0 && m1 > right) 
      right = m1;
    else if (m1 < 0 && m1 < left)
      left = -1*m1;
    if (m2 > 0 && m2 > bottom)
      bottom = m2;
    else if (m2 < 0 && m2 < top)
      top = -1*m2;
  }
  y_dev = transformation_mats[num()-1].at<double>(5);

  for (int i = 0; i < num(); ++i) {
    Mat mat = transformation_mats[i];
    _imgs[i]._leftBorder += left;
    _imgs[i]._rightBorder += left;
    copyMakeBorder(getImg(i)->_img, getImg(i)->_img, top, bottom, left, right, BORDER_CONSTANT);
    cv::warpAffine(_imgs[i]._img, _imgs[i]._img, mat(Rect(0,0,3,2)), _imgs[i]._img.size());
  }

  paranoma = Mat::zeros(_imgs[0].size(), CV_8UC3);
}

void MyCanvas::drifting(int y_dev)
{
  double warpY = double(y_dev)/double(paranoma.cols);
  Mat drift_mat = (Mat_<double>(2,3) << 1, 0, 0, -warpY, 1, 0);
  cv::warpAffine(paranoma, paranoma, drift_mat, paranoma.size());
}
