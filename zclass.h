#include <map>
#include <algorithm>
#include <iostream>

	template<typename A, typename B>
std::pair<B,A> flip_pair(const std::pair<A,B> &p)
{
	return std::pair<B,A>(p.second, p.first);
}

	template<typename A, typename B>
std::multimap<B,A> flip_map(const std::map<A,B> &src)
{
	std::multimap<B,A> dst;
	std::transform(src.begin(), src.end(), std::inserter(dst, dst.begin()), flip_pair<A,B>);
	return dst;
}

class zclass {
	public:
		std::map<std::string,double> orig;
		std::multimap<double,std::string> flip;
		zclass();
		int add(std::string member, double score);
		int card();
		int count (double min, double max);
		std::string range(int min, int max);
};
