#include "timer.h"
#include<unistd.h>

timer::timer() {
	time = 0;
}

void timer::go(std::string key) {
	while(time>0) {
		usleep(1);
		time--;
	}
	//expiry_callback(key);
}

void timer::run(std::string key, int t) {
	info* i = new info;
	i->t = &time;
	i->key = key;
	time = t;
	pthread_t thread;
	pthread_create(&thread, NULL, expiry_callback, i);
}
