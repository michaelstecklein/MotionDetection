#include <stdio.h>
#include <math.h>
#include "opencv2/opencv.hpp"
#include "Analytics.hpp"
#include "ThreadManager.hpp"
#include "PipelineBuffer.hpp"
#include "ImageSaver.hpp"


#define PIXEL_VAR_COEFF		1
#define SPATIAL_VAR_COEFF	1
#define COST_THRESHHOLD		1
#define LPF_DIVISOR			10 // length of squares to break raw image into during lpf


static void preprocess_lpf(PipelineBuffer *buff) {
    CV_Assert(buff->rawImage.depth() == CV_8U); // accept only char type matrices
	//CV_Assert(buff->rawImage.isContinuous()); // TODO
    int nChannels = buff->rawImage.channels();
    int nRows = buff->rawImage.rows;
    int nCols = buff->rawImage.cols * nChannels; // TODO account for channels
	int nPixels = nRows*nCols;

	// construct lpf image
	int lpfRows = nRows / LPF_DIVISOR;
	int lpfCols = nCols / LPF_DIVISOR;
	buff->lpfImage.create(lpfRows, lpfCols, buff->rawImage.type());

    for(int i_chunk = 0; i_chunk < lpfRows; ++i_chunk) { // for each chunk
        for(int j_chunk = 0; j_chunk < lpfCols; ++j_chunk) {
			// calculate the average value for that chunk
			int sum = 0;
			for (int i_inner = 0; i_inner < LPF_DIVISOR; ++i_inner) { // for each pixel in the given chunk
				for (int j_inner = 0; j_inner < LPF_DIVISOR; ++j_inner) {
					int i_buff = i_chunk*LPF_DIVISOR + i_inner; // calculate global indexes
					int j_buff = j_chunk*LPF_DIVISOR + j_inner;
					sum += buff->rawImage.at<uchar>(i_buff,j_buff);
				}
			}
			const int numPixelsInChunk = LPF_DIVISOR*LPF_DIVISOR;
			int avg = sum / numPixelsInChunk;
			buff->lpfImage.at<uchar>(i_chunk,j_chunk) = avg;
        }
    }
}

double x_com;
double y_com;
static double calculateSpatialVariance(Mat *mat) {
	/* Weighted two-dimensional variance in x,y directions with pixel color
	 * as weight. The variance is calculated around the "center of mass" of
	 * the image.	var = x_var + y_var,  x_var,y_var = E[pixel*(x,y-mean)]	*/
    CV_Assert(mat->depth() == CV_8U); // accept only char type matrices
	//CV_Assert(mat->isContinuous()); // TODO
    int nChannels = mat->channels();
    int nRows = mat->rows;
    int nCols = mat->cols * nChannels; // TODO account for channels
	int nPixels = nRows*nCols;

	// center of mass (mean)
	// x_com = (m1*x1+m2*x2+...) / (m1+m2+...)
	double x_numerator = 0;
	double y_numerator = 0;
	double pixel_sum = 0; // denominator
    for(int i = 0; i < nRows; ++i) {
        uchar *p = mat->ptr<uchar>(i);
        for(int j = 0; j < nCols; ++j) {
            uchar pixel = p[j];
			x_numerator += j*pixel;
			y_numerator += i*pixel;
			pixel_sum += pixel;
        }
    }
	x_com = x_numerator / pixel_sum;
	y_com = y_numerator / pixel_sum;
	printf("Centers of mass:   x_com: %f   y_com: %f\n", x_com, y_com);

	// spatial variance
	// spat_var = weighted_var_x + weighted_var_y
	// weighted_var_x = ( m1*(x1-x_com)^2 + m2*(x2-x_com)^2 + ...) / (m1+m2+...)
	double sum_sqr_x = 0;
	double sum_sqr_y = 0;
    for(int i = 0; i < nRows; ++i) {
        uchar *p = mat->ptr<uchar>(i);
        for(int j = 0; j < nCols; ++j) {
            uchar pixel = p[j];
			sum_sqr_x += pixel*pow(j-x_com, 2);
			sum_sqr_y += pixel*pow(i-y_com, 2);
        }
    }
	double weighted_var_x = sum_sqr_x / pixel_sum;
	double weighted_var_y = sum_sqr_y / pixel_sum;

	return weighted_var_x + weighted_var_y;
}

