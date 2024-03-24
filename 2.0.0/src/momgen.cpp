#include <docopt/docopt.h>
#include <map>

#include "../lib/print.hpp"
#include "../lib/strops.hpp"
#include "../lib/momgen.hpp"

static const char *VERSION = "My Own Makefile GENerator, version 2.0.0";
static const char USAGE[] =
R"(My Own Makefile GENerator (v.2.0.0): a simple C/C++ makefile generator
created by anson <thesearethethingswesaw@gmail.com>

usage:
	momgen (-h | --help)
	momgen --version
	momgen [-acdefMmops] [--lib=<library_name>] <project_name>

options:
	-a, --all-warn	      include all, extra and pedantic error checking for CFLAGS
	-d, --debug	      include debugging symbols
	-e, --error	      include a flag to treat warnings as errors
	-f, --ofast	      force all optimizations that do not affect speed
	-M, --modern	      use the latest standard for C or C++
	-m, --multi	      generate a Makefile for a multi-file project
	-o, --opti	      attach "-O2" to C/CXXFLAGS
	-p, --cpp	      signal momgen to make a C++ Makefile rather than C
	-s, --osize	      force all optimizations that do not affect size
	--lib=<library_name>  includes an arbitrary library to link with when
			      compiling, e.g. "--lib=ncurses" for -lncurses

copyright (c) 2024, see LICENSE for related details
this program uses docopt for argument parsing <http://docopt.org/>)";

int main(int argc, char *argv[])
{
	// This program shouldn't function without arguments
	if(argc < 2) { PrintTo(std::cerr, argv[0], ": too few arguments, try \"--help\"\n"); return -1; }

	//static std::ofstream fileobj;
	static std::map<std::string, docopt::value> args;
	static std::string message, projectname;
	// docopt parsing, using the alternative parsing function so we can capture all
	// exceptions given to us
	try { args = docopt::docopt_parse(USAGE, { argv + 1, argv + argc }, true, VERSION); }
	catch(const docopt::DocoptLanguageError &e)
	{
		message = e.what();
		PrintTo(std::cerr, argv[0], ": DocoptLanguageError caught: ", ToLower(message), "\n");
		return -1;
	}
	catch(const docopt::DocoptArgumentError &e)
	{
		message = e.what();
		PrintTo(std::cerr, argv[0], ": ", ToLower(message), ", try \"--help\"\n");
		return -1;
	}
	catch(const docopt::DocoptExitHelp &e)    { Println(USAGE); return 0; }
	catch(const docopt::DocoptExitVersion &e) { Println(VERSION); return 0; }

	// The argument vector represented as a map allows for quick querying that
	// makes sense when reading, plus I don't want to argue with docopt
	projectname = args.at("<project_name>").asString();

	std::string returns = AsString(MakeDirVars(), BuildRule(true, true, true), "\n", OtherRule(true, true));
	Println(returns);

	// TODO uncomment when ready to delete a bunch of test files
	//fileobj.open(projectname, std::ios::out);
	//if(!fileobj.good()) { std::perror(argv[0]); return -1; }

	// Clean up
	//fileobj.close();
	return 0;
}
