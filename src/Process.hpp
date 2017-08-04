#ifndef _process_hpp_
#define _process_hpp_



#define PIXEL_VAR_THRESHOLD			25.0
#define SPATIAL_VAR_THRESHOLD		425.0
#define CHANGE_THRESHOLD			10
#define PERCENT_CHANGE_THRESHOLD	10


#define LPF_DIVISOR				10 // length of squares to break raw image into during lpf

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


void preprocess_lpf(PipelineBuffer *buff);

double calculateSpatialVariance(Mat *mat);

double calculateVariance(Mat *mat);

void calculateCenterOfMass(Mat *mat, bgr<double> *x_com_out, bgr<double> *y_com_out);

void getDifferenceImage(Mat *img1, Mat *img2, Mat *diff_img_out);

void getDifferenceImageWithCOM(Mat *img1, Mat *img2, Mat *diff_img_out); // centers of mass for the 3 channels are marked


#endif /*  _process_hpp_ */
