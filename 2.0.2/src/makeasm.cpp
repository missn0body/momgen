#include "../lib/makeasm.hpp"

std::string MakeVars(parcel &in, const std::string &lib)
{
	bool IsCpp = in[IS_CPP], HasLib = in[HAS_LIB], WantLint = in[WANT_LINT];
	std::string ret, Compiler, Flags, Lib, Remove, LAssign, LFlags, BinName;

	// VERSION = <insert version number here>
	ret 	 = AsString(MakeAssign("VERSION", "<insert version number here>"), "\n");
	// CC/CXX = gcc/g++
	Compiler = IsCpp ? MakeAssign("CXX", "g++\n") : MakeAssign("CC", "gcc\n");
	// CC/CXXFLAGS = ...
	Flags    = AsString(MakeAssign(IsCpp ? "CXXFLAGS" : "CFLAGS", in.MakeFlags()), "\n");
	Lib      = (!HasLib || lib.empty()) ? "" : MakeAssign("LIBFLAGS", AsString("-l", lib, "\n"));
	Remove   = MakeAssign("RM", "rm\n");

	if(WantLint)
	{
		LAssign = MakeAssign("LINT", "cppcheck\n");
		LFlags  = MakeAssign("LINTFLAGS", "--check-level=exhaustive --enable=all --inconclusive --suppress=missingIncludeSystem --verbose\n");
	}

	BinName  = MakeAssign("BIN", in.project);
	ret += AsString(Compiler, Flags, Lib, Remove, LAssign, LFlags, "\n", BinName, "\n\n");
	return ret;
}

std::string MakeDirVars(const std::array<std::string, 4> names)
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

std::string MakeSrcObj(const parcel &in)
{
	bool IsCpp = in[IS_CPP];
	std::string ret, SrcsAssign, SrcsDef, ObjsAssign, ObjsDef;

	// $(wildcard $(SDIR)/*.c/*.cpp) /**/
	SrcsAssign = ToVar(AsString("wildcard ", ToVar("SDIR"), "/*", IsCpp ? ".cpp" : ".c")); /**/
	// $(patsubst $(SDIR)/%.c/%.cpp, $(ODIR)/%.o, $(SRCS))
	ObjsAssign = ToVar(AsString("patsubst ", ToVar("SDIR"), "/%", IsCpp ? ".cpp, " : ".c, ", ToVar("ODIR"), "/%.o, ", ToVar("SRCS")));

	SrcsDef = AsString(MakeAssign("SRCS", SrcsAssign), "\n"); // SRCS = ...
	ObjsDef = AsString(MakeAssign("OBJS", ObjsAssign), "\n"); // OBJS = ...

	ret = AsString(SrcsDef += ObjsDef, "\n");
	return ret;
}

std::string BuildRule(const parcel &in)
{
	bool IsCpp = in[IS_CPP], IsMult = in[IS_MULTI], HasLib = in[HAS_LIB];
	std::string ret, AllLabel, BinLabel, BinAct, ObjLabel, ObjAct, DirCheck = "";
	// all: $(BIN)
	AllLabel = AsString(ToLabel("all"), " ", BinVar, "\n\n");

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
		// $(BIN): $(BIN).o
		BinLabel = AsString(ToLabel(BinVar), " ", BinVar, ".o\n");

		// $(CC/XX) $(C/XXFLAGS) $(BIN).o -o $@
		if(IsCpp) BinAct = AsString("\t", CXXVar, " ", ToVar("CXXFLAGS"));
		else	  BinAct = AsString("\t", CCVar,  " ", ToVar("CFLAGS"));
		BinAct += AsString(" ", BinVar, ".o -o $@");

		// %.o: %.cpp (or %.c)
		ObjLabel = AsString("%.", ToLabel(ext.at(2)), " ");
		ObjLabel += (IsCpp) ? "%.cpp\n" : "%.c\n";
	}

	// $(CC/XX) $(C/XXFLAGS) -c $< -o $@
	if(IsCpp) ObjAct = AsString("\t", CXXVar, " ", ToVar("CXXFLAGS"), " ");
	else	  ObjAct = AsString("\t", CCVar, " ", ToVar("CFLAGS"), " ");
	ObjAct += "-c $< -o $@\n";

	// Adding a library if it is necessary
	if(HasLib) BinAct += AsString(" ", ToVar("LIBFLAGS"));
	ret += AsString(MakeSection("Building"), "\n\n", AllLabel, BinLabel, BinAct, "\n\n", ObjLabel, ObjAct, "\n", DirCheck, "\n");
	return ret;
}

