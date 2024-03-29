#ifndef momgen__momgen__hpp
#define momgen__momgen__hpp

#ifndef __cplusplus
#error This header file is only to be used with C++ code
#endif /* __cplusplus */

/*
	Crucial functions related to the operation of momgen.
	Made by anson in 2024, see LICENSE for related details
*/

#include "strops.hpp"
#include "utils.hpp"

// TODO start replacing these with parcels!

// TODO std::string MakeVars(bool IsCpp = false, bool IsMult = false, bool HasLib = false, bool WantLint = false);
std::string MakeDirVars	(const std::array<std::string, 4> names = defnames);
std::string MakeSrcObj	(bool IsCpp = false);
std::string BuildRule	(bool IsCpp = false, bool IsMult = false, bool HasLib = false);
// TODO std::string MakeDist(bool IsCpp = false);
std::string OtherRule	(bool IsCpp = false, bool IsMult = false, bool WantLint = false);

#endif /* momgen__momgen__hpp */
