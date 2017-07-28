#include "Analytics.hpp"
#ifdef ANALYTICS

#include <vector>
#include <ctime>
#include <mutex>
#include <stdlib.h>
#include <math.h>
using namespace std;


static mutex mtx;

static clock_t captureStart;
static vector<int> captureTimes;
static int numCaptures = 0;
static bool firstCapture = true; // skip first, has to do some initialization

static clock_t processStart;
static vector<int> processTimes;
static int numProcesses = 0;

static clock_t disposeStart;
static vector<int> disposeTimes;
static int numDisposes = 0;


static void calcAnalytics(vector<int>& times, double& outMean, double& outVar, int& outHigh, int& outLow) {
	int N = times.size();	
	if (N == 0)
		return;
	double sum;
	double sumSquares;
	outHigh = times.front();
	outLow = times.front();
	for (int time : times) {
		sum += (double)time;
		if (outHigh < time)
			outHigh = time;
		if (outLow > time)
			outLow = time;
	}
	outMean = sum / ((double) N);
	for (int time : times) {
		sumSquares += pow( time-outMean, 2);
	}
	outVar = sumSquares / ((double) N);
}

static float usToHz(double in) {
	return 1000000 / in;
}

static void calcAndPrintAnalytics(const char *stage, int num_iterations, vector<int>& times) {
	double mean;
	double variance;
	int high;
	int low;
	calcAnalytics(times, mean, variance, high, low);
	printf("\n");
	printf("%s :::\n",stage);
	printf("%i iterations\n",num_iterations);
	printf("mean:\t%.2fus, %.2fHz\n",mean,usToHz(mean));
	printf("var:\t%.2fus\n",variance);
	printf("high:\t%ius, %.2fHz\n",high,usToHz((float)high));
	printf("low:\t%ius, %.2fHz\n",low,usToHz((float)low));
}

static void calcAndPrintAnalytics() {
	printf("------------ Analytics ------------\n");
	printf("Number of iterations: %i\n",ANALYTICS_NUM_ITERATIONS);
	calcAndPrintAnalytics("Capture", numCaptures, captureTimes);
	calcAndPrintAnalytics("Process", numProcesses, processTimes);
	calcAndPrintAnalytics("Dispose", numDisposes, disposeTimes);
	printf("-----------------------------------\n");
}

static void checkIterationCount() {
	if(	numCaptures >= ANALYTICS_NUM_ITERATIONS	&&
		numProcesses >= ANALYTICS_NUM_ITERATIONS	&&
		numDisposes >= ANALYTICS_NUM_ITERATIONS ) {
		// print out analytics and kill program
		mtx.lock(); // stop any other threads from going past here
		calcAndPrintAnalytics();
		exit(0);
	}
}


void startCaptureAnalytics(void) {
	captureStart = clock();
}

void endCaptureAnalytics(void) {
	if (firstCapture) { // skip first, has to do some initialization
		firstCapture = false;
		return;
	}
	captureTimes.push_back(clock() - captureStart);
	numCaptures++;
	checkIterationCount();
}

void startProcessAnalytics(void) {
	processStart = clock();
}

void endProcessAnalytics(void) {
	processTimes.push_back(clock() - processStart);
	numProcesses++;
	checkIterationCount();
}

void startDisposeAnalytics(void) {
	disposeStart = clock();
}

void endDisposeAnalytics(void) {
	disposeTimes.push_back(clock() - disposeStart);
	numDisposes++;
	checkIterationCount();
}


#endif /* #ifdef ANALYTICS */
