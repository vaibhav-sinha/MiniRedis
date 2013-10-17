#include "bitv.h"

bitv::bitv() {
	index = 0;
	data.push_back(false);
}

void bitv::set(int bit, int val) {
	if(bit <= index) {
		data[bit] = val;
	}
	else {
		for(int i=index+1; i<bit; i++) {
			data.push_back(false);
		}
		data.push_back(val);
		index = bit;
	}
}
