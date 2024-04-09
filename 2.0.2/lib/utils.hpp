#ifndef momgen__utils__hpp
#define momgen__utils__hpp

#ifndef __cplusplus
#error This header file is only to be used with C++ code
#endif /* __cplusplus */

/*
	Seperate utilities and definitions for Makefile generation
	Made by anson in 2024, see LICENSE for related details
*/

#include <array>
#include "strops.hpp"
#include "parcel.hpp"

const std::string defname = "Makefile";
const std::array<std::string, 3> ext = { "c", "cpp", "o" };
const std::array<std::string, 4> defnames = { "bin", "obj", "src", "doc" };

// Some shorthands that I imagine will make the later work much more bareable
static inline std::string ToOpt(const std::string &what, bool longOpt = false)  { return AsString(longOpt ? "--" : "-", what); }
static inline std::string ToVar(const std::string &what) 			{ return AsString("$(", what, ")"); }
static inline std::string ToLabel(const std::string &what)			{ return AsString(what, ":"); }

static inline std::string MakeAssign(const std::string &first, const std::string &second) { return AsString(first, " = ", second); }
static inline std::string BindExt(const std::string &name, const std::string &what) 	  { return AsString(name, ".", what); }
static inline std::string MakeSection(const std::string &what)				  { return AsString("# ", what, " ", Mult("-", 45 - what.length())); }

const std::string BinVar  = ToVar("BIN");
const std::string ObjsVar = ToVar("OBJS");
const std::string CCVar	  = ToVar("CC");
const std::string CXXVar  = ToVar("CXX");

#endif /* momgen__utils__hpp */
