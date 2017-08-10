#include <stdio.h>
#include <math.h>
#include "opencv2/opencv.hpp"
#include "Analytics.hpp"
#include "ThreadManager.hpp"
#include "PipelineBuffer.hpp"
#include "ImageSaver.hpp"
#include "Process.hpp"





void preprocess_lpf(PipelineBuffer *buff) {
    CV_Assert(buff->rawImage.depth() == CV_8U); // accept only char type matrices
    int nChannels = buff->rawImage.channels();
	CV_Assert(nChannels == 3);
    int nRows = buff->rawImage.rows;
    int nCols = buff->rawImage.cols;
	int nPixels = nRows*nCols;

	// construct lpf image
	int lpfRows = nRows / LPF_DIVISOR;
	int lpfCols = nCols / LPF_DIVISOR;
	buff->lpfImage.create(lpfRows, lpfCols, buff->rawImage.type());

	Mat_<Vec3b> raw_vec3b = buff->rawImage;
	Mat_<Vec3b> lpf_vec3b = buff->lpfImage;
    for(int i_chunk = 0; i_chunk < lpfRows; ++i_chunk) { // for each chunk
        for(int j_chunk = 0; j_chunk < lpfCols; ++j_chunk) {
			// calculate the average value for that chunk
			bgr<int> sum = bgr<int>();
			for (int i_inner = 0; i_inner < LPF_DIVISOR; ++i_inner) { // for each pixel in the given chunk
				for (int j_inner = 0; j_inner < LPF_DIVISOR; ++j_inner) {
					int i = i_chunk*LPF_DIVISOR + i_inner; // calculate global indexes
					int j = j_chunk*LPF_DIVISOR + j_inner;
					bgr<uchar> pixel = {	raw_vec3b(i,j)[BLUE],
											raw_vec3b(i,j)[GREEN],
											raw_vec3b(i,j)[RED]	};
					sum.b += pixel.b;
					sum.g += pixel.g;
					sum.r += pixel.r;
				}
			}
			const int numPixelsInChunk = LPF_DIVISOR*LPF_DIVISOR;
			bgr<int> avg = {	sum.b / numPixelsInChunk,
									sum.g / numPixelsInChunk,
									sum.r / numPixelsInChunk	};
			lpf_vec3b(i_chunk,j_chunk)[BLUE] = avg.b;
			lpf_vec3b(i_chunk,j_chunk)[GREEN] = avg.g;
			lpf_vec3b(i_chunk,j_chunk)[RED] = avg.r;
        }
    }
}

void calculateCenterOfMass(Mat *mat, bgr<double> *x_com_out, bgr<double> *y_com_out) {
	/* x_com = (m1*x1+m2*x2+...) / (m1+m2+...)
	 * y_com = (m1*y1+m2*y2+...) / (m1+m2+...)	*/
    int nChannels = mat->channels();
	CV_Assert(nChannels == 3);
    int nRows = mat->rows;
    int nCols = mat->cols;
	int nPixels = nRows*nCols;
	Mat_<Vec3b> mat_vec3b = *mat;
	bgr<double> x_numerator = bgr<double>();
	bgr<double> y_numerator = bgr<double>();
	bgr<double> pixel_sum = bgr<double>(); // denominator
    for(int i = 0; i < nRows; ++i) {
        for(int j = 0; j < nCols; ++j) {
			bgr<uchar> pixel = {	mat_vec3b(i,j)[BLUE],
									mat_vec3b(i,j)[GREEN],
									mat_vec3b(i,j)[RED]	};
			x_numerator.b += j * pixel.b;
			x_numerator.g += j * pixel.g;
			x_numerator.r += j * pixel.r;
			y_numerator.b += i * pixel.b;
			y_numerator.g += i * pixel.g;
			y_numerator.r += i * pixel.r;
			pixel_sum.b += pixel.b;
			pixel_sum.g += pixel.g;
			pixel_sum.r += pixel.r;
        }
    }
	bgr<double> x_com = {	x_numerator.b / pixel_sum.b,
							x_numerator.g / pixel_sum.g,
							x_numerator.r / pixel_sum.r	};
	bgr<double> y_com = {	y_numerator.b / pixel_sum.b,
							y_numerator.g / pixel_sum.g,
							y_numerator.r / pixel_sum.r	};

	*x_com_out = x_com;
	*y_com_out = y_com;
}