std::string MakeDist(const parcel &in)
{
	bool IsMult = in[IS_MULTI];
	std::string ret = AsString(MakeSection("Distribution"), "\n\n");
	std::string DistLabels, DistAct, SelfLabels, SelfAct;

	// $(DDIR)/$(BIN).7
	std::string ManSource = AsString(ToVar("DDIR"), "/", ToVar("BIN"), ".7");
	std::string Compress = "gzip -vk $<", BinVersion = AsString(BinVar, "-", ToVar("VERSION"));

	/*
		dist: clean
		dist: all
		dist: $(DDIR)/$(BIN).7
	*/
	DistLabels = AsString(ToLabel("dist"), " clean\n", ToLabel("dist"), " all\n", ToLabel("dist"), " ", ManSource, "\n");

	/*
		self: clean
		self: all
		self: $(DDIR)/$(BIN).7
	*/
	SelfLabels = AsString(ToLabel("self"), " clean\n", ToLabel("self"), " all\n", ToLabel("self"), " ", ManSource, "\n");

	DistAct = AsString("\t", Compress, "\n");
	SelfAct = AsString("\t", Compress, "\n");

	// tar -czvf $(BIN)-$(VERSION).tar.gz $(BDIR)/$(BIN) $(DDIR)/* ../README.md ../LICENSE --transform 's,^,$(BIN)-$(VERSION)/,' */
	DistAct += AsString(
				"\ttar -czvf ",
				BinVersion, ".tar.gz ",
				IsMult ? AsString(ToVar("BDIR"), "/") : "", BinVar, " ",
				ToVar("DDIR"), "/*", // */
				"../README.md ../LICENSE --transform ",
				"'s,^,", BinVersion, "/,'\n"
			     );
	// md5sum $(BIN)-$(VERSION).tar.gz
	DistAct += AsString("\tmd5sum ", BinVersion, ".tar.gz\n\n");

	// sudo mkdir /usr/local/man/man7/ -p
	SelfAct += "\tsudo mkdir /usr/local/man/man7/ -p\n";
	// sudo mv $(BDIR)/$(BIN) /usr/local/bin/
	SelfAct += AsString("\tsudo mv ", IsMult ? AsString(ToVar("BDIR"), "/") : "", BinVar, " /usr/local/bin\n");
	// sudo mv $(DDIR)/$(BIN).7.gz /usr/local/man/man7/
	SelfAct += AsString("\tsudo mv ", ToVar("DDIR"), "/", BinVar, ".7.gz /usr/local/man/man7/\n");
	// sudo mandb -q
	SelfAct += "\tsudo mandb -q\n\n";

	ret += AsString(DistLabels, DistAct, SelfLabels, SelfAct);
	return ret;
}

std::string OtherRule(const parcel &in)
{
	bool IsCpp = in[IS_CPP], IsMult = in[IS_MULTI], WantLint = in[WANT_LINT];
	std::string ret = AsString(MakeSection("Other"), "\n\n");

	if(WantLint)
	{
		// lint: $(SRCS)
		ret += AsString(ToLabel("lint"), " ");
		if(IsMult) ret += AsString(ToVar("SRCS"));
		else	   ret += AsString(BinVar, (IsCpp) ? ".cpp" : ".c");
		// $(LINT) $(LINTFLAGS) $<
		ret += AsString("\n\t", ToVar("LINT"), " ", ToVar("LINTFLAGS"), " $<\n");
	}

	// clean:
	//	$(RM) -f
	ret += AsString(ToLabel("clean"), "\n\t", ToVar("RM"), " -f ");

	if(IsMult) ret += AsString(ToVar("BDIR"), "/* ", ToVar("ODIR"), "/*\n"); /**/
	else	   ret += AsString(BinVar, " *.o\n");

	return ret;
}
