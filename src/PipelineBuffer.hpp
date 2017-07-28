#ifndef __pipelinebuffer_hpp_
#define __pipelinebuffer_hpp_

#include <mutex>
#include <ctime>
#include "opencv2/opencv.hpp"
using namespace std;
using namespace cv;


class PipelineBuffer {
	public:
		enum Status {
			Empty,
			Capturing,
			PreProcess,
			Processing,
			PostProcess,
			Disposing
		};
		Status status; // TODO update this var
		const int buffer_id;
		int image_id;
		time_t timestamp;
		bool preprocessed;
		bool save_image;
		Mat rawImage;
		Mat lpfImage;

		PipelineBuffer(const int buff_id);
		~PipelineBuffer();

		void saveImage(const char *path);
};


#endif /* __pipelinebuffer_hpp_ */
