#include "Analytics.hpp"
#include "ThreadManager.hpp"


PipelineManager pipelineManager;


static void captureHandle() {
	PipelineBuffer *buff = NULL;
	while (1) {
		buff = pipelineManager.getEmptyBuffer();
		startCaptureAnalytics();
		capture(buff);
		endCaptureAnalytics();
		pipelineManager.releaseCapture(buff);
	}
}

void processHandle() {
	PipelineBuffer *newBuff = NULL;
	PipelineBuffer *oldBuff = NULL;
	while (1) {
		if (oldBuff == NULL) // first iteration only
			oldBuff = pipelineManager.getPreProcessBuffer();
		newBuff = pipelineManager.getPreProcessBuffer();
		startProcessAnalytics();
		process(newBuff, oldBuff);
		endProcessAnalytics();
		pipelineManager.releaseProcess(oldBuff);
		oldBuff = newBuff;
	}
}

void disposeHandle() {
	PipelineBuffer *buff = NULL;
	while (1) {
		buff = pipelineManager.getPostProcessBuffer();
		startDisposeAnalytics();
		dispose(buff);
		endDisposeAnalytics();
		pipelineManager.releaseDispose(buff);
	}
}



void runThreads() {
	thread captureThread(captureHandle);
	thread processThread(processHandle);
	thread disposeThread(disposeHandle);
	captureThread.join();
	processThread.join();
	disposeThread.join();
}
