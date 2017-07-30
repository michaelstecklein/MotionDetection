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


static void saveAsFile(const char *fname, Mat *img) { // TODO remove
	FILE *filp;
	filp = fopen(fname,"w");

    int nChannels = img->channels();
    int nRows = img->rows;
    int nCols = img->cols; // TODO account for channels
	printf("%s r:%i c:%i chan:%i\n",fname,nRows,nCols,nChannels);
	for (int i = 0; i < nRows; i++)
		for (int j = 0; j < nCols; j++)
			fprintf(filp, "%i\n", img->at<uchar>(i,j));

	fprintf(filp, "-------\n");
	fclose(filp);
}

#define BLUE	0
#define GREEN	1
#define RED		2
template <typename T>
class bgr {
  public:
	T b,g,r;
	bgr (void) : bgr (0,0,0) {}
	bgr (T _b, T _g, T _r) {
		b = _b; g = _g; r = _r; }
};

static void preprocess_lpf(PipelineBuffer *buff) {
    CV_Assert(buff->rawImage.depth() == CV_8U); // accept only char type matrices
    int nChannels = buff->rawImage.channels();
	CV_Assert(nChannels == 3);
    int nRows = buff->rawImage.rows;
    int nCols = buff->rawImage.cols;
	int nPixels = nRows*nCols;
	printf("%s r:%i c:%i chan:%i\n","preprocess_lpf",nRows,nCols,nChannels); // TODO remove
	double aspectRatio = ((double)nCols) / nRows; // TODO remove
	printf("aspectRation: %f\n",aspectRatio);

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
					printf("bgr: %i %i %i\n",pixel.b,pixel.g,pixel.r); // TODO remove
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

static double calculateSpatialVariance(Mat *mat) {
	/* Weighted two-dimensional variance in x,y directions with pixel color
	 * as weight. The variance is calculated around the "center of mass" of
	 * the image.	var = x_var + y_var,  x_var,y_var = E[pixel*(x,y-mean)]	*/
    CV_Assert(mat->depth() == CV_8U); // accept only char type matrices
    int nChannels = mat->channels();
	CV_Assert(nChannels == 3);
    int nRows = mat->rows;
    int nCols = mat->cols; // TODO account for channels
	int nPixels = nRows*nCols;
	Mat_<Vec3b> mat_vec3b = *mat;
	printf("%s r:%i c:%i chan:%i\n","calculateSpatialVariance",nRows,nCols,nChannels); // TODO remove
	double aspectRatio = ((double)nCols) / nRows;
	printf("aspectRation: %f\n",aspectRatio);

	/* Center of Mass (mean)
	 * x_com = (m1*x1+m2*x2+...) / (m1+m2+...)
	 * y_com = (m1*y1+m2*y2+...) / (m1+m2+...)	*/
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
	printf("Centers of mass, blue:   x_com: %f   y_com: %f\n", x_com.b, y_com.b);

	/* Spatial Variance
	 * spat_var = weighted_var_x + weighted_var_y
	 * weighted_var_x = var_x_channel_b + var_x_channel_g + var_x_channel_r
	 * var_x_channel_m = ( m1*(x1-x_com)^2 + m2*(x2-x_com)^2 + ...) / (m1+m2+...)	*/
	bgr<double> sum_sqr_x = bgr<double>();
	bgr<double> sum_sqr_y = bgr<double>();
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
        }
    }
	bgr<double> weighted_var_x = {	sum_sqr_x.b / pixel_sum.b,
									sum_sqr_x.g / pixel_sum.g,
									sum_sqr_x.r / pixel_sum.r	};
	bgr<double> weighted_var_y = {	sum_sqr_y.b / pixel_sum.b,
									sum_sqr_y.g / pixel_sum.g,
									sum_sqr_y.r / pixel_sum.r	};

	/* Spatial variance is sum of var_x and var_y, which are sums of the variances
	 * of the channels b, g, and r.	*/
	return	weighted_var_x.b + weighted_var_y.b + \
			weighted_var_x.g + weighted_var_y.g + \
			weighted_var_x.r + weighted_var_y.r;
}

static double calculateVariance(Mat *mat) {
	/* Calculates the variance of the parameter Mat's pixels  where: var = var_b + var_g + var_r
	 * for channels blue(b), green(g), and red(r).	*/
    CV_Assert(mat->depth() == CV_8U); // accept only char type matrices
    int nChannels = mat->channels();
	CV_Assert(nChannels == 3);
	printf("nChannels=%i\n",nChannels);
	CV_Assert(nChannels==3);
    int nRows = mat->rows;
    int nCols = mat->cols;
	int nPixels = nRows*nCols;
	Mat_<Vec3b> mat_vec3b = *mat;
	printf("%s r:%i c:%i chan:%i\n","calculateVariance",nRows,nCols,nChannels); // TODO remove
	double aspectRatio = ((double)nCols) / nRows;
	printf("aspectRation: %f\n",aspectRatio);

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

	saveAsFile("tmp_pre1", &newBuff->lpfImage); // TODO remove
	saveAsFile("tmp_pre2", &oldBuff->lpfImage); // TODO remove
	/* Take a difference of the lpf images to see changes.	*/
	cv::absdiff(newBuff->lpfImage, oldBuff->lpfImage, image_diff);
	saveAsFile("tmp_diff", &image_diff); // TODO remove

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
