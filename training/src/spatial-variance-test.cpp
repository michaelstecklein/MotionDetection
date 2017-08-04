#include <stdio.h>
#include "opencv2/opencv.hpp"
#include "PipelineBuffer.hpp"
#include "Process.hpp"



#define MAT_SIZE	4



void fillMat(int n, Mat *mat) {
	Mat_<Vec3b> vec3b = *mat;
	for (int i = 0; i < MAT_SIZE; i++) {
		for (int j = 0; j < MAT_SIZE; j++) {
			if (i > 0 && j > 0) {
				vec3b(i,j)[0] = (i+j);
				vec3b(i,j)[1] = (i+j);
				vec3b(i,j)[2] = n*(i+j)*(i+j);
			} else {
				vec3b(i,j)[0] = 0;
				vec3b(i,j)[1] = 0;
				vec3b(i,j)[2] = 0;
			}
		}
	}
}


int main(void) {

	Mat mat;
	mat.create(MAT_SIZE,MAT_SIZE,CV_8UC3);

	for (int i = 1; i <= 3; i++) {
		fillMat(i,&mat);
		double spvar = calculateSpatialVariance(&mat);
		printf("spvar: %f  i: %i\n",spvar,i);
	}

}
