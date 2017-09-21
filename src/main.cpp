#include <iostream>
#include <cstdlib>

#include "tm_usage.h"
#include "stitch.h"

using namespace cv;
using namespace std;

int main(int argc, const char** argv)
{
  cout << endl;
  // Parsing
  // $ ./main <INPUT_DIRECTORY> <OUTPUT_IMAGE> <DETECT_METHOD> <BLEND_METHOD> 
  string detectMethod = "harris";
  string blendMethod = "feather";
  if (argc < 3) {
    cerr << "[ERROR] Missing arguments" << endl
         << "Please read README.txt for more instruction" << endl;
    return -1;
  } 
  else if (argc > 5) {
    cerr << "[ERROR] Too much arguments" << endl
         << "Please read README.txt for more instruction" << endl;
    return -1;
  }
  else if (argc == 3) {
    cout << "[WARNING] Using default detecting method: HARRIS" << endl;
    cout << "[WARNING] Using default blending method: FEATHER" << endl;
    cout << endl;
  }
  else if (argc == 4) {
    detectMethod = argv[3];
    cout << "[WARNING] Using default blending method: FEATHER" << endl;
    cout << endl;
  } 
  else if (argc == 5) {
    detectMethod = argv[3];
    blendMethod = argv[4]; 
  }
 //

  CommonNs::TmUsage tmusg;
  CommonNs::TmStat stat;
  tmusg.periodStart();
  double timer = 0.0;
    
  cout << "===== Loading Images =====" << endl;
  //
  MyCanvas myCanvas(argv[1], 3);
  //
  tmusg.getPeriodUsage(stat); 
  cout << "Time used: " << double(stat.uTime)/1000000 << "s" << endl;
  cout << endl;

  cout << "===== Extracting Features =====" << endl;
  //
  if (!myCanvas.detecting(detectMethod))
    return -1;
  //
  tmusg.getPeriodUsage(stat); 
  cout << "Time used: " << double(stat.uTime)/1000000 << "s" << endl;
  cout << endl;

  cout << "===== Matching Features =====" << endl;
  //
  int y_dev = 0;
  myCanvas.matching(y_dev);
  //
  tmusg.getPeriodUsage(stat); 
  cout << "Time used: " << double(stat.uTime)/1000000 << "s" << endl;
  cout << endl;

  cout << "===== Stitching Images =====" << endl;
  //
  if (!myCanvas.blending(blendMethod))
    return -1;
  //myCanvas.weakCropping();
  //myCanvas.drifting(y_dev);
  //myCanvas.strictCropping();
  myCanvas.show();
  cout << "Saving resulting paranoma to " << argv[2] << endl;
  myCanvas.save(argv[2]);
  //
  tmusg.getPeriodUsage(stat); 
  cout << "Time used: " << double(stat.uTime)/1000000 << "s" << endl;

  waitKey(0); 
  return 0;
}
