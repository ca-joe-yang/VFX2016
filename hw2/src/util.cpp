#include "util.h"

void parseFilePath(string path, string& dir, string& base)
{
  char *dirc, *basec;
  dirc = strdup(path.c_str());
  basec = strdup(path.c_str());
  dir = dirname(dirc);
  base = basename(basec);
}

void toLowerCase(const string& str, string& lower)
{
  lower = "";
  locale loc;
  for (int i = 0; i < str.length(); ++i)
    lower += tolower(str[i], loc);
}

void getImagesInDir(const string dir, vector<string>& files) 
{
  DIR *dp = opendir(dir.c_str());
  struct dirent *ent;
  if (dp == NULL) 
  {
    cerr << "CANNOT find directory " + dir << endl;
    return;
  }
  string formatList[] = {"jpg", "png", "jpeg", "JPG"};
  vector<string> imgFormat(formatList, formatList+4);

  while ((ent = readdir(dp)) != NULL) {
    string name = ent->d_name;
    if (name == "." || name == "..") continue;
    for (int i = 0; i < imgFormat.size(); ++i) {
      string format = imgFormat[i];
      if (name.size() <= format.size()) continue;
      if (name.substr(name.size()-1 - format.size()) == "." + format) {
        files.push_back(name);
        break;
      }
    }
  }
  sort(files.begin(), files.end());
  closedir(dp);
}

int myrandom (int i) { return rand()%i; }

bool isColorRGB(const Vec3b v3, const uchar r, const uchar g, const uchar b) 
{
  return (v3[0] == r) && (v3[1] == g) && (v3[2] == b);
}

