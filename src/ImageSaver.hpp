#ifndef _imagesaver_h_
#define _imagesaver_h_

#include "opencv2/opencv.hpp"
#include "PipelineBuffer.hpp"


void saveRawImage(PipelineBuffer& buff);

void saveDifferenceImage(PipelineBuffer& oldBuff, Mat& newImg, Mat& diff); // name created from oldImg

void saveLPFImage(PipelineBuffer& buff);


#endif /* _imagesaver_h_ */
