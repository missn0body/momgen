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

// TODO std::string MakeVars(bool IsCpp = false, bool IsMult = false, bool HasLib = false, bool WantLint = false);
std::string MakeDirVars	(const std::array<std::string, 4> names = defnames);
std::string MakeSrcObj	(const parcel &in);
std::string BuildRule	(const parcel &in);
std::string MakeDist	(void);
std::string OtherRule	(const parcel &in);

#endif /* momgen__momgen__hpp */
