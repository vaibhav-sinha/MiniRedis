#include<string>
#include <pthread.h>

class info {
	public:
		int* t;
		std::string key;
};

extern void *expiry_callback(info*);

class timer {
	public:
	int time;
	timer();
	void run(std::string,int);
	void go(std::string);
};
