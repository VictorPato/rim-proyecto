#include <iostream>
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/xfeatures2d.hpp"

#include <dirent.h>
#include <sys/stat.h>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

using namespace cv;
using namespace cv::xfeatures2d;
using std::cout;
using std::endl;

#if defined WIN32 || defined _WIN32
#define IS_WINDOWS 1
#endif

#if defined linux || defined __linux
#define IS_LINUX 1
#endif

void agregar_archivo(const std::string &dirname, const std::string &name, std::vector<std::string> &list) {
    std::string fullpath = dirname + "/" + name;
#if IS_WINDOWS
    struct stat64 st;
  int status = stat64(fullpath.c_str(), &st);
#elif IS_LINUX
    struct stat st;
    int status = stat(fullpath.c_str(), &st);
#endif
    if (status == 0 && S_ISREG(st.st_mode)) {
        list.push_back(fullpath);
    }
}

std::vector<std::string> listar_archivos(const std::string &dirname) {
    std::vector<std::string> list;
#if IS_WINDOWS
    DIR *dp = opendir(dirname.c_str());
    if (dp == NULL) {
        std::cout << "error abriendo " << dirname << std::endl;
        return list;
    }
    struct dirent *dir_entry;
    while ((dir_entry = readdir(dp)) != NULL) {
        std::string name(dir_entry->d_name);
        agregar_archivo(dirname, name, list);
    }
    if (closedir(dp) != 0) {
        std::cout << "error cerrando " << dirname << std::endl;
    }
#elif IS_LINUX
    struct dirent **namelist = NULL;
    int len = scandir(dirname.c_str(), &namelist, NULL, NULL);
    if (len < 0) {
        std::cout << "error abriendo " << dirname << std::endl;
        return list;
    }
    for (int i = 0; i < len; ++i) {
        std::string name(namelist[i]->d_name);
        agregar_archivo(dirname, name, list);
        free(namelist[i]);
    }
    free(namelist);
#endif
    std::sort(list.begin(), list.end());
    return list;
}

int main(int argc, char* argv[]) {
    if( argc != 3 ){
        std::cout << " Usage: ./create_descriptors <source_folder> <destination_folder>" << std::endl;
        return -1; 
    }

    std::string source_folder = argv[1];
    std::string destination_folder = argv[2];

    std::vector<std::string> source_images = listar_archivos(source_folder);

    int i = 0;
    int percent = 0;
    int prev_percent = -1;
    int total_size = source_images.size();

    // iterate through all images to create descriptors
    int minHessian = 400;
    Ptr<SURF> detector = SURF::create(minHessian);
    for (const std::string &image_name : source_images) {
        Mat img = imread(image_name, IMREAD_GRAYSCALE );
        std::vector<KeyPoint> keypoints;
        Mat descriptors;
        detector->detectAndCompute(img, noArray(), keypoints, descriptors);

        std::string new_name = image_name.substr(image_name.find('/'));
        new_name = destination_folder + new_name.substr(0, new_name.find('.')) + ".xml";

        FileStorage file(new_name, cv::FileStorage::WRITE);
        file << "descriptor" << descriptors;

        // Display progress
        i++;
        percent = (i * 100) / total_size;
        if (percent - prev_percent == 1) {
            std::cout << "\r" << "Progreso: [";
            if (percent < 100)
                std::cout << " ";
            if (percent < 10)
                std::cout << " ";
            std::cout << percent << "%] ";
            std::cout << "[";
            for (int j = 1; j <= 100; j++) {
                if (j <= percent)
                    std::cout << "#";
                else
                    std::cout << ".";
            }
            std::cout << "]" << std::flush;
            prev_percent = percent;
        }
    }
}
