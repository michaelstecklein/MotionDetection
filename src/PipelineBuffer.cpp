#include "PipelineBuffer.hpp"



PipelineBuffer::PipelineBuffer(const int buff_id) : buffer_id(buff_id) {
	preprocessed = false;
	save_image = false;
}

PipelineBuffer::~PipelineBuffer() {
}

void PipelineBuffer::saveImage(const char *path) {
	// TODO
}
