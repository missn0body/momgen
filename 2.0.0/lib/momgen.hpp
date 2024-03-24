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

std::string MakeDirVars(const std::array<std::string, 4> names = defnames)
{
	std::string ret;
	std::size_t i = 0;

	for(auto &str : { "BDIR", "ODIR", "SDIR", "DDIR" })
	{
		// B/O/S/DDIR = bin/obj/src/doc
		ret += AsString(MakeAssign(str, names.at(i++)), "\n");
	}

	ret += "\n";
	return ret;
}

std::string MakeSrcObj(bool IsCpp = false)
{
	/*
		SRCS = $(wildcard $(SDIR)/*.c/*.cpp)
		OBJS = $(patsubst $(SDIR)/%.c/%.cpp, $(ODIR)/%.o, $(SRCS))
	*/

	// TODO complete!
	std::string ret;
	return ret;
}

std::string OtherRule(bool IsMult = false, bool WantLint = false)
{
	std::string ret = AsString(MakeSection("Other"), "\n\n");
	if(WantLint)
	{
		// lint: $(SRCS)
		ret += AsString(ToLabel("lint"), " ", ToVar("SRCS"));
		// $(LINT) $(LINTFLAGS) $<
		ret += AsString("\n\t", ToVar("LINT"), " ", ToVar("LINTFLAGS"), " $<\n");
	}

	// clean:
	//	$(RM) -f
	ret += AsString(ToLabel("clean"), "\n\t", ToVar("RM"), " -f ");

	if(IsMult) ret += AsString(ToVar("BDIR"), "/*", ToVar("ODIR"), "/*\n"); /**/
	else	   ret += AsString(ToVar("BIN"), " *.o\n");

	return ret;
}

std::string BuildRule(bool IsCpp = false, bool IsMult = false, bool HasLib = false)
{
	std::string ret, AllLabel, BinLabel, BinAct, ObjLabel, ObjAct, DirCheck = "";
	// all: $(BIN)
	AllLabel = AsString(ToLabel("all"), " ", ToVar("BIN"), "\n\n");

	if(IsMult)
	{
		// $(BIN): $(BDIR) $(ODIR) $(OBJS)
		BinLabel = AsString(ToLabel(BinVar), " ", ToVar("BDIR"), " ", ToVar("ODIR"), " ", ObjsVar, "\n");

		// $(CC/XX) $(C/XXFLAGS) $(OBJS) -o $(BDIR)/$@
		if(IsCpp) BinAct = AsString("\t", CXXVar, " ", ToVar("CXXFLAGS"), " ");
		else	  BinAct = AsString("\t", CCVar, " ", ToVar("CFLAGS"), " ");
		BinAct += AsString(ObjsVar, " -o ", ToVar("BDIR"), "/$@");

		// $(ODIR)/%.o: $(SDIR)/%.cpp (or %.c)
		ObjLabel = AsString(ToVar("ODIR"), "/%.", ToLabel(ext.at(2)), " ");
		if(IsCpp) ObjLabel += AsString(ToVar("SDIR"), "/%.cpp\n");
		else	  ObjLabel += AsString(ToVar("SDIR"), "/%.c\n");

		// Directory checks for multi-file projects
		DirCheck += AsString(ToLabel(ToVar("BDIR")), "\n\t mkdir $@\n");
		DirCheck += AsString(ToLabel(ToVar("ODIR")), "\n\t mkdir $@\n");
	}
	else
	{
		// $(BIN): $(OBJS)
		BinLabel = AsString(ToLabel(BinVar), " ", ObjsVar, "\n");

		// $(CC/XX) $(C/XXFLAGS) $(OBJS) -o $@
		if(IsCpp) BinAct = AsString("\t", CXXVar, " ", ToVar("CXXFLAGS"));
		else	  BinAct = AsString("\t", CCVar,  " ", ToVar("CFLAGS"));
		BinAct += AsString(" ", ObjsVar, " -o $@");

		// %.o: %.cpp (or %.c)
		ObjLabel = AsString("%.", ToLabel(ext.at(2)), " ");
		ObjLabel += (IsCpp) ? "%.cpp\n" : "%.c\n";
	}

	// $(CC/XX) $(C/XXFLAGS) -c $< -o $@
	if(IsCpp) ObjAct = AsString("\t", CXXVar, " ", ToVar("CXXFLAGS"), " ");
	else	  ObjAct = AsString("\t", CCVar, " ", ToVar("CCFLAGS"), " ");
	ObjAct += "-c $< -o $@";

	// Adding a library if it is necessary
	if(HasLib) BinAct += AsString(" ", ToVar("LIBFLAGS"));
	ret += AsString(MakeSection("Building"), "\n\n", AllLabel, BinLabel, BinAct, "\n\n", ObjLabel, ObjAct, "\n\n", DirCheck);
	return ret;
}

#endif /* momgen__momgen__hpp */
