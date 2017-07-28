#include <execinfo.h>
#include <signal.h>
#include <unistd.h>
#include "PipelineManager.hpp"
#include "ThreadManager.hpp"
using namespace std;


int main(void) {
	runThreads();
	printf("done\n");
}
