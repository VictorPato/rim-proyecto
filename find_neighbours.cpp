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
#include <queue>
#include <utility>

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
        std::cout << " Usage: ./find_neighbours <descriptors_folder> <image>" << std::endl;
        return -1; 
    }

    std::string descriptors_folder = argv[1];
    std::string image_name = argv[2];

    std::vector<std::string> descriptors = listar_archivos(descriptors_folder);

    int i = 0;
    int percent = 0;
    int prev_percent = -1;
    int total_size = descriptors.size();

    // iterate through all images to create descriptors
    int minHessian = 500;
    Ptr<SURF> detector = SURF::create(minHessian);
    detector->setUpright(true);
    Mat img = imread(image_name, IMREAD_GRAYSCALE );
    std::vector<KeyPoint> keypoints;
    Mat descriptor_of_image;
    detector->detectAndCompute(img, noArray(), keypoints, descriptor_of_image);

    std::priority_queue<std::pair<int, std::string>, std::vector<std::pair<int, std::string>>, std::greater<std::pair<int, std::string>>> neighbours;

    for (const std::string &descriptor_name : descriptors) {
        Mat desc;
        FileStorage file(descriptor_name, cv::FileStorage::READ);
        file["descriptor"] >> desc;

        //-- Step 2: Matching descriptor vectors with a FLANN based matcher
        // Since SURF is a floating-point descriptor NORM_L2 is used
        Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::FLANNBASED);
        std::vector<std::vector<DMatch>> knn_matches;
        matcher->knnMatch(descriptor_of_image, desc, knn_matches, 2);
        //-- Filter matches using the Lowe's ratio test
        const float ratio_thresh = 0.7f;
        int score = 0;
        // std::vector<DMatch> good_matches;
        for (size_t i = 0; i < knn_matches.size(); i++) {
            if (knn_matches[i][0].distance < ratio_thresh * knn_matches[i][1].distance) {
                // good_matches.push_back(knn_matches[i][0]);
                score++;
            }
        }

        int k = 4;

        if (neighbours.size() < k || (neighbours.top().first < score)) {
            neighbours.push(make_pair(score, descriptor_name));
        }

        if (neighbours.size() > 4) {
            neighbours.pop();
        }

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
    std::cout << std::endl;

    while (!neighbours.empty()) {
        std::cout << neighbours.top().second << std::endl;
        neighbours.pop();
    }
}
