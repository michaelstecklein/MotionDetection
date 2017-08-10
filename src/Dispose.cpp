#include <stdio.h>
#include "ThreadManager.hpp"
#include "PipelineBuffer.hpp"
#include "ImageSaver.hpp"


void dispose(PipelineBuffer *buff) {

#ifndef SAVE_ALL_IMAGES
	if (buff->save_image)
#endif /* SAVE_ALL_IMAGES */
	saveRawImage(*buff);

	if (buff->save_image) // TODO remove
		printf("MOTION DETECTED %i\n", buff->image_id);
	
	buff->preprocessed = false;
	buff->image_id = -1;
}
