#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////////
// DEFINITIONS BEGIN HERE
//////////////////////////////////////////////////////////////////////////////

static const char *VERSION = "1.0.0";

static constexpr size_t header_length = 5;
static const char *compiler_header[] =
{
	"# Simply replace CC with your own compiler\n",
	"# if it doesn't match the ones listed below.\n\n",
	"#CC = i586-pc-msdosdjgpp-g[cc/++]\n",
	"#CC = i686-w64-mingw32-g[cc/++]\n",
	"#CC = x86_64-w64-mingw32-g[cc/++]\n"
};

static const char *default_cc[] = { "gcc", "g++" };

static const char *warn_flags[] = { "-Wall ", "-Wextra ", "-Wpedantic ", "-Werror " };
static const char *opt_flags[]	= { "-O1 ", "-O2 ", "-O3 ", "-Os ", "-Ofast " };
static const char *debug_flag	= "-g ";

static const char *modern_c 	= "-std=c2x ";
static const char *modern_cpp 	= "-std=c++23 ";

typedef unsigned short flag_t;
typedef struct momkcontext
{
	flag_t wants;
	char *project_name;
	char *compiler;
	char *compiler_flags;
	char *misc_labels;
	char *dirs;
	char *recipe;

} momk;

static momk *context = nullptr;

inline void setbit(flag_t *input, short what) { *input |= what; }
inline bool   test(flag_t *input, short what) { return ((*input & what) != 0); }

enum { IS_CPP    = (1 << 0), IS_MULTI      = (1 << 1), IS_MODERN = (1 << 2),
       DEBUG_SYM = (1 << 3), WARN_AS_ERROR = (1 << 4), OPTIMIZE  = (1 << 5),
       OPTI_FAST = (1 << 6), OPTI_SIZE	   = (1 << 7), ALL_WARN	 = (1 << 8) };

constexpr size_t buf_size = 256;
constexpr size_t line_buf = 32;
constexpr size_t var_buf = 16;

//////////////////////////////////////////////////////////////////////////////
// FUNCTIONS BEGIN HERE
//////////////////////////////////////////////////////////////////////////////

void assemble_cc(char *buffer, flag_t *flags)
{
	if(flags == nullptr || buffer == nullptr) return;
	snprintf(buffer, line_buf, "CC = %s\nRM = rm\n", test(flags, IS_CPP) ? default_cc[1] : default_cc[0]);
}

void assemble_dirs(char *buffer, const char *project, flag_t *flags)
{
	if(buffer == nullptr || !test(flags, IS_MULTI)) return;

	char target[var_buf] = {0};

	strncpy(buffer, "BDIR = bin\nODIR = obj\nSDIR = src\n\n", buf_size);
	strncat(buffer, "SOURCE = $(shell find src/*.c -printf \"%f \")\n"/**/, buf_size);
	snprintf(target, var_buf, "TARGET = %s\n\n", project);
	strncat(buffer, target, buf_size);
	strncat(buffer, "BIN = $(TARGET:%=$(BDIR)/%)\nOBJ = $(SOURCE:%.c=$(ODIR)/%.o)", buf_size);
}

void assemble_ccflags(char *buffer, flag_t *flags)
{
	if(flags == nullptr) return;

	bool isplus = test(flags, IS_CPP);
	snprintf(buffer, buf_size, "CFLAGS = ");

	if(test(flags, ALL_WARN))
	{
		strcat(buffer, warn_flags[0]);
		strcat(buffer, warn_flags[1]);
		strcat(buffer, warn_flags[2]);
	}

	if	(test(flags, WARN_AS_ERROR))	    strncat(buffer, warn_flags[3], buf_size);
	if	(test(flags, IS_MODERN) &&  isplus) strncat(buffer, modern_cpp, buf_size);
	else if (test(flags, IS_MODERN) && !isplus) strncat(buffer, modern_c, buf_size);
	if	(test(flags, DEBUG_SYM))	    strncat(buffer, debug_flag, buf_size);
	if	(test(flags, OPTIMIZE))		    strncat(buffer, opt_flags[1], buf_size);
	else if (test(flags, OPTI_SIZE))	    strncat(buffer, opt_flags[3], buf_size);
	else if (test(flags, OPTI_FAST))	    strncat(buffer, opt_flags[4], buf_size);

	return;
}

void assemble_misc_labels(char *buffer, const char *project, flag_t *flags)
{
	if(buffer == nullptr || project == nullptr) return;
	snprintf(buffer, line_buf, ".PHONY: all clean\nall: %s\n", project);

	char format[line_buf] = {0};
	if(test(flags, IS_MULTI)) snprintf(format, buf_size, "clean:\n\t$(RM) -f $(OBJ) $(BIN)");
	else 			  snprintf(format, buf_size, "clean:\n\t$(RM) -f *.o %s\n", project);

	strncat(buffer, format, line_buf);
	return;
}

