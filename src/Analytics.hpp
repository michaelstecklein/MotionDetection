#ifndef _analytics_hpp_
#define _analytics_hpp_

#include <atomic>
#include <stdbool.h>
using namespace std;

/*	If the macro 'ANALYTICS' is defined, this module with provide functionality
	so that the average time per buffer can be calculated for each pipeline 
	stage. The ThreadManager should discretize all therads and call these
	functions around each stage.	*/


#ifdef ANALYTICS
	#define ANALYTICS_NUM_ITERATIONS	200
	#define SAVE_ALL_IMAGES	// save all images for debug/analysis purposes
	void startCaptureAnalytics(void);
	void endCaptureAnalytics(void);
	void startProcessAnalytics(void);
	void endProcessAnalytics(void);
	void startDisposeAnalytics(void);
	void endDisposeAnalytics(void);
#else
	#define startCaptureAnalytics()		
	#define endCaptureAnalytics()		
	#define startProcessAnalytics()		
	#define endProcessAnalytics()		
	#define startDisposeAnalytics()		
	#define endDisposeAnalytics()		
#endif


#endif /* _analytics_hpp_ */
