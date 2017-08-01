#ifndef _threadmanager_hpp_
#define _threadmanager_hpp_

#include <thread>
#include <mutex>
#include <stdbool.h>
#include "PipelineManager.hpp"


extern PipelineManager pipelineManager;

extern void capture(PipelineBuffer *buff);
extern void process(PipelineBuffer *newBuff, PipelineBuffer *oldBuff);
extern void process(PipelineBuffer *newBuff, PipelineBuffer *oldBuff,
			double *spatial_var_out, double *pixel_var_out, bool save_diff_image); // for use in training/debugging
extern void dispose(PipelineBuffer *buff);


void runThreads(void);


#endif /*  _threadmanager_hpp_ */
