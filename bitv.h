#include <vector>
#include <deque>
#include <string>

class bitv {
	public:
		std::deque<bool> data;
		//std::vector<bool> data;
		int index;
		//std::string str;
		void set(int bit, int val);
		bitv();
};
