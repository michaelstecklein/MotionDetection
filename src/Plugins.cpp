#include <thread>
#include <unistd.h>
#include <stdlib.h>
#include "Plugins.hpp"



#ifdef RASPBERRYPI
	#define TOGGLE_LED
#elif BEAGLEBONEBLACK
	#define TOGGLE_LED
#endif




#ifdef TOGGLE_LED

void turn_on_led(void) {
	#ifdef RASPBERRYPI
		system("sudo echo none | sudo dd of=/sys/class/leds/led0/trigger");
		system("sudo echo 1 | sudo dd of=/sys/class/leds/led0/brightness");
	#endif
	#ifdef BEAGLEBONEBLACK
		// TODO
	#endif
}

void turn_off_led(void) {
	#ifdef RASPBERRYPI
		system("sudo echo none | sudo dd of=/sys/class/leds/led0/trigger");
		system("sudo echo 0 | sudo dd of=/sys/class/leds/led0/brightness");
	#endif
	#ifdef BEAGLEBONEBLACK
		// TODO
	#endif
}

static bool delay_task_running = false;
static bool reset_delay = false;

static void delay_task(void) {
	delay_task_running = true;
	reset_delay = false;
	do {
		usleep(500000); // 1/2 second
	} while (!reset_delay)
	delay_task_running = false;
	turn_off_led();
}

thread delay_thread;

static void start_delay_task(void) {
	if (!delay_task_running) {
		delay_thread(delay_task);
	} else {
		reset_delay = true;
	}	
}

static void toggle_led(void) {
	turn_on_led();
	start_delay_task();
}

#endif /* TOGGLE_LED */





void plugins_external_signal(void) {
	#ifdef TOGGLE_LED
		toggle_led();
	#endif
}