void assemble_recipe(char *buffer, const char *project, flag_t *flags)
{
	if(buffer == nullptr || project == nullptr) return;
	char obj_recipe[line_buf] = {0}, bin_recipe[line_buf] = {0};

	strncpy(obj_recipe, "\t$(CC) $^ -o $@ $(CFLAGS) -r\n", line_buf);
	strncpy(bin_recipe, "\t$(CC) $^ -o $@ $(CFLAGS)\n\n", line_buf);

	if(test(flags, IS_MULTI))
	{
		strncpy(buffer, "$(BIN): $(OBJ)\n", buf_size);
		strncat(buffer, bin_recipe, buf_size);
		strncat(buffer, "$(OBJ): $(ODIR)/%.o: $(SDIR)/%.c\n", buf_size);
		strncat(buffer, obj_recipe, buf_size);
	}
	else
	{
		char obj_line[line_buf] = {0}, bin_line[line_buf] = {0};

		snprintf(obj_line, line_buf, "%s.o: %s.c\n", project, project);
		snprintf(bin_line, line_buf, "%s: %s.o\n", project, project);

		strncpy(buffer, bin_line, buf_size);
		strncat(buffer, bin_recipe, buf_size);
		strncat(buffer, obj_line, buf_size);
		strncat(buffer, obj_recipe, buf_size);
	}

	return;
}

void cleanup(void)
{
	if(context == nullptr) return;

	free(context->project_name);
	free(context->compiler);
	free(context->compiler_flags);
	free(context->misc_labels);
	free(context->dirs);
	free(context->recipe);
	free(context);
}


void show_usage(void)
{
	printf("My Own MaKefile generator (%s): a stripped down, simplistic GNU Makefile generator\n", VERSION);
	puts("usage:\n\tmomk -h / --help");
	puts("\tmomk [-acdefMmops] project_name\n");

	printf("options:\n\t%5s\t%s\n",  "-a, --all-warn", 	"include all, extra and pedantic error checking for CFLAGS");
	printf("\t%5s\t%s\n",		  "-c, --cpp", 		"generate a C++ rather than C Makefile");
	printf("\t%5s\t%s\n",		  "-d, --debug", 	"adds \"-g\" in CFLAGS for debugging symbols");
	printf("\t%5s\t%s\n",		  "-e, --error", 	"adds \"-Werror\" in CFLAGS to treat warnings as errors");
	printf("\t%5s\t%s\n",		  "-f, --ofast", 	"adds \"-Ofast\" in CFLAGS to include all optimizations that don't affect program speed");
	printf("\t%5s\t%s\n",		  "-M, --modern", 	"sets compiler standard to the most recent possible (e.g. --std=c2x)");
	printf("\t%5s\t%s\n",		  "-m, --multi", 	"generates a multi-file Makefile for seperate sources and libraries");
	printf("\t%5s\t%s\n",		  "-o, --opti", 	"adds \"-O2\" in CFLAGS for a non-compromising level of optimization");
	printf("\t%5s\t%s\n",		  "-p, --preview", 	"prints output to stdout rather than a file");
	printf("\t%5s\t%s\n",		  "-s, --osize", 	"adds \"-Osize\" in CFLAGS to include all optimizations that don't bloat program size");

	printf("\n\"--opti\", \"--ofast\", and \"--osize\" are mutually exclusive.\n");
}

