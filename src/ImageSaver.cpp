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


static string fmt2(int i) {
	string str = to_string(i);
	if (i > 9) // not single digit
		return str;
	return "0" + str;
}

static string getTime(struct tm *time) {
	return fmt2(time->tm_hour) + ":" + fmt2(time->tm_min) + ":" + fmt2(time->tm_sec);
}

static string getDate(struct tm *time) {
	return to_string(time->tm_year + 1900) + "-" + fmt2(time->tm_mon + 1) + "-" + fmt2(time->tm_mday);
}

static string getNameRoot(PipelineBuffer& buff) { // YYYY/MM/DD__HH:MM:SS__tag
	struct tm *time = localtime(&buff.timestamp);
	int tag = buff.image_id % 1000;
	return getDate(time) + "__" + getTime(time) + "__" + to_string(tag);
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