double calculateSpatialVariance(Mat *mat) {
	/* Weighted two-dimensional variance in x,y directions with pixel color
	 * as weight. The variance is calculated around the "center of mass" of
	 * the image.	var = x_var + y_var,  x_var,y_var = E[pixel*(x,y-mean)]	*/
    CV_Assert(mat->depth() == CV_8U); // accept only char type matrices
    int nChannels = mat->channels();
	CV_Assert(nChannels == 3);
    int nRows = mat->rows;
    int nCols = mat->cols;
	int nPixels = nRows*nCols;
	Mat_<Vec3b> mat_vec3b = *mat;

	/* Center of Mass (mean) */
	bgr<double> x_com;
	bgr<double> y_com;
	calculateCenterOfMass(mat, &x_com, &y_com);

	/* Spatial Variance
	 * Is the weighted sum of var_x and var_y, which are sums of the variances of the channels b, g, and r times their
	 * pixel sums all divided by the sum of pixels sums.
	 * spatial_var = weighted_var_x + weighted_var_y
	 * weighted_var_x = ( var_x.b*pixel_sum.b + var_x.g*pixel_sum.g + var_x.r*pixel_sum.r ) / (pixel_sum.b + pixel_sum.g + pixel_sum.r)
	 * var_x = ( m1*(x1-x_com)^2 + m2*(x2-x_com)^2 + ...) / (m1+m2+...)	for m = b,g,r	*/
	bgr<double> sum_sqr_x = bgr<double>();
	bgr<double> sum_sqr_y = bgr<double>();
	bgr<double> pixel_sum = bgr<double>(); // denominator
    for(int i = 0; i < nRows; ++i) {
        for(int j = 0; j < nCols; ++j) {
			bgr<uchar> pixel = {	mat_vec3b(i,j)[BLUE],
									mat_vec3b(i,j)[GREEN],
									mat_vec3b(i,j)[RED]	};
			sum_sqr_x.b += pixel.b * pow( j - x_com.b, 2);
			sum_sqr_x.g += pixel.g * pow( j - x_com.g, 2);
			sum_sqr_x.r += pixel.r * pow( j - x_com.r, 2);
			sum_sqr_y.b += pixel.b * pow( i - y_com.b, 2);
			sum_sqr_y.g += pixel.g * pow( i - y_com.g, 2);
			sum_sqr_y.r += pixel.r * pow( i - y_com.r, 2);
			pixel_sum.b += pixel.b;
			pixel_sum.g += pixel.g;
			pixel_sum.r += pixel.r;
        }
    }
	bgr<double> var_x = {	sum_sqr_x.b / pixel_sum.b,
									sum_sqr_x.g / pixel_sum.g,
									sum_sqr_x.r / pixel_sum.r	};
	bgr<double> var_y = {	sum_sqr_y.b / pixel_sum.b,
									sum_sqr_y.g / pixel_sum.g,
									sum_sqr_y.r / pixel_sum.r	};

	double weighted_var_x = ( var_x.b*pixel_sum.b + var_x.g*pixel_sum.g + var_x.r*pixel_sum.r ) \
										 / ( pixel_sum.b + pixel_sum.g + pixel_sum.r );
	double weighted_var_y = ( var_y.b*pixel_sum.b + var_y.g*pixel_sum.g + var_y.r*pixel_sum.r ) \
										 / ( pixel_sum.b + pixel_sum.g + pixel_sum.r );
	return weighted_var_x + weighted_var_y; // spatial_var
}

