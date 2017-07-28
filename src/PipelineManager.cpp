#include <assert.h>
#include "PipelineBuffer.hpp"
#include "PipelineManager.hpp"

#define PB	PipelineBuffer


PipelineManager::PipelineManager() {
	for (int i = 0; i < NUM_BUFFERS; i++) {
		emptyQ.push( new PipelineBuffer(i) );
	}
}

PipelineManager::~PipelineManager() {
	int deletedCnt = 0;
	while (!emptyQ.empty()) {
		delete emptyQ.front();
		emptyQ.pop();
		deletedCnt++;
	}
	while (!preprocessQ.empty()) {
		delete preprocessQ.front();
		preprocessQ.pop();
		deletedCnt++;
	}
	while (!postprocessQ.empty()) {
		delete postprocessQ.front();
		postprocessQ.pop();
		deletedCnt++;
	}
	//assert(deletedCnt == NUM_BUFFERS);
}


PipelineBuffer *PipelineManager::getEmptyBuffer() {
	PipelineBuffer *buff = NULL;
	while (!buff) { // block until a buffer is available
		lock_guard<mutex> *lockg = new lock_guard<mutex>(emptyQ_mtx);
		if (!emptyQ.empty()) {
			buff = emptyQ.front();
			emptyQ.pop();
		}
		delete lockg;
	}
	return buff;
}

void PipelineManager::releaseCapture(PipelineBuffer *buff) {
	preprocessQ_mtx.lock();
	preprocessQ.push(buff);
	preprocessQ_mtx.unlock();
}

PipelineBuffer *PipelineManager::getPreProcessBuffer() {
	PipelineBuffer *buff = NULL;
	while (!buff) { // block until a buffer is available
		lock_guard<mutex> *lockg = new lock_guard<mutex>(preprocessQ_mtx);
		if (!preprocessQ.empty()) {
			buff = preprocessQ.front();
			preprocessQ.pop();
		}
		delete lockg;
	}
	return buff;
}

void PipelineManager::releaseProcess(PipelineBuffer *buff) {
	postprocessQ_mtx.lock();
	postprocessQ.push(buff);
	postprocessQ_mtx.unlock();
}

PipelineBuffer *PipelineManager::getPostProcessBuffer() {
	PipelineBuffer *buff = NULL;
	while (!buff) { // block until a buffer is available
		lock_guard<mutex> *lockg = new lock_guard<mutex>(postprocessQ_mtx);
		if (!postprocessQ.empty()) {
			buff = postprocessQ.front();
			postprocessQ.pop();
		}
		delete lockg;
	}
	return buff;
}

void PipelineManager::releaseDispose(PipelineBuffer *buff) {
	emptyQ_mtx.lock();
	emptyQ.push(buff);
	emptyQ_mtx.unlock();
}
