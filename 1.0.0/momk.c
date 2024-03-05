#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////////
// DEFINITIONS BEGIN HERE
//////////////////////////////////////////////////////////////////////////////

static const char *compiler_header[] =
{
	"# Simply replace CC with your own compiler",
	"# if it doesn't match the ones listed below.",

	"#CC = i586-pc-msdosdjgpp-gcc",
	"#CC = i686-w64-mingw32-gcc",
	"#CC = x86_64-w64-mingw32-gcc"
};

static const char *ext[]  = { ".c", ".cpp", ".o" };

static const char *mk_vars[] 	  = { "CC", "CFLAGS", "RM", "BDIR", "ODIR", "SDIR", "BIN", "OBJ" };
static const char *mk_labels[]	  = { ".PHONY: ", "all: ", "clean: " };

static const char *default_cc[]   = { "gcc", "g++" };
static const char *default_del	  = "rm";

static const char *default_dir[]  = { "bin/", "obj/", "lib/", "src/" };

static const char *warn_flags[]   = { "-Wall ", "-Wextra ", "-Wpedantic ", "-Werror " };
static const char *opt_flags[]	  = { "-O1 ", "-O2 ", "-O3 ", "-Os ", "-Ofast " };
static const char *debug_flag	  = "-g ";
static const char *other_flags[]  = { "-o", "-r" };

static const char *modern_c 	  = "-std=c2x ";
static const char *modern_cpp	  = "-std=c++23 ";

typedef unsigned short flag_t;
typedef struct momkcontext
{
	flag_t wants;
	char *project_name, *compiler, *compiler_flags, *clean;

} momk;

static momk *context = nullptr;

inline void   setbit(flag_t *input, short what) { *input |= what; }
inline void unsetbit(flag_t *input, short what) { *input &= ~what; }
inline bool 	test(flag_t *input, short what) { return ((*input & what) != 0); }

enum { IS_CPP    = (1 << 0), IS_MULTI      = (1 << 1), IS_MODERN = (1 << 2),
       DEBUG_SYM = (1 << 3), WARN_AS_ERROR = (1 << 4), OPTIMIZE  = (1 << 5),
       OPTI_FAST = (1 << 6), OPTI_SIZE	   = (1 << 7), ALL_WARN	 = (1 << 8) };

constexpr size_t buffer_size = 256;
constexpr size_t smaller_buf = (buffer_size / 4);

//////////////////////////////////////////////////////////////////////////////
// FUNCTIONS BEGIN HERE
//////////////////////////////////////////////////////////////////////////////

// Helper functions
static inline void make_var   (char *dest, const char *src)			  { snprintf(dest, smaller_buf, "$(%s) ", src); }
static inline void make_label (char *dest, const char *src)			  { snprintf(dest, smaller_buf, "%s: ", src); }
static inline void make_equals(char *dest, const char *first, const char *second) { snprintf(dest, smaller_buf, "%s = %s", first, second ); }

void assemble_cc(char *buffer, flag_t *flags)
{
	if(flags == nullptr || buffer == nullptr) return;
	make_equals(buffer, mk_vars[1], test(flags, IS_CPP) ? default_cc[1] : default_cc[0]);
}

void assemble_ccflags(char *buffer, flag_t *flags)
{
	if(flags == nullptr) return;

	bool isplus = test(flags, IS_CPP);
	// Assemble the first part of the variable assignment
	// e.g. "CC = "
	snprintf(buffer, buffer_size, "%s = ", mk_vars[1]);
	// Then the gauntlet
	if(test(flags, ALL_WARN))
	{
		strcat(buffer, warn_flags[0]);
		strcat(buffer, warn_flags[1]);
		strcat(buffer, warn_flags[2]);
	}

	if	(test(flags, WARN_AS_ERROR))	    strncat(buffer, warn_flags[3], buffer_size);
	if	(test(flags, IS_MODERN) &&  isplus) strncat(buffer, modern_cpp, buffer_size);
	else if (test(flags, IS_MODERN) && !isplus) strncat(buffer, modern_c, buffer_size);
	if	(test(flags, DEBUG_SYM))	    strncat(buffer, debug_flag, buffer_size);
	if	(test(flags, OPTIMIZE))		    strncat(buffer, opt_flags[1], buffer_size);
	else if (test(flags, OPTI_SIZE))	    strncat(buffer, opt_flags[3], buffer_size);
	else if (test(flags, OPTI_FAST))	    strncat(buffer, opt_flags[4], buffer_size);

	return;
}

//void assemble_mk_labels(char *buffer, flag_t *flags) { return; }

void assemble_clean(char *buffer, const char *rm)
{
	if(buffer == nullptr || rm == nullptr) return;
	make_equals(buffer, mk_vars[2], rm);
}

void cleanup(void)
{
	if(context == nullptr) return;

	free(context->project_name);
	free(context->compiler);
	free(context->compiler_flags);
	free(context->clean);
	free(context);
}

int main(void)
{
	// Define defaults for the generation context
	context = malloc(sizeof(struct momkcontext));
	if(context == nullptr) { perror("momk"); exit(EXIT_FAILURE); }

	// Allocate memory for our compiler flags
	context->wants 		= 0x00;
	context->project_name	= malloc(11);
	context->compiler	= malloc(smaller_buf);
	context->compiler_flags = malloc(buffer_size);
	context->clean		= malloc(smaller_buf);

	if( context->compiler_flags == nullptr || context->project_name == nullptr ||
	    context->compiler == nullptr       || context->clean == nullptr )
		{ perror("momk"); exit(EXIT_FAILURE); }

	atexit(cleanup);

	// Set flags that the assemble_ccflags will check
	setbit(&context->wants, IS_MODERN);
	setbit(&context->wants, IS_CPP);
	setbit(&context->wants, DEBUG_SYM);
	setbit(&context->wants, ALL_WARN);
	// Then call it, passing in our buffer
	assemble_ccflags(context->compiler_flags, &context->wants);
	puts(context->compiler_flags);

	return 0;
}
