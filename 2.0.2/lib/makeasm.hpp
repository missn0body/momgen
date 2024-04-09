#ifndef momgen__momgen__hpp
#define momgen__momgen__hpp

#ifndef __cplusplus
#error This header file is only to be used with C++ code
#endif /* __cplusplus */

/*
	Crucial functions related to the operation of momgen.
	Made by anson in 2024, see LICENSE for related details
*/

#include "parcel.hpp"
#include "strops.hpp"
#include "utils.hpp"

std::string MakeVars	(parcel &in, const std::string &lib);
std::string MakeDirVars	(const std::array<std::string, 4> names = defnames);
std::string MakeSrcObj	(const parcel &in);
std::string BuildRule	(const parcel &in);
std::string MakeDist	(const parcel &in);
std::string OtherRule	(const parcel &in);

#endif /* momgen__momgen__hpp */
