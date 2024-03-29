#include "../lib/parcel.hpp"

void parcel::MakeFlags(void)
{
	std::string ret;

	if(this->flag.test(ALL_WARN)) 	   ret.append("-Wall -Wextra -Wpedantic ");
	if(this->flag.test(WARN_AS_ERROR)) ret.append("-Werror ");

	if	(this->flag.test(IS_MODERN) &&  this->flag.test(IS_CPP)) ret.append("-std=c++23 ");
	else if (this->flag.test(IS_MODERN) && !this->flag.test(IS_CPP)) ret.append("-std=c2x ");

	// These boolean one-liners are to test if only one flag is raised and the other
	// two are down, like a three-way toggle switch. The Makefile must only have
	// one optimization flag at a time, no more, no less
	auto test = [&](auto what) -> bool { return this->flag.test(what) == true; };
	bool only_opti = (test(OPTIMIZE)  && !test(OPTI_FAST) && !test(OPTI_SIZE));
	bool only_fast = (test(OPTI_FAST) && !test(OPTIMIZE)  && !test(OPTI_SIZE));
	bool only_size = (test(OPTI_SIZE) && !test(OPTIMIZE)  && !test(OPTI_FAST));

	if(only_opti) ret.append("-O2 ");
	if(only_fast) ret.append("-Ofast ");
	if(only_size) ret.append("-Osize ");

	if(this->flag.test(DEBUG_SYM)) ret.append("-g");

	// Some cleanup because Makefiles can break with trailing whitespace
	if(ret.back() == ' ') ret.pop_back();
	this->compiler_flags = ret;
}
