#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <sstream>
#include <time.h>
#include "opencv2/opencv.hpp"
#include "ImageSaver.hpp"
#include "PipelineBuffer.hpp"

#define RAW_PATH	"images"
#define DIFF_PATH	"images/differences"
#define LPF_PATH	DIFF_PATH
#define IMG_EXT		".jpg"
#define DIFF_TAG	"_diff"
#define LPF_TAG		"_lpf"
#define NEW_TAG		"_new"
#define OLD_TAG		"_old"


static string getNameRoot(PipelineBuffer& buff) { // MM-DD-YYYY_HH-MM-SS_tag
	struct tm *time = gmtime(&buff.timestamp);
	int tag = buff.image_id % 1000;
	stringstream ss;
	ss <<	time->tm_mon << "-" << time->tm_mday << "-" << time->tm_year << \
			"_" << time->tm_hour << "-" << time->tm_min << "-" << time->tm_sec << \
			"_" << tag;
	return ss.str();
}

static void saveImg(string path, string name, Mat& img) {
	imwrite(path + "/" + name + IMG_EXT, img );
}

static void dir(string path) {
	// If dir doesn't exist, make it
	string cmd = "mkdir -p " + path;
	int ret = system(cmd.c_str());
	if (ret < 0) {
		printf ("Error creating directory %s\n", path.c_str());
		exit(1);
	}
}

void saveRawImage(PipelineBuffer& buff) {
	dir(RAW_PATH);
	saveImg(RAW_PATH, getNameRoot(buff), buff.rawImage);
}

void saveDifferenceImage(PipelineBuffer& oldBuff, Mat& newImg, Mat& diff) {
	dir(DIFF_PATH);
	saveImg(DIFF_PATH, getNameRoot(oldBuff) + NEW_TAG, diff);
	saveImg(DIFF_PATH, getNameRoot(oldBuff) + OLD_TAG, diff);
	saveImg(DIFF_PATH, getNameRoot(oldBuff) + DIFF_TAG, diff);
}

void saveLPFImage(PipelineBuffer& buff) {
	dir(LPF_PATH);
	saveImg(LPF_PATH, getNameRoot(buff) + LPF_TAG, buff.lpfImage);
}
