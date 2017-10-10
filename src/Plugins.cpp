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
		system("sudo echo 1 | sudo dd of=/sys/class/leds/led0/brightness");
	#endif
	#ifdef BEAGLEBONEBLACK
		// TODO
	#endif
}

void turn_off_led(void) {
	#ifdef RASPBERRYPI
		system("sudo echo 0 | sudo dd of=/sys/class/leds/led0/brightness");
	#endif
	#ifdef BEAGLEBONEBLACK
		// TODO
	#endif
}

static void init_led(void) {
	#ifdef RASPBERRYPI
		system("sudo echo none | sudo dd of=/sys/class/leds/led0/trigger");
	#endif
	#ifdef BEAGLEBONEBLACK
		// TODO
	#endif
}

static bool led_on = false;

static void toggle_led(void) {
	if (led_on)
		turn_off_led();
	else
		turn_on_led();
	led_on = !led_on;
}

#endif /* TOGGLE_LED */





void plugins_external_signal(void) {
	#ifdef TOGGLE_LED
		toggle_led();
	#endif
}
