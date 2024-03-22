#ifndef momgen__momgen__hpp
#define momgen__momgen__hpp

#ifndef __cplusplus
#error This header file is only to be used with C++ code
#endif /* __cplusplus */

/*
	Crucial functions related to the operation of momgen.
	Made by anson in 2024, see LICENSE for related details
*/

#include "includes.hpp"
#include "strops.hpp"

const std::string defname = "Makefile";
const std::array<std::string, 3> ext = { ".c", ".cpp", ".o" };

// Some shorthands that I imagine will make the later work much more bareable
static inline std::string ToOpt(const std::string &what, bool longOpt = false)  { return AsString(longOpt ? "--" : "-", what); }
static inline std::string ToVar(const std::string &what) 			{ return AsString("$(", what, ")"); }
static inline std::string ToLabel(const std::string &what)			{ return AsString(what, ": "); }

static inline std::string MakeAssign(const std::string &first, const std::string &second) { return AsString(first, " = ", second); }
static inline std::string BindExt(const std::string &name, const std::string &what) 	  { return AsString(name, ".", what); }

const std::string BinVar  = ToVar("BIN");
const std::string ObjsVar = ToVar("OBJS");
const std::string CCVar	  = ToVar("CC");
const std::string CXXVar  = ToVar("CXX");

std::string BuildRule(bool IsCpp = false, bool IsMult = false, bool HasLib = false)
{
	/*
		%.o: %.cpp (or %.c)
			$(CC) $(CFLAGS) -c $< -o $@

		For multi:

		$(ODIR)/%.o: $(SDIR)/%.cpp (or %.c)
			$(CC) $(CFLAGS) -c $< -o $@
	*/

	std::string ret, BinLabel, BinAct, ObjLabel, ObjAct;

	if(IsMult)
	{
		// $(BIN): $(BDIR) $(ODIR) $(OBJS)
		BinLabel = AsString(ToLabel(BinVar), " ", ToVar("BDIR"), " ", ToVar("ODIR"), " ", ObjsVar, "\n");
		// $(CC/XX) $(C/XXFLAGS) $(OBJS) -o $(BDIR)/$@
		if(IsCpp) BinAct = AsString("\t", CXXVar, " ", ToVar("CXXFLAGS"), " ", ObjsVar, " -o ", ToVar("BDIR"), "/$@\n");
	}
	else
	{
		// $(BIN): $(OBJS)
		BinLabel = AsString(ToLabel(BinVar), " ", ObjsVar, "\n");
		// $(CC/XX) $(C/XXFLAGS) $(OBJS) -o $@
		if(IsCpp) BinAct = AsString("\t", CXXVar, " ", ToVar("CXXFLAGS"), " ", ObjsVar, "-o $@\n");
	}

	if(HasLib) BinAct += AsString(" ", ToVar("LIBFLAGS"));
	ret += AsString(BinLabel, BinAct, ObjLabel, ObjAct);
	return ret;
}

#endif /* momgen__momgen__hpp */