static double calculateVariance(Mat *mat) {
	/* Calculates the variance of the parameter Mat's pixels  where: var = var_b + var_g + var_r
	 * for channels blue(b), green(g), and red(r).	*/
    CV_Assert(mat->depth() == CV_8U); // accept only char type matrices
    int nChannels = mat->channels();
	printf("nChannels=%i\n",nChannels);
	CV_Assert(nChannels==3);
    int nRows = mat->rows;
    int nCols = mat->cols;
	int nPixels = nRows*nCols;
    if (mat->isContinuous()) {
        nCols *= nRows;
        nRows = 1;
    }

	// mean
	double b_sum = 0; // blue
	double g_sum = 0; // green
	double r_sum = 0; // red
    for(int i = 0; i < nRows; ++i) {
        uchar *p = mat->ptr<uchar>(i);
        for(int j = 0; j < nCols; ++j) {
            uchar b_pixel = p[j*nChannels + 0];
            uchar g_pixel = p[j*nChannels + 1];
            uchar r_pixel = p[j*nChannels + 2];
			b_sum += b_pixel;
			g_sum += g_pixel;
			r_sum += r_pixel;
        }
    }
	double b_mean = b_sum / nPixels;
	double g_mean = g_sum / nPixels;
	double r_mean = r_sum / nPixels;

	// variance
	double b_sum_sqrs = 0; // blue
	double g_sum_sqrs = 0; // green
	double r_sum_sqrs = 0; // red
    for(int i = 0; i < nRows; ++i) {
        uchar *p = mat->ptr<uchar>(i);
        for(int j = 0; j < nCols; ++j) {
            uchar b_pixel = p[j*nChannels + 0];
            uchar g_pixel = p[j*nChannels + 1];
            uchar r_pixel = p[j*nChannels + 2];
			b_sum_sqrs += pow(b_pixel-b_mean, 2);
			g_sum_sqrs += pow(g_pixel-g_mean, 2);
			r_sum_sqrs += pow(r_pixel-r_mean, 2);
        }
    }
	double b_var = b_sum_sqrs / nPixels;
	double g_var = g_sum_sqrs / nPixels;
	double r_var = r_sum_sqrs / nPixels;

	return b_var + g_var + r_var;
}


Mat image_diff;

void process(PipelineBuffer *newBuff, PipelineBuffer *oldBuff, double *spatial_var_out, double *pixel_var_out) {
	printf("Processing %i   %i   and   %i   %i\n", newBuff->buffer_id, newBuff->image_id, oldBuff->buffer_id, oldBuff->image_id);

	/* Calculate lpfImage, whose pixels are averages of square chunks of the original
	 * image. This is a crude low-pass filter and will remove minor camera shifts between
	 * frames as well as reduce processing.	*/
	if (!newBuff->preprocessed) {
		preprocess_lpf(newBuff);
		newBuff->preprocessed = true;
	}
	if (!oldBuff->preprocessed) {
		preprocess_lpf(oldBuff);
		oldBuff->preprocessed = true;
	}

	/* Take a difference of the lpf images to see changes.	*/
	cv::absdiff(newBuff->lpfImage, oldBuff->lpfImage, image_diff);

	/* Calculate spatial variance (x, y) and pixel variance for the difference image.	*/
	double spatial_var = calculateSpatialVariance(&image_diff);
	double pixel_var = calculateVariance(&image_diff);

	/* Calculate cost with linear cost function.   c = A*pixel_var - B*spatial_var	*/
	double cost = PIXEL_VAR_COEFF * pixel_var  -  SPATIAL_VAR_COEFF * spatial_var;

	/* Threshhold the cost to decide if motion occurred, and set save_image flag if so.	*/
	if (cost > COST_THRESHHOLD)
		oldBuff->save_image = true;

	/* Set output parameters */
	*spatial_var_out = spatial_var;
	*pixel_var_out = pixel_var;


#ifdef SAVE_ALL_IMAGES
	saveDifferenceImage(oldBuff, rawBuff->rawImage, image_diff);
	saveLPFImage(oldBuff->lpfImage);
#endif /* SAVE_ALL_IMAGES */
}


void process(PipelineBuffer *newBuff, PipelineBuffer *oldBuff) {
	double tmp1, tmp2;
	process(newBuff, oldBuff, &tmp1, &tmp2);
}
