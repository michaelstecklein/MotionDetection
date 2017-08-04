#include <stdio.h>
#include "ThreadManager.hpp"
#include "PipelineBuffer.hpp"
#include "ImageSaver.hpp"


void dispose(PipelineBuffer *buff) {
	printf("Disposing %i   %i     %i\n", buff->buffer_id, buff->image_id,buff->save_image);

#ifndef SAVE_ALL_IMAGES
	if (buff->save_image)
#endif /* SAVE_ALL_IMAGES */
	saveRawImage(*buff);
	
	buff->preprocessed = false;
	buff->image_id = -1;
}
