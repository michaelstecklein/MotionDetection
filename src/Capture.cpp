#include <stdio.h>
#include "opencv2/opencv.hpp"
#include "ThreadManager.hpp"
#include "PipelineBuffer.hpp"
using namespace cv;


int id = 1;
VideoCapture cap(0); // open the default camera


void capture(PipelineBuffer *buff) {
	printf("Capturing %i   %i\n", buff->buffer_id, buff->image_id);
	if (!cap.isOpened()) {
		printf("ERROR: VideoCapture is not opened\n");
		exit(-1);
	}
	buff->image_id = id++;
	cap >> buff->rawImage;
	buff->timestamp = time(0);
}
