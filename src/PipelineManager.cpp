#include <assert.h>
#include "PipelineBuffer.hpp"
#include "PipelineManager.hpp"

#define PB	PipelineBuffer

// Rather than comparing consecutive images, compare images with this seperation
#define PROCESS_SEPERATION	5


PipelineManager::PipelineManager() {
	for (int i = 0; i < NUM_BUFFERS; i++) {
		emptyQ.push_back( new PipelineBuffer(i) );
	}
}

PipelineManager::~PipelineManager() {
	int deletedCnt = 0;
	while (!emptyQ.empty()) {
		delete emptyQ.front();
		emptyQ.pop_front();
		deletedCnt++;
	}
	while (!preprocessQ.empty()) {
		delete preprocessQ.front();
		preprocessQ.pop_front();
		deletedCnt++;
	}
	while (!postprocessQ.empty()) {
		delete postprocessQ.front();
		postprocessQ.pop_front();
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
			emptyQ.pop_front();
		}
		delete lockg;
	}
	return buff;
}

void PipelineManager::releaseCapture(PipelineBuffer *buff) {
	preprocessQ_mtx.lock();
	preprocessQ.push_back(buff);
	preprocessQ_mtx.unlock();
}

void PipelineManager::checkPreProcessLength(void) {
	bool block = true;
	while (block) {
		lock_guard<mutex> *lockg = new lock_guard<mutex>(preprocessQ_mtx);
		if (preprocessQ.size() > PROCESS_SEPERATION) {
			block = false;
		}
		delete lockg;
	}
}

PipelineBuffer *PipelineManager::getNewPreProcessBuffer() {
	checkPreProcessLength();
	lock_guard<mutex> lockg(preprocessQ_mtx);
	return preprocessQ[PROCESS_SEPERATION];
}

PipelineBuffer *PipelineManager::getOldPreProcessBuffer() {
	checkPreProcessLength();
	PipelineBuffer *buff = NULL;
	while (!buff) { // block until a buffer is available
		lock_guard<mutex> *lockg = new lock_guard<mutex>(preprocessQ_mtx);
		if (!preprocessQ.empty()) {
			buff = preprocessQ.front();
			preprocessQ.pop_front();
		}
		delete lockg;
	}
	return buff;
}

void PipelineManager::releaseProcess(PipelineBuffer *buff) {
	postprocessQ_mtx.lock();
	postprocessQ.push_back(buff);
	postprocessQ_mtx.unlock();
}

PipelineBuffer *PipelineManager::getPostProcessBuffer() {
	PipelineBuffer *buff = NULL;
	while (!buff) { // block until a buffer is available
		lock_guard<mutex> *lockg = new lock_guard<mutex>(postprocessQ_mtx);
		if (!postprocessQ.empty()) {
			buff = postprocessQ.front();
			postprocessQ.pop_front();
		}
		delete lockg;
	}
	return buff;
}

void PipelineManager::releaseDispose(PipelineBuffer *buff) {
	emptyQ_mtx.lock();
	emptyQ.push_back(buff);
	emptyQ_mtx.unlock();
}