double calculateVariance(Mat *mat) {
	/* Calculates the variance of the parameter Mat's pixels  where: var = var_b + var_g + var_r
	 * for channels blue(b), green(g), and red(r).	*/
    CV_Assert(mat->depth() == CV_8U); // accept only char type matrices
    int nChannels = mat->channels();
	CV_Assert(nChannels == 3);
	CV_Assert(nChannels==3);
    int nRows = mat->rows;
    int nCols = mat->cols;
	int nPixels = nRows*nCols;
	Mat_<Vec3b> mat_vec3b = *mat;

	// mean
	bgr<double> sum = bgr<double>();
    for(int i = 0; i < nRows; ++i) {
        for(int j = 0; j < nCols; ++j) {
			bgr<uchar> pixel =	{	mat_vec3b(i,j)[0],
									mat_vec3b(i,j)[1],
									mat_vec3b(i,j)[2] };
			sum.b += pixel.b;
			sum.g += pixel.g;
			sum.r += pixel.r;
        }
    }
	bgr<double> mean = {	sum.b / nPixels,
							sum.g / nPixels,
							sum.r / nPixels };

	// variance
	bgr<double> sum_sqrs = bgr<double>();
    for(int i = 0; i < nRows; ++i) {
        for(int j = 0; j < nCols; ++j) {
			bgr<uchar> pixel =	{	mat_vec3b(i,j)[0],
									mat_vec3b(i,j)[1],
									mat_vec3b(i,j)[2] };
			sum_sqrs.b += pow( pixel.b - mean.b, 2);
			sum_sqrs.g += pow( pixel.g - mean.g, 2);
			sum_sqrs.r += pow( pixel.r - mean.r, 2);
        }
    }
	bgr<double> var = {	sum_sqrs.b / nPixels,
						sum_sqrs.g / nPixels,
						sum_sqrs.r / nPixels };

	return var.b + var.g + var.r;
}

void getDifferenceImage(Mat *img1, Mat *img2, Mat *diff_img_out) {
	cv::absdiff(*img1, *img2, *diff_img_out);
}

#define COM_CIRCLE_RADIUS	4
void getDifferenceImageWithCOM(Mat *img1, Mat *img2, Mat *diff_img_out) {
	cv::absdiff(*img1, *img2, *diff_img_out);
	bgr<double> x_com, y_com;
	calculateCenterOfMass(diff_img_out, &x_com, &y_com);
	cv::circle(*diff_img_out, Point(int(x_com.b), int(y_com.b)), COM_CIRCLE_RADIUS, Scalar(255,0,0)); // blue
	cv::circle(*diff_img_out, Point(int(x_com.g), int(y_com.g)), COM_CIRCLE_RADIUS, Scalar(0,255,0)); // green
	cv::circle(*diff_img_out, Point(int(x_com.r), int(y_com.r)), COM_CIRCLE_RADIUS, Scalar(0,0,255)); // red
}

double percentChange(Mat *img) {
    CV_Assert(img->depth() == CV_8U); // accept only char type matrices
	CV_Assert(img->channels() == 3);
	int numChange = 0;
	int numPixels = 0;
	Mat_<Vec3b> vec3b = *img;
	for (int i = 0; i < img->rows; i++) {
		for (int j = 0; j < img->cols; j++) {
			if (vec3b(i,j)[0] > CHANGE_THRESHOLD ||
				vec3b(i,j)[1] > CHANGE_THRESHOLD ||
				vec3b(i,j)[2] > CHANGE_THRESHOLD ) {
				numChange++;
			}
			numPixels++;
		}
	}
	return 100 * ((double) numChange) / ((double) numPixels);
}


Mat image_diff;

void process(PipelineBuffer *newBuff, PipelineBuffer *oldBuff) {

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
	getDifferenceImage(&newBuff->lpfImage, &oldBuff->lpfImage, &image_diff);

	/* Calculate spatial variance (x, y) and pixel variance for the difference image.	*/
	double spatial_var = calculateSpatialVariance(&image_diff);
	double pixel_var = calculateVariance(&image_diff);

	/* Threshhold the cost to decide if motion occurred, and set save_image flag if so.	*/
	if (pixel_var >= PIXEL_VAR_THRESHOLD && spatial_var >= SPATIAL_VAR_THRESHOLD)
		// both must meet thresholds, then motion
		oldBuff->save_image = true;
	else
		oldBuff->save_image = false;
	if (percentChange(&image_diff) > PERCENT_CHANGE_THRESHOLD)
		// If >'x'% of pixels are changed above a certain threshhold, no motion
		oldBuff->save_image = false;


#ifdef SAVE_ALL_IMAGES
	saveDifferenceImage(*oldBuff, newBuff->rawImage, image_diff);
	saveLPFImage(*oldBuff);
#endif /* SAVE_ALL_IMAGES */
}
