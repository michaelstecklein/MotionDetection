#include <stdio.h>
#include "ThreadManager.hpp"
#include "PipelineBuffer.hpp"
#include "ImageSaver.hpp"
#include "Plugins.hpp"


void dispose(PipelineBuffer *buff) {

#ifndef SAVE_ALL_IMAGES
	if (buff->save_image)
#endif /* SAVE_ALL_IMAGES */
	saveRawImage(*buff);

	if (buff->save_image) {
		printf("MOTION DETECTED %i\n", buff->image_id); // TODO remove
		plugins_external_signal();
	}
	
	buff->preprocessed = false;
	buff->image_id = -1;
}
