/*
	Pipeline:

			________________________ ...
			|
			V		<- empty
		Capture ( and Label)
			|
			V		<- pre-process
		Process (x2)
			|
			V		<- post-process
		Dispose (Save/Discard)
			|
			-----------------------> ...
*/
#ifndef __pipelinemanagager_hpp_
#define __pipelinemanagager_hpp_

#include <deque>
#include "PipelineBuffer.hpp"
using namespace std;

#define NUM_BUFFERS		10


class PipelineManager {
	public:
		PipelineManager();
		~PipelineManager();

		/*	Return a buffer that is in the respective place in the
			pipeline. Will hang while no buffer is available. */
		PipelineBuffer *getEmptyBuffer(void);
		PipelineBuffer *getNewPreProcessBuffer(void); // should call new before old
		PipelineBuffer *getOldPreProcessBuffer(void);
		PipelineBuffer *getPostProcessBuffer(void);

		/*	Called by given pipeline stage after the stage
			finishes. This is required of all stages to keep the
			pipeline moving.	*/
		void releaseCapture(PipelineBuffer *buff);
		void releaseProcess(PipelineBuffer *buff);
		void releaseDispose(PipelineBuffer *buff);

	private:
		deque<PipelineBuffer*> emptyQ;
		mutex emptyQ_mtx;
		deque<PipelineBuffer*> preprocessQ;
		mutex preprocessQ_mtx;
		deque<PipelineBuffer*> postprocessQ;
		mutex postprocessQ_mtx;
		void checkPreProcessLength(void);
};


#endif /* __pipelinemanagager_hpp_ */
