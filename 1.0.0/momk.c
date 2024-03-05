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
	"#CC = i586-pc-msdosdjgpp-gcc\n",
	"#CC = i686-w64-mingw32-gcc\n",
	"#CC = x86_64-w64-mingw32-gcc\n"
};

static const char *ext[]  = { ".c", ".cpp" };

static const char *mk_vars[] 	  = { "CC", "CFLAGS", "RM", "BDIR", "ODIR", "SDIR", "BIN", "OBJ", "SOURCE" };
static const char *mk_labels[]	  = { ".PHONY", "all", "clean" };

static const char *default_cc[]   = { "gcc", "g++" };
static const char *default_del	  = "rm";

static const char *default_dir[]  = { "bin/", "obj/", "lib/", "src/" };

static const char *warn_flags[]   = { "-Wall ", "-Wextra ", "-Wpedantic ", "-Werror " };
static const char *opt_flags[]	  = { "-O1 ", "-O2 ", "-O3 ", "-Os ", "-Ofast " };
static const char *debug_flag	  = "-g ";

static const char *modern_c 	  = "-std=c2x ";
static const char *modern_cpp	  = "-std=c++23 ";

typedef unsigned short flag_t;
typedef struct momkcontext
{
	flag_t wants;
	char *project_name, *compiler, *compiler_flags, *rm, *labels;

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
static inline void make_var   (char *dest, const char *src) 			  { snprintf(dest, smaller_buf / 4, "$(%s)", src); }
static inline void make_equals(char *dest, const char *first, const char *second) { snprintf(dest, smaller_buf, "%s = %s", first, second ); }

void assemble_cc(char *buffer, flag_t *flags)
{
	if(flags == nullptr || buffer == nullptr) return;
	make_equals(buffer, mk_vars[0], test(flags, IS_CPP) ? default_cc[1] : default_cc[0]);
}

void assemble_rm(char *buffer, const char *rm)
{
	if(buffer == nullptr || rm == nullptr) return;
	make_equals(buffer, mk_vars[2], rm);
}

void assemble_mk_labels(char *buffer, const char *project, flag_t *flags)
{
	if(buffer == nullptr || project == nullptr) return;
	snprintf(buffer, smaller_buf, "%s: %s %s\n%s: %s\n", mk_labels[0], mk_labels[1], mk_labels[2], mk_labels[1], project);

	constexpr size_t var_size = (smaller_buf / 4);
	char format[smaller_buf] = {0}, rm_var[var_size] = {0}, bin_var[var_size] = {0}, obj_var[var_size] = {0};

	make_var(rm_var, mk_vars[2]);
	make_var(bin_var, mk_vars[6]);
	make_var(obj_var, mk_vars[7]);

	if(test(flags, IS_MULTI)) snprintf(format, smaller_buf, "%s:\n\t%s -rf %s %s", mk_labels[2], rm_var, bin_var, obj_var);
	else 			  snprintf(format, smaller_buf, "%s:\n\t%s -rf *.o %s\n", mk_labels[2], rm_var, project);

	strncat(buffer, format, smaller_buf);
	return;
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

void cleanup(void)
{
	if(context == nullptr) return;

	free(context->project_name);
	free(context->compiler);
	free(context->compiler_flags);
	free(context->rm);
	free(context->labels);
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
	context->project_name	= malloc(11);
	context->compiler	= malloc(smaller_buf);
	context->compiler_flags = malloc(buffer_size);
	context->rm		= malloc(smaller_buf);
	context->labels		= malloc(smaller_buf);

	if( context->compiler_flags == nullptr || context->project_name == nullptr ||
	    context->compiler == nullptr       || context->rm == nullptr	   ||
	    context->labels == nullptr )
		{ perror("momk"); exit(EXIT_FAILURE); }

	atexit(cleanup);

	// Set flags that the assemble_ccflags will check
	setbit(&context->wants, IS_MODERN);
	//setbit(&context->wants, IS_MULTI);
	setbit(&context->wants, IS_CPP);
	setbit(&context->wants, DEBUG_SYM);
	setbit(&context->wants, ALL_WARN);

	// Then call it, passing in our buffer
	strncpy(context->project_name, project, 11);
	assemble_cc(context->compiler, &context->wants);
	assemble_ccflags(context->compiler_flags, &context->wants);
	assemble_rm(context->rm, default_del);
	assemble_mk_labels(context->labels, project, &context->wants);

	// Final assembly
	for(size_t i = 0; i < header_length; i++) printf("%s", compiler_header[i]);
	puts(context->compiler);
	puts(context->compiler_flags);
	puts(context->rm);
	puts(context->labels);

	return 0;
}
