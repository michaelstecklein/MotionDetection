#include "opencv2/opencv.hpp"
#include "ThreadManager.hpp"
#include "PipelineBuffer.hpp"




static void temp(void) {
	const char *name = "../data/motion/1_1.jpg";
	Mat mat = cv::imread(name,CV_LOAD_IMAGE_COLOR);
	if(!mat.data) {
		printf("Error opening %s\n",name);
		exit(1);
	}

    int nChannels = mat.channels();
    int nRows = mat.rows;
    int nCols = mat.cols;
	int nPixels = nRows*nCols;

	printf("::: %s :::\n",name);
	printf("nChannels: %i\n",nChannels);
	printf("nRows: %i\n",nRows);
	printf("nCols: %i\n",nCols);
	printf("nPixels: %i\n",nPixels);

	int cnt = 0;
    for(int i = 0; i < nRows; ++i) {
        uchar *p = mat.ptr<uchar>(i);
        for(int j = 0; j < nCols; ++j) {
            uchar pixel = p[j];
			printf("row:%i\tcol:%i\tpixel:%i\n",i,j,pixel);
			if (cnt > 12)
				break;
			cnt++;
		}
		if (cnt > 12)
			break;
	}

	namedWindow("Display window", WINDOW_AUTOSIZE);	// Create a window for display.
    imshow("Display window", mat );					// Show our image inside it.
	waitKey(0);

	exit(0);
}

int main(int argc, char ** argv) {
	//temp();
	if (argc < 3) {
		printf("Usage: ./printvariances <img1.jpg> <img2.jpg>\n");
		exit(1);
	}

	Mat img1 = cv::imread(argv[1], 1);
	Mat img2 = cv::imread(argv[2], 1);
	if(!img1.data || !img2.data) {
		printf("Error opening %s or %s.\n",argv[1],argv[2]);
		exit(1);
	}

	PipelineBuffer buff1(1);
	PipelineBuffer buff2(2);

	buff1.rawImage = img1;
	buff2.rawImage = img2;

	double spatial_var, pixel_var;
	process(&buff1, &buff2, &spatial_var, &pixel_var);

	printf("%f, %f\n", spatial_var, pixel_var);
}
