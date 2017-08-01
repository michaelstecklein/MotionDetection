#include <stdbool.h>
#include "opencv2/opencv.hpp"
#include "ThreadManager.hpp"
#include "PipelineBuffer.hpp"


#define START_TAG	"<vars>"
#define END_TAG		"</vars>"



int main(int argc, char ** argv) {
	if (argc < 3) {
		printf("Usage: ./printvariances <img1.jpg> <img2.jpg>\n");
		exit(1);
	}

	// read in images
	Mat img1 = cv::imread(argv[1], 1);
	Mat img2 = cv::imread(argv[2], 1);
	if(!img1.data || !img2.data) {
		printf("Error opening %s or %s.\n",argv[1],argv[2]);
		exit(1);
	}

	// fill buffers with images
	PipelineBuffer buff1(1);
	PipelineBuffer buff2(2);
	buff1.rawImage = img1;
	buff2.rawImage = img2;

	// run process
	double spatial_var, pixel_var;
	process(&buff1, &buff2, &spatial_var, &pixel_var, true);

	// output variances
	printf("%s%f,%f%s\n", START_TAG, spatial_var, pixel_var, END_TAG);
}
