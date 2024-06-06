#include <docopt/docopt.h>
#include <fstream>
#include <map>

#include "../lib/print.hpp"
#include "../lib/makeasm.hpp"

static const char *VERSION = "My Own Makefile GENerator, version 2.0.2";
static const char USAGE[] =
R"(My Own Makefile GENerator (v.2.0.2): a simple C/C++ makefile generator
created by anson <thesearethethingswesaw@gmail.com>

usage:
	momgen (-h | --help)
	momgen --version
	momgen [-aDdeflMmoPps] <project_name>
	momgen [-aDdeflMmoPps] <project_name> --lib=<library_name>

options:
	-a, --all-warn        include all, extra and pedantic error checking for CFLAGS
	-d, --debug           include debugging symbols
	-D, --dist            include a section for compressing and distributing binaries
	-e, --error           include a flag to treat warnings as errors
	-f, --ofast           force all optimizations that do not affect speed
	-l, --lint            generate a linting section in Makefile
	-M, --modern          use the latest standard for C or C++
	-m, --multi           generate a Makefile for a multi-file project
	-o, --opti            attach "-O2" to C/CXXFLAGS
	-P, --preview         output to stdout rather than a file
	-p, --cpp             signal momgen to make a C++ Makefile rather than C
	-s, --osize           force all optimizations that do not affect size
	--lib=<library_name>  includes an arbitrary library to link [default: ]

copyright (c) 2024, see LICENSE for related details
this program uses docopt for argument parsing <http://docopt.org/>)";

int main(int argc, char *argv[])
{
	// This program shouldn't function without arguments
	if(argc < 2) { Println(std::cerr, argv[0], ": too few arguments, try \"--help\""); return -1; }

	static std::ofstream fileobj;
	static std::map<std::string, docopt::value> args;
	static std::string message;
	// docopt parsing, using the alternative parsing function so we can capture all
	// exceptions given to us
	try { args = docopt::docopt_parse(USAGE, { argv + 1, argv + argc }, true, VERSION); }
	catch(const docopt::DocoptLanguageError &e)
	{
		message = e.what();
		Println(std::cerr, argv[0], ": DocoptLanguageError caught: ", ToLower(message));
		return -1;
	}
	catch(const docopt::DocoptArgumentError &e)
	{
		message = e.what();
		Println(std::cerr, argv[0], ": ", ToLower(message), ", try \"--help\"");
		return -1;
	}
	catch(const docopt::DocoptExitHelp &e)    { Println(USAGE); return 0; }
	catch(const docopt::DocoptExitVersion &e) { Println(VERSION); return 0; }

	// The argument vector represented as a map allows for quick querying that
	// makes sense when reading, plus I don't want to argue with docopt
	static const std::string projectname = args.at("<project_name>").asString();
	static const std::string libname = args.at("--lib").asString();
	static parcel set(projectname);

	auto b = [&](const auto &str) -> bool { return args.at(str).asBool() == true; };
	bool tostdout = b("--preview");

	if(b("--cpp"))       set.set(IS_CPP);
	if(b("--dist"))	     set.set(WANT_DIST);
	if(b("--multi"))     set.set(IS_MULTI);
	if(b("--modern"))    set.set(IS_MODERN);
	if(b("--debug"))     set.set(DEBUG_SYM);
	if(b("--error"))     set.set(WARN_AS_ERROR);
	if(b("--opti"))      set.set(OPTIMIZE);
	if(b("--ofast"))     set.set(OPTI_FAST);
	if(b("--osize"))     set.set(OPTI_SIZE);
	if(b("--all-warn"))  set.set(ALL_WARN);
	if(b("--lint"))	     set.set(WANT_LINT);
	if(!libname.empty()) set.set(HAS_LIB);

	if(!tostdout)
	{
		fileobj.open(defname, std::ios::out);
		if(!fileobj.good()) { std::perror(argv[0]); return -1; }
	}

	std::string returns = AsString(MakeVars(set, libname));
	if(set[IS_MULTI]) returns += AsString(MakeDirVars(), MakeSrcObj(set));
	returns += BuildRule(set);
	if(set[WANT_DIST]) returns += MakeDist(set);
	returns += OtherRule(set);

	if(tostdout) Println(returns);
	else
	{
		Println(fileobj, returns);
		Println("Be sure to replace the VERSION variable in your Makefile.");
	}

	// Clean up
	if(fileobj.is_open()) fileobj.close();
	return 0;
}
