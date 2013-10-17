#include "zclass.h"
#include <iostream>
#include <string>
#include <stdlib.h>

zclass::zclass() {
}

int zclass::add(std::string member, double score) {
	int ret = orig.insert(std::make_pair(member,score)).second;
	//flip = flip_map(orig);
	flip.insert(flip_pair(std::make_pair(member,score)));
	return ret;
}

int zclass::card() {
	return orig.size();
}

int zclass::count(double min, double max) {
	std::multimap<double,std::string>::iterator it_min;
	std::multimap<double,std::string>::iterator it_max;
	std::multimap<double,std::string>::iterator it;
	it_min = flip.lower_bound(min);
	it_max = flip.upper_bound(max);

	//if(it_min == it_max) return 1;

	if(it_min == flip.end()) return 0;

	int cnt = 0;
	for(it=it_min; it!=it_max; it++) {
		cnt++;
	}
	//return cnt + 1 - (it_max==flip.end());
	return cnt ;
}

std::string zclass::range(int min, int max) {
	std::multimap<double,std::string>::iterator it_min;
	std::multimap<double,std::string>::iterator it_max;
	std::multimap<double,std::string>::iterator it;
	std::string ret("");
	int mod_min,mod_max;
	int size = flip.size();

	if(min>=0) {
		mod_min = min;
	}
	else {
		mod_min = size + min;
	}

	if(max>=0) {
		mod_max = max;
	}
	else {
		mod_max = size + max;
	}

	if(mod_min > mod_max) return "";

	if(mod_min > size) return "";

	if(mod_min < 0) return "";

	if(mod_max < 0) return "";

	if(mod_max > size-1) {
		it_max = flip.end();
	}
	else {
		it_max = flip.begin();
		for(int j=0;j<=mod_max && (it_max != flip.end());j++) it_max++;
		//it_max = flip.begin() + mod_max + 1;
	}

	it_min = flip.begin();
	for(int k=0;k<mod_min;k++) it_min++;
	//it_min = flip.begin() + mod_min;

	int index = 0;
	for(it = it_min; it != it_max; it++) {
		ret.append(std::to_string(index));
		ret.append(") ");
		//ret.append(it.second);
		ret.append(it->second);
		ret.append("\n");
		index++;
	}
	return ret;
}
