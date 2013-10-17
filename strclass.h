#include<string>
#include<bitset>
#include<vector>

class strclass {
	public:
		std::string str;
		std::vector<std::bitset<8> > v;
		strclass();
		strclass(std::string);
		void set(std::string);
		bool setbit(int bit, int val);
		bool getbit(int bit);
};