int main(int argc, char *argv[])
{
	if(argc < 2 || !*argv[1]) { fprintf(stderr, "%s: too few arguments. try \"--help\"\n", argv[0]); exit(EXIT_FAILURE); }
	FILE *default_stream = nullptr;

	// Define defaults for the generation context
	context = malloc(sizeof(struct momkcontext));
	if(context == nullptr) { perror(argv[0]); exit(EXIT_FAILURE); }

	// Allocate memory for our compiler flags
	context->wants 		= 0x00;
	context->project_name	= malloc(var_buf);
	context->compiler	= malloc(line_buf);
	context->compiler_flags = malloc(buf_size);
	context->misc_labels	= malloc(buf_size);
	context->dirs		= nullptr;
	context->recipe		= malloc(buf_size);

	// Memory check
	if( context->compiler_flags == nullptr	|| context->project_name == nullptr ||
	    context->compiler == nullptr	|| context->misc_labels == nullptr  ||
	    context->recipe == nullptr )
		{ perror(argv[0]); exit(EXIT_FAILURE); }

	// I don't want to repeat a bunch of free() statements
	// so I assign a function to do it for me, and valgrind
	// doesn't mind one bit
	atexit(cleanup);

	// Args parsing here
	int c;
	while(--argc > 0 && (*++argv)[0] != '\0')
	{
		if((*argv)[0] != '-')
		{
			if(context->project_name[0] != '\0')
			{
				fprintf(stderr, "%s: discarded program input -- \"%s\"\n", argv[0], *argv);
				continue;
			}

			strncpy(context->project_name, *argv, var_buf);
		}

		if((*argv)[0] == '-')
		{
			// If there's another dash, then it's a long option.
			// Move the pointer up 2 places and compare the word itself.
			if((*argv)[1] == '-')
			{
				// Using continue statements here so that the user
				// can use both single character and long options
				// simultaniously, and the loop can test both.
				if(strcmp((*argv) + 2, "help")     == 0) { show_usage(); exit(EXIT_SUCCESS); }
				if(strcmp((*argv) + 2, "modern")   == 0) { setbit(&context->wants, IS_MODERN);     continue; }
				if(strcmp((*argv) + 2, "cpp")      == 0) { setbit(&context->wants, IS_CPP);	   continue; }
				if(strcmp((*argv) + 2, "debug")    == 0) { setbit(&context->wants, DEBUG_SYM);     continue; }
				if(strcmp((*argv) + 2, "all-warn") == 0) { setbit(&context->wants, ALL_WARN);      continue; }
				if(strcmp((*argv) + 2, "multi")    == 0) { setbit(&context->wants, IS_MULTI);      continue; }
				if(strcmp((*argv) + 2, "error")    == 0) { setbit(&context->wants, WARN_AS_ERROR); continue; }
				if(strcmp((*argv) + 2, "stdout")   == 0) { default_stream = stdout; continue; }
				if(strcmp((*argv) + 2, "ofast")    == 0)
				{
					if(test(&context->wants, OPTIMIZE) || test(&context->wants, OPTI_SIZE)) continue;
					setbit(&context->wants, OPTI_FAST);
					continue;
				}
				if(strcmp((*argv) + 2, "osize")  == 0)
				{
					if(test(&context->wants, OPTIMIZE) || test(&context->wants, OPTI_FAST)) continue;
					setbit(&context->wants, OPTI_SIZE);
					continue;
				}
				if(strcmp((*argv) + 2, "opti")  == 0)
				{
					if(test(&context->wants, OPTI_FAST) || test(&context->wants, OPTI_SIZE)) continue;
					setbit(&context->wants, OPTIMIZE);
					continue;
				}
			}
			while((c = *++argv[0]))
			{
				// Single character option testing here.
				switch(c)
				{
					case 'h': show_usage(); exit(EXIT_SUCCESS);
					case 'M': setbit(&context->wants, IS_MODERN);  	  break;
					case 'c': setbit(&context->wants, IS_CPP);    	  break;
					case 'd': setbit(&context->wants, DEBUG_SYM);     break;
					case 'a': setbit(&context->wants, ALL_WARN);      break;
					case 'm': setbit(&context->wants, IS_MULTI);	  break;
					case 'e': setbit(&context->wants, WARN_AS_ERROR); break;
					case 'p': default_stream = stdout; break;
					case 'f':
						if(test(&context->wants, OPTIMIZE) || test(&context->wants, OPTI_SIZE)) break;
						setbit(&context->wants, OPTI_FAST);
						break;
					case 's':
						if(test(&context->wants, OPTIMIZE) || test(&context->wants, OPTI_FAST)) continue;
						setbit(&context->wants, OPTI_SIZE);
						continue;
					case 'o':
						if(test(&context->wants, OPTI_FAST) || test(&context->wants, OPTI_SIZE)) continue;
						setbit(&context->wants, OPTIMIZE);
						continue;
					// This error flag can either be set by a
					// completely unrelated character inputted,
					// or you managed to put -option instead of
					// --option.
					default: fprintf(stderr, "%s: unknown option\n", argv[0]); exit(EXIT_FAILURE);
				}
			}

			continue;
		}
	}

	if(default_stream == nullptr) default_stream = fopen("Makefile", "w");

	// Then call it, passing in our buffer
	assemble_cc(context->compiler, &context->wants);
	assemble_ccflags(context->compiler_flags, &context->wants);

	if(test(&context->wants, IS_MULTI))
	{
		context->dirs = malloc(buf_size);
		assemble_dirs(context->dirs, context->project_name, &context->wants);
	}

	assemble_misc_labels(context->misc_labels, context->project_name, &context->wants);
	assemble_recipe(context->recipe, context->project_name, &context->wants);

	// Final assembly
	for(size_t i = 0; i < header_length; i++) fprintf(default_stream, "%s", compiler_header[i]);
	fputs(context->compiler, default_stream);
	fputs(context->compiler_flags, default_stream);
	fputc('\n', default_stream);

	if(context->dirs != nullptr)
	{
		fputs(context->dirs, default_stream);
		fputc('\n', default_stream);
	}

	fputs(context->misc_labels, default_stream);
	fputc('\n', default_stream);
	fputs(context->recipe, default_stream);

	fclose(default_stream);
	return 0;
}
