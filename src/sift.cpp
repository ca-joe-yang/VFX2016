#include "stitch.h"

void computeDoG(const Mat& G1, const Mat& G2, Mat& DoG)
{
  assert(G1.size() == G2.size());
  assert(G1.type() == CV_8U);
  assert(G2.type() == CV_8U);

  DoG = Mat(G1.size(), CV_8S);
  for (int y = 0; y < DoG.rows; ++y) {
    for (int x = 0; x < DoG.cols; ++x) {
      int u1 = G1.at<uchar>(y, x);
      int u2 = G2.at<uchar>(y, x);
      char u3 = DoG.at<char>(y, x);
      u3 = (u1-u2);
      DoG.at<char>(y, x) = u3;
    }
  }
}

void MyImage::SIFT_Detector()
{
  vector< vector<Mat> > G_pyramid;
  vector< vector<Mat> > DoG_pyramid;
  vector< vector<Mat> > grad_x_pyramid;
  vector< vector<Mat> > grad_y_pyramid;
  vector< vector<Mat> > grad_mag_pyramid;
  vector<Mat> mask_pyramid;
  vector<Mat> border_pyramid;

  const int scale_levels = 3;
  const int octaves_num = 4;
  const double sigma_min = 1.6;
  const double k = pow(2, 1.0/double(scale_levels));
  const double r_th = 10.0;
  const double default_C_DoG = 0.015;
  const double C_DoG = (pow(2, 1.0/double(scale_levels))-1.0)/(pow(2, 1.0/3.0)-1.0)*default_C_DoG;
  const double R_threshold = (r_th+1)*(r_th+1)/r_th;

  Mat img_smooth;
  GaussianBlur(_imgGray, img_smooth, Size(0,0), 0.5, 0);

  Mat img_grad_x;
  Sobel(img_smooth, img_grad_x, CV_64F, 1, 0, 1);
  Mat img_grad_y;
  Sobel(img_smooth, img_grad_y, CV_64F, 0, 1, 1);
  Mat img_grad_x2 = img_grad_x.mul(img_grad_x);
  Mat img_grad_y2 = img_grad_y.mul(img_grad_y);
  Mat img_grad_mag;
  sqrt(img_grad_x2+img_grad_y2, img_grad_mag);
 
  Mat mask = _mask.clone();
  Mat border = _border.clone();
  for (int o = 0; o < octaves_num; ++o) {
    vector<Mat> layer;
    G_pyramid.push_back(layer);
    grad_x_pyramid.push_back(layer);
    grad_y_pyramid.push_back(layer);
    grad_mag_pyramid.push_back(layer);
    DoG_pyramid.push_back(layer);

    Mat img0;
    double sample_rate = pow(2,o);
    Size sample(_img.cols/sample_rate, _img.rows/sample_rate);
    resize(_imgGray, img0, sample);
    resize(mask, mask, sample);
    resize(border, border, sample);
    mask_pyramid.push_back(mask.clone());
    border_pyramid.push_back(border.clone());

    for (int s = 0; s < scale_levels+3; ++s) {
      Mat G;
      double sigma = sigma_min*pow(k,s);
      GaussianBlur(img0, G, Size(0,0), sigma, 0);
      G_pyramid[o].push_back(G);
 
      Mat G_grad_x;
      Sobel(G, G_grad_x, CV_64F, 1, 0, 1);
      grad_x_pyramid[o].push_back(G_grad_x);
      Mat G_grad_y;
      Sobel(G, G_grad_y, CV_64F, 0, 1, 1);
      grad_y_pyramid[o].push_back(G_grad_y);
      Mat G_grad_mag;
      Mat G_grad_x2 = G_grad_x.mul(G_grad_x);
      Mat G_grad_y2 = G_grad_y.mul(G_grad_y);
      sqrt(G_grad_x2+G_grad_y2, G_grad_mag);
      grad_mag_pyramid[o].push_back(G_grad_mag);
   
      //#define SHOW_G_PYRAMID
      #ifdef SHOW_G_PYRAMID
      string name = to_string(o) + "_" + to_string(s);
      namedWindow(name, CV_WINDOW_AUTOSIZE);
      imshow(name, grad_x_pyramid[o][s]);
      #endif
    }
    for (int s = 0; s < scale_levels+2; ++s) {
      Mat DoG;
      computeDoG(G_pyramid[o][s+1], G_pyramid[o][s], DoG);
      DoG_pyramid[o].push_back(DoG);
      
      //#define SHOW_DOG_PYRAMID
      #ifdef SHOW_DOG_PYRAMID
      string name = to_string(o) + "_" + to_string(s);
      namedWindow(name, CV_WINDOW_AUTOSIZE);
      imshow(name, DoG_pyramid[o][s]);
      #endif
    }
  }

  vector<MyFeature> f_canditates;
  for (int o = 0; o < octaves_num; ++o) {
    for (int s = 1; s <= scale_levels; ++s) {
      for (int y = 1; y < DoG_pyramid[o][s].rows-1; ++y) {
        for (int x = 1; x < DoG_pyramid[o][s].cols-1; ++x) {
          if (mask_pyramid[o].at<uchar>(y,x) == 0) continue;
          if (border_pyramid[o].at<uchar>(y,x) == 255) continue;
          double D = DoG_pyramid[o][s].at<char>(y, x);
          bool localMax = true, localMin = true;
          for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
              for (int ds = -1; ds <= 1; ++ds) {
                if (dx == 0 && dy == 0 && ds == 0) continue;
                double value = DoG_pyramid[o][s+ds].at<char>(y+dy, x+dx);
                if (value >= D) localMax = false; 
                if (value <= D) localMin = false;
              }
            }
          }
          if (localMax || localMin) {
            MyFeature feature(x,y,s,o);
            f_canditates.push_back(feature);
          }
        }
      }
    }
  }

  // Remove bad features
  for (int f = 0; f < f_canditates.size(); ++f) {
    MyFeature feature = f_canditates[f];
    int x = feature.pt.x;
    int y = feature.pt.y;
    int s = feature.s;
    int o = feature.o;

    double D = DoG_pyramid[o][s].at<char>(y, x);
    double D_r = DoG_pyramid[o][s].at<char>(y,x+1);
    double D_l = DoG_pyramid[o][s].at<char>(y,x-1);
    double D_t = DoG_pyramid[o][s].at<char>(y-1,x);
    double D_b = DoG_pyramid[o][s].at<char>(y+1,x);
    double D_tr = DoG_pyramid[o][s].at<char>(y-1,x+1);
    double D_tl = DoG_pyramid[o][s].at<char>(y-1,x-1);
    double D_br = DoG_pyramid[o][s].at<char>(y+1,x+1);
    double D_bl = DoG_pyramid[o][s].at<char>(y+1,x-1);
    double Dx = (D_r-D_l)/2.0;
    double Dy = (D_b-D_t)/2.0;
    double Dxx = D_r+D_l-2.0*D;
    double Dyy = D_t+D_b-2.0*D;
    double Dxy = (D_br+D_tl-D_bl-D_tr)/4.0;
    Mat hessian = (Mat_<double>(2, 2) << Dxx, Dxy, Dxy, Dyy);
           
    double det = determinant(hessian);
    double tr = trace(hessian)[0];
    double R = tr*tr/det;
    double contrast = D - 0.5/det*(Dx*Dx*Dyy-Dx*Dy*Dxy-Dx*Dy*Dxy+Dy*Dy*Dxx);
  
    if (std::abs(D) >= 255*0.8*C_DoG && 
        std::abs(R) < R_threshold && 
        std::abs(contrast) >= 255*0.03) 
    {
      Point2f x_m(0,0);
      if (det != 0) {
        x_m.x = -1/det*(Dx*Dyy-Dy*Dxy);
        x_m.y = -1/det*(-Dx*Dxy+Dy*Dxx);
      }
      feature.pt.x += x_m.x;
      feature.pt.y += x_m.y;
      _features.push_back(feature);
    }
  }
  cout << _features.size() << " features extracted" << endl; 

  // Orientation
  for (int f = 0; f < _features.size(); ++f) {
    cout << '\r'
         << "Computing the orientation of feature #" << f+1 << flush;
    MyFeature feature = _features[f];
  
    int x = feature.pt.x;
    int y = feature.pt.y;
    int s = feature.s;
    int o = feature.o;
    int width = G_pyramid[o][s].cols;
    int height = G_pyramid[o][s].rows;
    
    double sigma = sigma_min*pow(k,s);
    double S = sigma*pow(2,o);
 
    Mat g_kernel_x = getGaussianKernel(2*width, 1.5*sigma);
    Mat g_kernel_y = getGaussianKernel(2*height, 1.5*sigma);
  
    int local = 6*sigma;
    vector<double> orient_histo(36, 0.0);
    for (int dx = -local; dx <= local; ++dx) {
      for (int dy = -local; dy <= local; ++dy) {
        if (x+dx < 0 || x+dx >= G_pyramid[o][s].cols ||
            y+dy < 0 || y+dy >= G_pyramid[o][s].rows)
          continue;
        double Lx = grad_x_pyramid[o][s].at<double>(y+dy, x+dx);
        double Ly = grad_y_pyramid[o][s].at<double>(y+dy, x+dx);
        double L_mag = grad_mag_pyramid[o][s].at<double>(y+dy, x+dx);
        double gx = g_kernel_x.at<double>(width+dx);
        double gy = g_kernel_y.at<double>(height+dy);
        double L_angle = atan2(Ly,Lx) * 180.0 / M_PI;
        if (L_angle<0) L_angle += 360;
        int angle_slot = L_angle/10;
        orient_histo[angle_slot] += L_mag*gy*gx;
      }
    }
    int max_slot = max_element(orient_histo.begin(), orient_histo.end())-orient_histo.begin();
    double max_angle = max_slot*10.0 + 5.0;
    feature._angle = max_angle;
    feature._scale = S;
    _features[f] = feature;
  }
  cout << "\r                                                                 \r" << flush;
  cout << "All features' orientation computed" << endl;

  // Compute SIFT descriptor
  for (int f = 0; f < _features.size(); ++f) {
    cout << '\r'
         << "Computing the SIFT descriptor of feature #" << f+1 << flush;
    MyFeature feature = _features[f];
  
    int x = feature.pt.x;
    int y = feature.pt.y;
    int s = feature.s;
    int o = feature.o;
    double scale = feature._scale;
    double angle = feature._angle;
    double X = x*pow(2,o);
    double Y = y*pow(2,o);

    Mat neighbor = Mat::zeros(1, 128, CV_64F);
    Mat g_kernel_x = getGaussianKernel(2*_img.cols, 0.5*scale);
    Mat g_kernel_y = getGaussianKernel(2*_img.rows, 0.5*scale);

    for (double n = Y-16*scale; n < Y+16*scale; ++n) {
      for (double m = X-16*scale; m < X+16*scale; ++m) {
        if (n < 0 || n >= _img.rows || m < 0 || m >= _img.cols) continue;
        double x_head = ((m-X)*cos(angle*M_PI/180.0) + (n-Y)*sin(angle*M_PI/180.0)) / scale;
        double y_head = (-1*(m-X)*sin(angle*M_PI/180.0) + (n-Y)*cos(angle*M_PI/180.0)) / scale;

        if (x_head < -8 || x_head > 7) continue;
        if (y_head < -8 || y_head > 7) continue;
        int x_slot = (x_head+8)/4;
        int y_slot = (y_head+8)/4;
        double grad_x = img_grad_x.at<double>(n, m);
        double grad_y = img_grad_y.at<double>(n, m);
        double grad_mag = img_grad_mag.at<double>(n, m);
        double grad_angle = atan2(grad_y,grad_x) * 180.0 / M_PI;
        if (grad_angle<0) grad_angle += 360;
        int angle_slot = grad_angle/45;
  
        double gx = g_kernel_x.at<double>(_img.cols+m-X);
        double gy = g_kernel_y.at<double>(_img.rows+n-Y);
      
        int slot_num = 8*(y_slot*4+x_slot)+angle_slot;
        neighbor.at<double>(8*(y_slot*4+x_slot)+angle_slot) += 
          grad_mag*gx*gy;
      }
    }
    normalize(neighbor, neighbor, 1, 0);

    feature.pt.x *= pow(2,o);
    feature.pt.y *= pow(2,o);
    _features[f] = feature;
    _features[f]._neighbor = neighbor.clone();
  } 
  cout << "\r                                                                 \r" << flush;
  cout << "All features' SIFT descriptor computed" << endl;
}

