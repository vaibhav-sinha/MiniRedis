#include "strclass.h"
#include <iostream>

strclass::strclass() {
}

strclass::strclass(std::string s) {
	str = s;
	for(int i=0; i<v.size(); i++) {
		v[i] = std::bitset<8>((str.c_str())[i]);
	}
	for(int i=v.size(); i<s.length(); i++) {
		v.push_back(std::bitset<8>((str.c_str())[i]));
	}
}

void strclass::set(std::string s) {
	str = s;
	for(int i=0; i<v.size(); i++) {
		v[i] = std::bitset<8>((str.c_str())[i]);
	}
	for(int i=v.size(); i<s.length(); i++) {
		v.push_back(std::bitset<8>((str.c_str())[i]));
	}
}

bool strclass::setbit(int bit, int val) {
	int I = bit/8;
	int i = bit%8;
	i = 7 - i;
	bool ret;
	if(I>=v.size()) {
		ret = 0;
	}
	else {
		ret = v[I][i];
	}
	for(int j=v.size(); j<=I; j++) {
		v.push_back(std::bitset<8>(std::string("00000000")));
		str.push_back('0');
	}
	v[I][i] = val!=0;
	str[I] = v[I].to_ulong();
	return ret;
}

bool strclass::getbit(int bit) {
	int I = bit/8;
	int i = bit%8;
	i = 7 - i;
	bool ret;
	if(I>=v.size()) {
		ret = 0;
	}
	else {
		ret = v[I][i];
	}
	return ret;
}
