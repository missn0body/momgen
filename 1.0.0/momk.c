#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////////
// DEFINITIONS BEGIN HERE
//////////////////////////////////////////////////////////////////////////////

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

inline void   setbit(flag_t *input, short what) { *input |= what; }
inline void unsetbit(flag_t *input, short what) { *input &= ~what; }
inline bool 	test(flag_t *input, short what) { return ((*input & what) != 0); }

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
	snprintf(buffer, line_buf, "CC = %s\nRM = rm", test(flags, IS_CPP) ? default_cc[1] : default_cc[0]);
}

void assemble_dirs(char *buffer, const char *project, flag_t *flags)
{
	if(buffer == nullptr || !test(flags, IS_MULTI)) return;

	char target[var_buf] = {0};

	strncpy(buffer, "BDIR = bin\nODIR = obj\nSDIR = src\n\n", buf_size);
	strncat(buffer, "SOURCE = $(shell find src/*.c -printf \"%f \")\n", buf_size);
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

int main(void)
{
	const char *project = "mohg";

	// Define defaults for the generation context
	context = malloc(sizeof(struct momkcontext));
	if(context == nullptr) { perror("momk"); exit(EXIT_FAILURE); }

	// Allocate memory for our compiler flags
	context->wants 		= 0x00;
	context->project_name	= malloc(var_buf);
	context->compiler	= malloc(line_buf);
	context->compiler_flags = malloc(buf_size);
	context->misc_labels	= malloc(buf_size);
	context->dirs		= nullptr;
	context->recipe		= malloc(buf_size);

	if( context->compiler_flags == nullptr	|| context->project_name == nullptr ||
	    context->compiler == nullptr	|| context->misc_labels == nullptr  ||
	    context->recipe == nullptr )
		{ perror("momk"); exit(EXIT_FAILURE); }

	atexit(cleanup);

	// Set flags that the assemble_ccflags will check
	setbit(&context->wants, IS_MODERN);
	//setbit(&context->wants, IS_MULTI);
	//setbit(&context->wants, IS_CPP);
	setbit(&context->wants, DEBUG_SYM);
	setbit(&context->wants, ALL_WARN);

	// Then call it, passing in our buffer
	strncpy(context->project_name, project, 11);
	assemble_cc(context->compiler, &context->wants);
	assemble_ccflags(context->compiler_flags, &context->wants);

	if(test(&context->wants, IS_MULTI))
	{
		context->dirs = malloc(buf_size);
		assemble_dirs(context->dirs, project, &context->wants);
	}

	assemble_misc_labels(context->misc_labels, project, &context->wants);
	assemble_recipe(context->recipe, project, &context->wants);

	// Final assembly
	for(size_t i = 0; i < header_length; i++) printf("%s", compiler_header[i]);
	puts(context->compiler);
	puts(context->compiler_flags);
	putchar('\n');

	if(context->dirs != nullptr)
	{
		puts(context->dirs);
		putchar('\n');
	}

	puts(context->misc_labels);
	putchar('\n');
	puts(context->recipe);

	return 0;
}
