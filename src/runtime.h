// ---------- RUNTIME HEADER ----------
#define _GNU_SOURCE

#ifndef __TINYC__
#define _FORTIFY_SOURCE 2
#endif

#include <stddef.h>
#include <wchar.h>
#include <wctype.h>
#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <locale.h>
#include <signal.h>
#include <sys/resource.h>
#include <time.h>
#include <sys/mman.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/stat.h>
#include <setjmp.h>
#include <stdbool.h>

#ifdef __TINYC__
#define __nmatch
#endif

#include <regex.h>

#define ALLOC_SIZE (1024 * 1024 * 1024)

#define PAGE_SIZE 4096
#define WORDSZ (sizeof(void*))

#define INITIAL_READ_SIZE 64
#define STACK_MARGIN_KB		50

#define CHECK_DEFINED(str, thing) if (!thing->defined) throw_error(#str" called before definition")

#define MEM_PROT PROT_READ|PROT_WRITE
#define MEM_FLAGS MAP_ANONYMOUS|MAP_PRIVATE|MAP_NORESERVE

typedef struct cognate_object ANY;
typedef ANY* restrict ANYPTR;
typedef ANY* restrict BOX;
typedef struct cognate_block BLOCK;
typedef _Bool BOOLEAN;
typedef double NUMBER;
typedef const char* restrict STRING;
typedef const struct cognate_list* restrict LIST;
typedef const char* restrict SYMBOL;
typedef struct cognate_file* restrict IO;
typedef const struct cognate_table* restrict TABLE;

typedef struct cognate_block
{
	void (*fn)(uint8_t*);
	uint8_t* env;
} cognate_block;

typedef enum cognate_type
{
	NIL = 0,
	number,
	symbol,
	boolean,
	string,
	box,
	list,
	table,
	io,
	block,
} cognate_type;

typedef struct cognate_object
{
	union
	{
		BOX box;
		BOOLEAN boolean;
		STRING string;
		LIST list;
		BLOCK block;
		SYMBOL symbol;
		NUMBER number;
		IO io;
		TABLE table;
	};
	cognate_type type;
} cognate_object;

#define MKEARLY(T) typedef struct _early_##T { T value ; _Bool defined ; } early_##T

MKEARLY(ANY);
MKEARLY(BOX);
MKEARLY(BLOCK);
MKEARLY(BOOLEAN);
MKEARLY(NUMBER);
MKEARLY(STRING);
MKEARLY(LIST);
MKEARLY(SYMBOL);

typedef struct cognate_table
{
	ANY key;
	ANY value;
	TABLE left;
	TABLE right;
	size_t level;
} cognate_table;

typedef struct cognate_list
{
	LIST next;
	ANY object;
} cognate_list;

typedef struct cognate_file
{
	STRING path;
	STRING mode;
	FILE* file;
} cognate_file;

typedef struct cognate_stack
{
	ANYPTR start; // Pointer to start.
	ANYPTR top;   // Pointer to top.
	ANYPTR absolute_start; // For the garbage collector
} cognate_stack;

#ifdef DEBUG

typedef struct backtrace
{
	const struct backtrace* restrict next;
	const char* name;
	const size_t line;
	const size_t col;
	const char* file;
	const char* line_str;
} backtrace;

typedef struct var_info
{
	const struct var_info* restrict next;
	const char* name;
	const ANY value;
} var_info;

#endif

#define PTR_MASK 0x0000ffffffffffff


#define ___name ___##name
#define SYM(name) ____##name
#define CALL(name, args) ___name args

#define SET(name, val) \
	if unlikely(pure) throw_error("Cannot mutate variable in pure function"); \
	___name = val;

#define unlikely(expr) (__builtin_expect((_Bool)(expr), 0))
#define likely(expr)	 (__builtin_expect((_Bool)(expr), 1))

static uintptr_t* space[2] = {NULL,NULL};
static char* bitmap[2] = {NULL,NULL};
static size_t alloc[2] = {0, 0};
static _Bool z = 0;

static _Bool pure = 0;


// Global variables
static cognate_stack stack;
static LIST cmdline_parameters = NULL;
static char* show_buffer = NULL;
#ifdef DEBUG
static const backtrace* trace = NULL;
static const var_info* vars = NULL;
#endif

_Bool breakpoints[10];

extern int main(int, char**);

static const char* restrict function_stack_start;

const SYMBOL SYMstart = "start";
const SYMBOL SYMend = "end";
const SYMBOL SYMcurrent = "current";
const SYMBOL SYMread = "read";
const SYMBOL SYMwrite = "write";
const SYMBOL SYMappend = "append";
const SYMBOL SYMreadHwrite = "read-write";
const SYMBOL SYMreadHappend = "read-append";
const SYMBOL SYMreadHwriteHexisting = "read-write-existing";

// Variables and	needed by functions.c defined in runtime.c
static void init_stack(void);
static void init_show_buffer(void);
static STRING show_object(const ANY object, char*, LIST);
static void _Noreturn __attribute__((format(printf, 1, 2))) throw_error_fmt(const char* restrict const, ...);
static void _Noreturn throw_error(const char* restrict const);
static ptrdiff_t compare_objects(ANY, ANY);
static _Bool match_objects(ANY, ANY);
static void destructure_lists(LIST, LIST);
static void destructure_objects(ANY, ANY);
#ifdef DEBUG
static void print_backtrace(int, const backtrace*, int);
#endif

static void* gc_malloc(size_t);
static void gc_collect(void);
static void gc_init(void);
static char* gc_strdup(char*);
static char* gc_strndup(char*, size_t);

// Variables and functions needed by compiled source file defined in runtime.c
static NUMBER unbox_NUMBER(ANY);
static BOX unbox_BOX(ANY);
static ANY box_BOX(BOX);
static ANY box_NUMBER(NUMBER);
static BOOLEAN unbox_BOOLEAN(ANY);
static ANY box_BOOLEAN(BOOLEAN);
static STRING unbox_STRING(ANY);
static ANY box_STRING(STRING);
static LIST unbox_LIST(ANY);
static ANY box_LIST(LIST);
static SYMBOL unbox_SYMBOL(ANY);
static ANY box_SYMBOL(SYMBOL);
static BLOCK unbox_BLOCK(ANY);
static ANY box_BLOCK(BLOCK);
static IO unbox_IO(ANY);
static ANY box_IO(IO);
static TABLE unbox_TABLE(ANY);
static ANY box_TABLE(TABLE);

static NUMBER radians_to_degrees(NUMBER);
static NUMBER degrees_to_radians(NUMBER);

static void cleanup(void);
static void push(ANY);
static ANY pop(void);
static ANY peek(void);
static int stack_length(void);

// Builtin functions needed by compiled source file defined in functions.c
static TABLE ___insert(ANY, ANY, TABLE);
static LIST ___empty(void);
static ANY ___if(BOOLEAN, ANY, ANY);
static void ___put(ANY);
static void ___print(ANY);
static NUMBER ___P(NUMBER, NUMBER);
static NUMBER ___M(NUMBER, NUMBER);
static NUMBER ___H(NUMBER, NUMBER);
static NUMBER ___S(NUMBER, NUMBER);
static NUMBER ___C(NUMBER, NUMBER);
static NUMBER ___modulo(NUMBER, NUMBER);
static NUMBER ___sqrt(NUMBER);
static NUMBER ___random(NUMBER, NUMBER);
static void ___clear(void);
static BOOLEAN ___true(void);
static BOOLEAN ___false(void);
static BOOLEAN ___or(BOOLEAN, BOOLEAN);
static BOOLEAN ___and(BOOLEAN, BOOLEAN);
static BOOLEAN ___xor(BOOLEAN, BOOLEAN);
static BOOLEAN ___not(BOOLEAN);
static BOOLEAN ___EE(ANY, ANY);
static BOOLEAN ___XE(ANY, ANY);
static BOOLEAN ___L(NUMBER, NUMBER);
static BOOLEAN ___G(NUMBER, NUMBER);
static BOOLEAN ___LE(NUMBER, NUMBER);
static BOOLEAN ___GE(NUMBER, NUMBER);
static BOOLEAN ___match(ANY, ANY);
static BOOLEAN ___numberQ(ANY);
static BOOLEAN ___symbolQ(ANY);
static BOOLEAN ___listQ(ANY);
static BOOLEAN ___stringQ(ANY);
static BOOLEAN ___blockQ(ANY);
static BOOLEAN ___booleanQ(ANY);
static BOOLEAN ___integerQ(ANY);
static BOOLEAN ___ioQ(ANY);
static BOOLEAN ___zeroQ(ANY);
static ANY ___first(ANY);
static ANY ___rest(ANY);
static STRING ___first_STRING(STRING);
static STRING ___rest_STRING(STRING);
static ANY ___first_LIST(LIST);
static LIST ___rest_LIST(LIST);
static STRING ___head(STRING);
static STRING ___tail(STRING);
static LIST ___push(ANY, LIST);
static BOOLEAN ___emptyQ(LIST);
static LIST ___list(BLOCK);
static STRING ___join(STRING, STRING);
static STRING ___substring(NUMBER, NUMBER, STRING);
static STRING ___input(void);
static IO ___open(SYMBOL, STRING);
static void ___close(IO);
static NUMBER ___number(STRING);
static STRING ___path(void);
static LIST ___stack(void);
static LIST ___parameters(void);
static void ___stop(void);
static STRING ___show(ANY);
static STRING ___show_NUMBER(NUMBER);
static STRING ___show_LIST(LIST);
static STRING ___show_BLOCK(BLOCK);
static STRING ___show_TABLE(TABLE);
static STRING ___show_IO(IO);
static STRING ___show_STRING(STRING);
static STRING ___show_SYMBOL(SYMBOL);
static STRING ___show_BOOLEAN(BOOLEAN);
static STRING ___show_BOX(BOX);

static BLOCK ___regex(STRING);
static BLOCK ___regexHmatch(STRING);
static NUMBER ___ordinal(STRING);
static STRING ___character(NUMBER);
static NUMBER ___floor(NUMBER);
static NUMBER ___round(NUMBER);
static NUMBER ___ceiling(NUMBER);
static NUMBER ___abs(NUMBER);
static void ___error(STRING);
//static BLOCK ___precompute(BLOCK);
static void ___wait(NUMBER);
static LIST ___split(STRING, STRING);
//static BLOCK ___remember(BLOCK);

static NUMBER ___sind(NUMBER);
static NUMBER ___cosd(NUMBER);
static NUMBER ___tand(NUMBER);
static NUMBER ___sin(NUMBER);
static NUMBER ___cos(NUMBER);
static NUMBER ___tan(NUMBER);

static NUMBER ___exp(NUMBER);
static NUMBER ___log(NUMBER, NUMBER);
static NUMBER ___ln(NUMBER);

static NUMBER ___asind(NUMBER);
static NUMBER ___acosd(NUMBER);
static NUMBER ___atand(NUMBER);
static NUMBER ___asin(NUMBER);
static NUMBER ___acos(NUMBER);
static NUMBER ___atan(NUMBER);

static NUMBER ___sinhd(NUMBER);
static NUMBER ___coshd(NUMBER);
static NUMBER ___tanhd(NUMBER);
static NUMBER ___sinh(NUMBER);
static NUMBER ___cosh(NUMBER);
static NUMBER ___tanh(NUMBER);

static const char *lookup_type(cognate_type);
static ptrdiff_t compare_lists(LIST, LIST);
static _Bool match_lists(LIST, LIST);
static void handle_error_signal(int, siginfo_t*, void *);
static void assert_impure(void);

#ifdef DEBUG
static _Bool debug = 0;
static size_t next_count = 0;
static size_t debug_lineno = 0;
#endif

static int _argc;
static char** _argv;

static void fn0(void);

int main(int argc, char** argv)
{
	function_stack_start = __builtin_frame_address(0);
	_argc = argc;
	_argv = argv;
	// Set locale for strings.
	if unlikely(setlocale(LC_ALL, "") == NULL)
	{
		throw_error("Cannot set locale");
	}
	// Init GC
	gc_init();
	// Seed the random number generator properly.
	struct timespec ts;
	if unlikely(clock_gettime(CLOCK_REALTIME, &ts) == -1)
	{
		throw_error("Cannot get system time");
	}
	srand(ts.tv_nsec ^ ts.tv_sec); // TODO make random more random.
	// Load parameters
	while (argc --> 1)
	{
		cognate_list* const tmp = gc_malloc (sizeof *tmp);
		tmp->object = box_STRING(argv[argc]);
		tmp->next = cmdline_parameters;
		cmdline_parameters = tmp;
	}
	// Bind error signals.
	struct sigaction error_signal_action;
   error_signal_action.sa_sigaction = handle_error_signal;
   sigemptyset(&error_signal_action.sa_mask);
   error_signal_action.sa_flags = SA_SIGINFO;
	char signals[] = { SIGHUP, SIGINT, SIGQUIT, SIGILL, SIGABRT, SIGBUS, SIGFPE, SIGPIPE, SIGTERM, SIGCHLD, SIGSEGV };
	for (size_t i = 0; i < sizeof(signals); ++i)
		if (sigaction(signals[i], &error_signal_action, NULL) == -1) throw_error("couldn't install signal handler");
	// Allocate buffer for object printing
	init_show_buffer();
	// Initialize the stack.
	init_stack();
#ifdef DEBUG
	if (getenv("COG_DEBUG")) debug=1;
#endif
	fn0();
	cleanup();
}

static void cleanup(void)
{
	if unlikely(stack.top != stack.start)
		throw_error_fmt("Exiting with %ti object(s) on the stack", stack.top - stack.start);
}

#ifdef DEBUG

#define BACKTRACE_PUSH(NAME, LINE, COL, FILE, LINE_STR, ID) \
	const backtrace _trace_##LINE##_##COL##_##ID = (backtrace) {.name = NAME, .line = (LINE), .col = (COL), .file = (FILE), .line_str=(LINE_STR), .next=trace}; \
	trace = &_trace_##LINE##_##COL##_##ID;

#define BACKTRACE_POP() \
	trace = trace->next;

/*
#define VARS_PUSH(NAME, CNAME, VALUE) \
	const var_info _varinfo_##CNAME = (var_info) {.name = NAME, .value = VALUE, .next=vars}; \
	vars = &_varinfo_##CNAME;

#define VARS_POP() \
	vars = vars->next;

static void debugger_step()
{
	if likely(!debug) return;
	if (next_count > 0)
	{
		next_count--;
		return;
	}
	print_backtrace(1, trace);
ask:
	fputs("\033[0;33m<DEBUG>\033[0m ", stderr);
	char buf[257] = {0};
	fgets(buf, 256, stdin);
	if (feof(stdin)) exit(EXIT_SUCCESS);
	if (*buf == '\n') goto ask;
	char op[65] = "\0";
	unsigned long int_arg = 0;
	char str_arg[129] = {0};
	sscanf(buf, "%64s %lu", op, &int_arg);
	sscanf(buf, "%64s %128s", op, str_arg);
	switch (*op)
	{
		case 'h': case 'H':
			// Help
			fputs("Usage:\n"
					"\tq       \t\033[0;1mquit\033[0m the debugger\n"
					"\th       \tshow this \033[0;1mhelp\033[0m message\n"
					"\ts [n]   \tshow (n items of) the \033[0;1mstack\033[0m\n"
					"\tc       \t\033[0;1mcontinue\033[0m execution\n"
					"\tr       \t\033[0;1mrestart\033[0m the program\n"
					"\tn [n]   \tgo to \033[0;1mnext\033[0m (n) tokens\n"
					"\tt [n]   \tprint (n items of) a back\033[0;1mtrace\033[0m\n"
					"\tl       \t\033[0;1mlist\033[0m the source program\n"
					"\tb [n]   \tset \033[0;1mbreakpoint\033[0m on line n\n"
					"\td [n]   \t\033[0;1mdelete\033[0m breakpoint on line n\n"
					"\tv [name]\tshow \033[0;1mvariable\033[0m of name\n", stderr);
			break;
		case 'r': case 'R':
			// Restart
			debug = 0;
			trace = NULL;
			vars = NULL;
			fn0(NULL);
			exit(EXIT_SUCCESS);
		case 's': case 'S':
			// Stack
			for (ANY* a = stack.top - 1;  a >= stack.start; --a)
			{
				fputs(show_object(*a, 0, NULL), stderr);
				fputc('\n', stderr);
			}
			break;
		case 'c': case 'C':
			// Continue
			debug = 0;
			return;
		case 'n': case 'N':
			// Next
			if (int_arg)
				next_count = int_arg - 1;
			return;
		case 't': case 'T':
			// Trace
			if (int_arg)
				print_backtrace(int_arg, trace);
			else print_backtrace(5, trace);
			break;
		case 'l': case 'L':
			// List TODO handle argument
			// TODO highlight current identifier like in traces
			for (size_t i = 0; source_file_lines[i]; ++i)
			{
				int broken = breakpoints[i];
				fprintf(stderr, "\033[0;2m[%3zi] %s\033[0m ", i+1, broken?"\033[0;33m*":" ");
				if (trace->line == i+1)
				{
					size_t len = strlen(trace->name);
					char* ln = source_file_lines[i];
					fprintf(stderr, "%.*s\033[0;1m%.*s\033[0;0m%s",
						(int)(trace->col - len - 1), ln,
						(int)len, ln + trace->col - len - 1,
						ln + trace->col - 1);
				}
				else fputs(source_file_lines[i], stderr);
				fputc('\n', stderr);
			}
			break;
		case 'q': case 'Q':
			// Quit
			fputs("Exiting...\n", stderr);
			exit (EXIT_SUCCESS);
		case 'b': case 'B':
			//if (int_arg > source_line_num) // TODO
			//	fprintf(stderr, "Line %zi is beyond end of file.\n", int_arg);
			if (int_arg) breakpoints[int_arg-1] = 1;
			else breakpoints[trace->line-1] = 1;
			break;
		case 'v': case 'V':;
			char* s = str_arg;
			for (size_t i = 0; i < strlen(s); ++i)
				s[i] = tolower(s[i]);
			if (!*s)
			{
				fputs("Usage: v [NAME]\n", stderr);
				break;
			}
			for (const var_info* restrict v = vars; v; v = v->next)
			{
				if (!strcmp(v->name, str_arg))
				{
					fprintf(stderr, "%c%s = %s\n", toupper(*s), s+1, show_object(v->value, 0, NULL));
					goto ask;
				}
			}
			fprintf(stderr, "No variable '%c%s' found\nNote: debug variables are dynamically scoped\n", toupper(*s), s+1);
			break;
		case 'd': case 'D':
			// Delete breakpoint
			if (!int_arg) fputs("Usage: d [LINE]\n", stderr);
			else breakpoints[int_arg-1] = 0;
			break;
		default:
			fprintf(stderr, "Invalid command '%s'\n", op);
			break;
	}
	goto ask;
}

static void check_breakpoint(size_t line)
{
	debug |= unlikely(breakpoints[line-1]);
}

*/

static void print_backtrace(int n, const backtrace* b, int last_spaces)
{
	if (!b || !n) return;
	int len = strlen(b->name);
	char* ln = strdup(b->line_str);
	char* tabs = ln;
	ssize_t col = b->col;
	while (*ln)
	{
		if (*ln != ' ' && *ln != '\t') break;
		ln++;
		col--;
	}
	for  ( ; *tabs ; tabs++) if (*tabs == '\t') *tabs = ' ';
	char pos[128];
	sprintf(pos, "[%s %zi:%zi]", b->file, b->line, b->col);
	int spaces = (strlen(pos)) + col - len/2 - 1;
	if (last_spaces)
	{
		fputs("\033[31;1m", stderr);
		if (last_spaces + 2 < spaces)
		{
			for (int i = 0 ; i < last_spaces+1 ; ++i) fputs(" ", stderr);
			fputs("\\", stderr);
			for (int i = last_spaces+1 ; i < spaces-2 ; ++i) fputs("_", stderr);
			fputs("\n", stderr);
			for (int i = 0 ; i < spaces-1 ; ++i) fputs(" ", stderr);
			fputs("\\\n", stderr);
		}
		else if (last_spaces > spaces + 2)
		{
			for (int i = 0 ; i < spaces+2 ; ++i) fputs(" ", stderr);
			for (int i = spaces+2 ; i < last_spaces-1 ; ++i) fputs("_", stderr);
			fputs("/\n", stderr);
			for (int i = 0 ; i < spaces+1 ; ++i) fputs(" ", stderr);
			fputs("/\n", stderr);
		}
		else
		{
			for (int i = 0 ; i < spaces-1 ; ++i) fputs(" ", stderr);
			if (last_spaces < spaces) fputs("\\\n", stderr);
			else if (last_spaces > spaces) fputs("  /\n", stderr);
			else fputs(" |\n", stderr);
		}
		fputs("\033[0m", stderr);
	}
	fprintf(stderr, "\033[0;2m%s\033[0m %.*s\033[31;1m%.*s\033[0m%s\n",
			pos,
			(int)(col - len - 1), ln,
			len, ln + col - len - 1,
			ln + col - 1);
	if (n <= 1)
	{
		for (int i = 0 ; i < spaces ; ++i) fputs(" ", stderr);
		fputs("\033[31;1m^\033[0m\n", stderr);
	}
	else print_backtrace(n - 1, b->next, spaces);
}
#endif

static _Noreturn __attribute__((format(printf, 1, 2))) void throw_error_fmt(const char* restrict const fmt, ...)
{
	char buf[1024];
	fputs("\n\n\033[31;1m    ", stderr);
	va_list args;
	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);
	fputs(buf, stderr);
	fputs("\n", stderr);
#ifndef DEBUG
	fputs("\n\033[0m", stderr);
#else
	print_backtrace(10, trace, strlen(buf)/2 + 4);
	/*
	if (isatty(fileno(stdin)))
	{
		debug = 1;
		debugger_step();
	} else print_backtrace(5, trace);
	*/
#endif
	exit(EXIT_FAILURE);
}

static _Noreturn void throw_error(const char* restrict const msg)
{
	throw_error_fmt("%s", msg);
}

static void handle_error_signal(int sig, siginfo_t *info, void *ucontext)
{
	if (sig == SIGSEGV)
	{
		char* addr = info->si_addr;
		if (addr >= (char*)stack.absolute_start && addr <= (char*)stack.absolute_start + ALLOC_SIZE)
			throw_error_fmt("Stack overflow (%zu items on the stack)", stack.top - stack.absolute_start);
		else
			throw_error("Memory error");
	}
	else throw_error_fmt("Recieved signal %i (%s)", sig, strsignal(sig));
}

static void assert_impure(void)
{
	if unlikely(pure) throw_error("Invalid operation for pure function");
}

cognate_table* table_skew(cognate_table* T)
{
	// input: T, a node representing an AA tree that needs to be rebalanced.
 	// output: Another node representing the rebalanced AA tree.
	if (!T) return NULL;
	else if (!T->left) return T;
	else if (T->left->level == T->level)
	{
      // Swap the pointers of horizontal left links.
		cognate_table* L = (cognate_table*)T->left;
		T->left = L->right;
		L->right = T; //L->right = table_skew(T);
		return L;
	}
	/*
	else if (T->right && T->right->left && T->right && T->right->left->level == T->right->level)
	{
		cognate_table* c = (cognate_table*)T->right;
		T->right = table_skew(c);
	}
	*/
	return T;
}

cognate_table* table_split(cognate_table* T)
{
	// input: T, a node representing an AA tree that needs to be rebalanced.
   // output: Another node representing the rebalanced AA tree.
	if (!T) return NULL;
	else if (!T->right || !T->right->right) return T;
	else if (T->level == T->right->right->level)
	{
		// We have two horizontal right links.  Take the middle node, elevate it, and return it.
		cognate_table* R = (cognate_table*)T->right;
		T->right = R->left;
		//R->right = table_split((cognate_table*)R->right);
		R->left = T;
		R->level++;
		return R;
	}
	else return T;
}

static char* show_table_helper(TABLE d, char* buffer, LIST checked)
{
	if (!d) return buffer;

	buffer = show_table_helper(d->left, buffer, checked);

	buffer = (char*)show_object(d->key, buffer, checked);
	*buffer++ = ':';
	buffer = (char*)show_object(d->value, buffer, checked);
	*buffer++ = ' ';

	buffer = show_table_helper(d->right, buffer, checked);

	return buffer;
}

static char* show_table(TABLE d, char* buffer, LIST checked)
{
	*buffer++ = '{';
	*buffer++ = ' ';
	buffer = show_table_helper(d, buffer, checked);
	*buffer++ = '}';
	*buffer = '\0';
	return buffer;
}

static char* show_io(IO i, char* buffer)
{
	if (i->file != NULL)
		return buffer + sprintf(buffer, "{ %s OPEN mode '%s' }", i->path, i->mode);
	else
		return buffer + sprintf(buffer, "{ %s CLOSED }", i->path);
}

static char* show_string(STRING s, char* buffer)
{
	*buffer++ = '"';
	for (const char* str = s ; *str ; ++str)
	{
		char c = *str;
		if unlikely(c >= '\a' && c <= '\r')
		{
			*buffer++ = '\\';
			*buffer++ = "abtnvfr"[c-'\a'];
		}
		else if (c == '\\') { *buffer++ = '\\'; *buffer++ = '\\'; }
		else if (c == '"')  { *buffer++ = '\\'; *buffer++ = '"';  }
		else *buffer++ = c;
	}
	*buffer++ = '"';
	*buffer = '\0';
	return buffer;
}

static char* show_number(NUMBER n, char* buffer)
{
	return buffer + sprintf(buffer, "%.14g", n);
}

static char* show_list(LIST l, char* buffer, LIST checked)
{
	*buffer++ = '(';
	for ( ; l ; l = l->next)
	{
		buffer = (char*)show_object(l->object, buffer, checked);
		if (!l->next) break;
		//*buffer++ = ',';
		*buffer++ = ' ';
	}
	*buffer++ = ')';
	*buffer = '\0';
	return buffer;
}

static char* show_boolean(BOOLEAN b, char* buffer)
{
	return buffer + sprintf(buffer, "%s", b ? "True" : "False");
}

static char* show_symbol(SYMBOL s, char* buffer)
{
	return buffer + sprintf(buffer, "%s", s);
}

static char* show_block(BLOCK b, char* buffer)
{
	void (*fn)(uint8_t*) = b.fn;
	return buffer + sprintf(buffer, "<block %p>", *(void**)&fn);
}

static char* show_box(BOX b, char* buffer, LIST checked)
{
	bool found = false;
	for (LIST l = checked ; l ; l = l->next)
		if (l->object.box == b)
		{
			*buffer++ = '.';
			*buffer++ = '.';
			*buffer++ = '.';
			goto end;
		}
	checked = (cognate_list*)___push(box_BOX(b), checked);
	*buffer++ = '[';
	buffer = (char*)show_object(*b, buffer, checked);
	*buffer++ = ']';
	checked = (cognate_list*)___rest_LIST(checked);
	end:
	*buffer = '\0';
	return buffer;
}

static STRING show_object (const ANY object, char* buffer, LIST checked)
{
	switch (object.type)
	{
		case NIL: throw_error("This shouldn't happen"); break;
		case number:  buffer = show_number (object.number,  buffer);          break;
		case io:      buffer = show_io     (object.io,      buffer);          break;
		case string:  buffer = show_string (object.string,  buffer);          break;
		case boolean: buffer = show_boolean(object.boolean, buffer);          break;
		case symbol:  buffer = show_symbol (object.symbol,  buffer);          break;
		case block:   buffer = show_block  (object.block,   buffer);          break;
		case table:   buffer = show_table  (object.table,   buffer, checked); break;
		case list:    buffer = show_list   (object.list,    buffer, checked); break;
		case box:     buffer = show_box    (object.box,     buffer, checked); break;
	}
	return buffer;
}

static void init_show_buffer(void)
{
	show_buffer = mmap(0, ALLOC_SIZE, MEM_PROT, MEM_FLAGS, -1, 0);
}

static void init_stack(void)
{
	stack.absolute_start = stack.top = stack.start
		= mmap(0, ALLOC_SIZE, MEM_PROT, MEM_FLAGS, -1, 0);
}

__attribute__((hot))
static void push(ANY object)
{
	*stack.top++ = object;
}

__attribute__((hot))
static ANY pop(void)
{
	if unlikely(stack.top == stack.start) throw_error("Stack underflow");
	return *--stack.top;
}

__attribute__((hot))
static ANY peek(void)
{
	if unlikely(stack.top == stack.start) throw_error("Stack underflow");
	return *(stack.top - 1);
}

static int stack_length(void)
{
	return stack.top - stack.start;
}

static const char* lookup_type(cognate_type type)
{
	switch(type)
	{
		case box:     return "box";
		case string:  return "string";
		case number:  return "number";
		case list:    return "list";
		case block:   return "block";
		case symbol:  return "symbol";
		case boolean: return "boolean";
		default:      return NULL;
	}
}

static ptrdiff_t compare_lists(LIST lst1, LIST lst2)
{
	if (!lst1) return -!!lst2;
	if (!lst2) return 1;
	ptrdiff_t diff;
	while (!(diff = compare_objects(lst1->object, lst2->object)))
	{
		if (!lst1->next) return -!!lst2->next;
		if (!lst2->next) return 1;
		lst1 = lst1 -> next;
		lst2 = lst2 -> next;
	}
	return diff;
}

static ptrdiff_t compare_blocks(BLOCK b1, BLOCK b2)
{
	if (b1.fn != b2.fn) return *(char**)&b1.fn - *(char**)&b2.fn;
	else return (char*)b1.env - (char*)b2.env;
}

static ptrdiff_t compare_numbers(NUMBER n1, NUMBER n2)
{
	double diff = n1 - n2;
	if (fabs(diff) <= 0.5e-14 * fabs(n1)) return 0;
	else return diff > 0 ? 1 : -1;
}

static ptrdiff_t compare_objects(ANY ob1, ANY ob2)
{
	if (ob1.type != ob2.type) return (ptrdiff_t)ob1.type - (ptrdiff_t)ob2.type;
	else switch (ob1.type)
	{
		case number:  return compare_numbers(unbox_NUMBER(ob1), unbox_NUMBER(ob2));
		case boolean: return (ptrdiff_t)unbox_BOOLEAN(ob1) - (ptrdiff_t)unbox_BOOLEAN(ob2);
		case string:  return (ptrdiff_t)strcmp(unbox_STRING(ob1), unbox_STRING(ob2));
		case symbol:  return unbox_SYMBOL(ob1) - unbox_SYMBOL(ob2);
		case list:    return compare_lists(unbox_LIST(ob1), unbox_LIST(ob2));
		case block:   return compare_blocks(unbox_BLOCK(ob1), unbox_BLOCK(ob2));
		case box:     return (char*)unbox_BOX(ob1) - (char*)unbox_BOX(ob2);
		default:      return 0; // really shouldn't happen
		/* NOTE
		 * The garbage collector *will* reorder objects in memory,
		 * which means that the relative addresses of blocks and boxes
		 * *will* change - which is why these can't be used to index tables
		 * for example.
		 */
	}
}

static _Bool match_lists(LIST lst1, LIST lst2)
{
	if (!lst1) return !lst2;
	if (!lst2) return 0;
	while (match_objects(lst1->object, lst2->object))
	{
		if (!lst1->next) return !lst2->next;
		if (!lst2->next) return 0;
		lst1 = lst1 -> next;
		lst2 = lst2 -> next;
	}
	return 0;
}

static void call_block(BLOCK b)
{
	b.fn(b.env);
}

static _Bool match_objects(ANY patt, ANY obj)
{
	cognate_type T = patt.type;
	if (T == block)
	{
		push (obj);
		call_block(unbox_BLOCK(patt));
		return unbox_BOOLEAN(pop());
	}
	else if (T != obj.type) return 0;
	switch (T)
	{
		case number:
			return fabs(unbox_NUMBER(patt) - unbox_NUMBER(obj))
				<= 0.5e-14 * fabs(unbox_NUMBER(patt));
		case boolean: return unbox_BOOLEAN(patt) == unbox_BOOLEAN(obj);
		case string:  return !strcmp(unbox_STRING(patt), unbox_STRING(obj));
		case symbol:  return unbox_SYMBOL(patt) == unbox_SYMBOL(obj);
		case list:    return match_lists(unbox_LIST(patt), unbox_LIST(obj));
		case box:     return match_objects(*unbox_BOX(patt), *unbox_BOX(obj));
		default:      return 0; // really shouldn't happen
	}
}

static void destructure_lists(LIST patt, LIST obj)
{
	if (!patt) return;
	destructure_lists(patt->next, obj->next);
	destructure_objects(patt->object, obj->object);
}

static void destructure_objects(ANY patt, ANY obj)
{
	if (patt.type == block)
	{
		push(obj);
		return;
	}
	switch (patt.type)
	{
		case list:   destructure_lists(unbox_LIST(patt), unbox_LIST(obj)); break;
		case box:    destructure_objects(*unbox_BOX(patt), *unbox_BOX(obj)); break;
		default:;
	}

}

static _Noreturn void type_error(char* expected, ANY got)
{
	char* s = "a";
	switch (expected[0])
		case 'a': case 'e': case 'i': case 'o': case 'u': case 'h':
			s = "an";
	throw_error_fmt("Expected %s %s but got %.64s", s, expected, ___show(got));
}

__attribute__((hot))
static NUMBER unbox_NUMBER(ANY box)
{
	if likely (box.type == number) return box.number;
	type_error("number", box);
	#ifdef __TINYC__
	return 0.0;
	#endif
}

__attribute__((hot))
static ANY box_NUMBER(NUMBER num)
{
	return (ANY) {.type = number, .number = num};
}

__attribute__((hot))
static BOX unbox_BOX(ANY b)
{
	if likely (b.type == box) return b.box;
	type_error("box", b);
	#ifdef __TINYC__
	return NULL;
	#endif
}

__attribute__((hot))
static ANY box_BOX(BOX b)
{
	return (ANY) {.type = box, .box = b};
}

__attribute__((hot))
static BOOLEAN unbox_BOOLEAN(ANY box)
{
	if likely (box.type == boolean) return box.boolean;
	type_error("boolean", box);
	#ifdef __TINYC__
	return 0;
	#endif
}

__attribute__((hot))
static ANY box_BOOLEAN(BOOLEAN b)
{
	return (ANY) {.type = boolean, .boolean = b};
}

__attribute__((hot))
static STRING unbox_STRING(ANY box)
{
	if likely (box.type == string) return box.string;
	type_error("string", box);
	#ifdef __TINYC__
	return NULL;
	#endif
}

__attribute__((hot))
static ANY box_STRING(STRING s)
{
	return (ANY) {.type = string, .string = s};
}

__attribute__((hot))
static LIST unbox_LIST(ANY box)
{
	if likely (box.type == list) return box.list;
	type_error("list", box);
	#ifdef __TINYC__
	return NULL;
	#endif
}

__attribute__((hot))
static ANY box_LIST(LIST s)
{
	return (ANY) {.type = list, .list = s};
}

__attribute__((hot))
static SYMBOL unbox_SYMBOL(ANY box)
{
	if likely (box.type == symbol) return box.symbol;
	type_error("list", box);
	#ifdef __TINYC__
	return NULL;
	#endif
}

__attribute__((hot))
static ANY box_SYMBOL(SYMBOL s)
{
	return (ANY) {.type = symbol, .symbol = s};
}

__attribute__((hot))
static BLOCK unbox_BLOCK(ANY box)
{
	if likely (box.type == block) return box.block;
	type_error("block", box);
	#ifdef __TINYC__
	return (struct cognate_block) {};
	#endif
}

/*
BLOCK block_copy(BLOCK b)
{
	size_t i = 0;
	for (; b->env[i] ; ++i);
	BLOCK B = gc_malloc(sizeof(b->fn) + (i + 1) * (sizeof b->env[0]) + i * sizeof(ANY));
	uint64_t* buf = (uint64_t*)B + i + 2;
	for (size_t i = 0; b->env[i] ; ++i)
	{
		B->env[i] = buf++;
		*(uint64_t*)(B->env[i]) = *(uint64_t*)(b->env[i]);
	}
	B->env[i] = NULL;
	B->fn = b->fn;
	return B;
}
*/
__attribute__((hot))
static ANY box_BLOCK(BLOCK s)
{
	return (ANY) {.type = block, .block = s};
}

__attribute__((hot))
static ANY box_IO(IO i)
{
	return (ANY) {.type = io, .io = i};
}


__attribute__((hot))
static IO unbox_IO(ANY box)
{
	if likely (box.type == io) return box.io;
	type_error("io", box);
	#ifdef __TINYC__
	return NULL;
	#endif
}

__attribute__((hot))
static ANY box_TABLE(TABLE d)
{
	return (ANY) {.type = table, .table = d};
}

__attribute__((hot))
static TABLE unbox_TABLE(ANY box)
{
	if likely (box.type == table) return box.table;
	type_error("table", box);
	#ifdef __TINYC__
	return NULL;
	#endif
}


#define EMPTY     0x0
#define ALLOC     0x1
#define FLATALLOC 0x2
#define FORWARD   0x3

static void gc_init(void)
{
	bitmap[0] = mmap(0, ALLOC_SIZE/8, MEM_PROT, MEM_FLAGS, -1, 0);
	bitmap[1] = mmap(0, ALLOC_SIZE/8, MEM_PROT, MEM_FLAGS, -1, 0);
	space[0]  = mmap(0, ALLOC_SIZE, MEM_PROT, MEM_FLAGS, -1, 0);
	space[1]  = mmap(0, ALLOC_SIZE, MEM_PROT, MEM_FLAGS, -1, 0);
	bitmap[0][0] = ALLOC;
	bitmap[1][0] = ALLOC;
}

__attribute__((noinline, hot, assume_aligned(sizeof(uint64_t)), returns_nonnull))
static void* gc_malloc(size_t sz)
{
	static ptrdiff_t interval = 1024l*1024l*10;
	interval -= sz;
	#ifndef GCTEST
	if unlikely(interval < 0)
	#endif
	{
		gc_collect();
		interval = 1024l*1024l*10l + alloc[z] * 6;
	}
	void* buf = space[z] + alloc[z];
	//assert(bitmap[z][alloc[z]] == ALLOC);
	alloc[z] += (sz + 7) / 8;
	//assert(!sz || bitmap[z][alloc[z]] == EMPTY);
	bitmap[z][alloc[z]] = ALLOC;
	//assert(!((ANY)buf & 7));
	return buf;
}

__attribute__((noinline, hot, assume_aligned(sizeof(uint64_t)), alloc_size(1), returns_nonnull))
static void* gc_flatmalloc(size_t sz)
{
	static ptrdiff_t interval = 1024l*1024l*10;
	interval -= sz;
	#ifndef GCTEST
	if unlikely(interval < 0)
	#endif
	{
		gc_collect();
		interval = 1024l*1024l*10l + alloc[z] * 6;
	}
	void* buf = space[z] + alloc[z];
	//assert(bitmap[z][alloc[z]] == ALLOC);
	bitmap[z][alloc[z]] = FLATALLOC;
	alloc[z] += (sz + 7) / 8;
	//assert(!sz || bitmap[z][alloc[z]] == EMPTY);
	bitmap[z][alloc[z]] = ALLOC;
	//assert(!((ANY)buf & 7));
	return buf;
}

__attribute__((hot))
static _Bool is_gc_ptr(uintptr_t object)
{
	return (uintptr_t*)object - space[!z] < alloc[!z];
}

__attribute__((hot))
static void gc_collect_root(uintptr_t* restrict addr)
{
	if (!is_gc_ptr(*addr)) return;
	struct action {
		uintptr_t from;
		uintptr_t* restrict to;
	};
	struct action* restrict act_stk_start = (struct action*)space[!z] + alloc[!z];
	struct action* restrict act_stk_top = act_stk_start;
	*act_stk_top++ = (struct action) { .from=*addr, .to=addr };
	while (act_stk_top-- != act_stk_start)
	{
		uintptr_t from = act_stk_top->from;
		uintptr_t* to = act_stk_top->to;
		const uintptr_t lower_bits = from & 7;
		uintptr_t index = (uintptr_t*)(from & ~7) - space[!z];
		ptrdiff_t offset = 0;
		while (bitmap[!z][index] == EMPTY) index--, offset++; // Ptr to middle of object
		if (bitmap[!z][index] == FORWARD)
			*to = lower_bits | (uintptr_t)((uintptr_t*)space[!z][index] + offset);
		else
		{
			_Bool flat = bitmap[!z][index] == FLATALLOC;
			//assert(bitmap[!z][index] == ALLOC);
			uintptr_t* buf = space[z] + alloc[z]; // Buffer in newspace
			//assert(bitmap[z][alloc[z]] == ALLOC);
			size_t sz = 1;
			for (;bitmap[!z][index+sz] == EMPTY;sz++);
			if (flat) bitmap[z][alloc[z]] = FLATALLOC;
			alloc[z] += sz;
			//assert(bitmap[z][alloc[z]] == EMPTY);
			bitmap[z][alloc[z]] = ALLOC;
			for (size_t i = 0;i < sz;i++)
			{
				uintptr_t from = space[!z][index+i];
				if (!flat && is_gc_ptr(from))
					*act_stk_top++ = (struct action) { .from=from, .to=buf+i };
				else buf[i] = from;
			}
			space[!z][index] = (uintptr_t)buf; // Set forwarding address
			bitmap[!z][index] = FORWARD;
			*to = lower_bits | (uintptr_t)(buf + offset);
		}
	}
}

static __attribute__((noinline,hot)) void gc_collect(void)
{
	/*
	printf("BEFORE:\n");
	for (int Z = 0 ; Z < 2 ; ++Z)
	{
		for (int i = 0 ; i <= alloc[Z] ; ++i) switch(bitmap[Z][i])
		{
			case EMPTY: fputc('-', stdout); break;
			case ALLOC: fputc('A', stdout); break;
			case FLATALLOC: fputc('a', stdout); break;
			case FORWARD: fputc('F', stdout); break; //printf("%zu", (uintptr_t*)space[Z][i] - space[!Z]); break;
		}
		if (Z == z) printf(" (active)");
		fputc('\n', stdout);
	}
	*/

	/*
	clock_t start, end;
	double cpu_time_used;
	size_t heapsz = alloc[z];
	start = clock();
	*/

	z = !z;
	memset(bitmap[z], EMPTY, alloc[z]+1);
	alloc[z] = 0;
	bitmap[z][0] = ALLOC;

	for (uintptr_t* root = (uintptr_t*)stack.absolute_start; root != (uintptr_t*)stack.top; ++root)
		gc_collect_root(root);

	jmp_buf a;
	if (setjmp(a)) return;

	for (uintptr_t* root = (uintptr_t*)&a; root < (uintptr_t*)function_stack_start; ++root)
		gc_collect_root(root); // Watch me destructively modify the call stack

	/*
	end = clock();
	printf("%lf seconds for %ziMB -> %ziMB\n", (double)(end - start) / CLOCKS_PER_SEC, heapsz * 8 /1024/1024, alloc[z] * 8 / 1024/1024);
	*/

	/*
	printf("AFTER:\n");
	for (int Z = 0 ; Z < 2 ; ++Z)
	{
		for (int i = 0 ; i <= alloc[Z] ; ++i) switch(bitmap[Z][i])
		{
			case EMPTY: fputc('-', stdout); break;
			case ALLOC: fputc('A', stdout); break;
			case FLATALLOC: fputc('a', stdout); break;
			case FORWARD: fputc('F', stdout); break; //printf("%zu", (uintptr_t*)space[Z][i] - space[!Z]); break;
		}
		if (Z == z) printf(" (active)");
		fputc('\n', stdout);
	}
	*/
	longjmp(a, 1);
}

static char* gc_strdup(char* src)
{
	const size_t len = strlen(src);
	return memcpy(gc_flatmalloc(len + 1), src, len + 1);
}

static char* gc_strndup(char* src, size_t bytes)
{
	const size_t len = strlen(src);
	if (len < bytes) bytes = len;
	char* dest = gc_flatmalloc(bytes + 1);
	dest[bytes] = '\0';
	return memcpy(dest, src, bytes);
}

static ANY ___if(BOOLEAN cond, ANY a, ANY b)
{
	return cond ? a : b;
}

static void ___put(ANY a)   { assert_impure(); fputs(___show(a), stdout); fflush(stdout); }
static void ___put_NUMBER(NUMBER a) { assert_impure(); fputs(___show_NUMBER(a), stdout); fflush(stdout); }
static void ___put_LIST(LIST a) { assert_impure(); fputs(___show_LIST(a), stdout); fflush(stdout); }
static void ___put_TABLE(TABLE a) { assert_impure(); fputs(___show_TABLE(a), stdout); fflush(stdout); }
static void ___put_IO(IO a) { assert_impure(); fputs(___show_IO(a), stdout); fflush(stdout); }
static void ___put_BLOCK(BLOCK a) { assert_impure(); fputs(___show_BLOCK(a), stdout); fflush(stdout); }
static void ___put_STRING(STRING a) { assert_impure(); fputs(___show_STRING(a), stdout); fflush(stdout); }
static void ___put_SYMBOL(SYMBOL a) { assert_impure(); fputs(___show_SYMBOL(a), stdout); fflush(stdout); }
static void ___put_BOOLEAN(BOOLEAN a) { assert_impure(); fputs(___show_BOOLEAN(a), stdout); fflush(stdout); }
static void ___put_BOX(BOX a) { assert_impure(); fputs(___show_BOX(a), stdout); }

static void ___print(ANY a) { assert_impure(); puts(___show(a)); }
static void ___print_NUMBER(NUMBER a) { assert_impure(); puts(___show_NUMBER(a)); }
static void ___print_LIST(LIST a) { assert_impure(); puts(___show_LIST(a)); }
static void ___print_TABLE(TABLE a) { assert_impure(); puts(___show_TABLE(a)); }
static void ___print_IO(IO a) { assert_impure(); puts(___show_IO(a)); }
static void ___print_BLOCK(BLOCK a) { assert_impure(); puts(___show_BLOCK(a)); }
static void ___print_STRING(STRING a) { assert_impure(); puts(___show_STRING(a)); }
static void ___print_SYMBOL(SYMBOL a) { assert_impure(); puts(___show_SYMBOL(a)); }
static void ___print_BOOLEAN(BOOLEAN a) { assert_impure(); puts(___show_BOOLEAN(a)); }
static void ___print_BOX(BOX a) { assert_impure(); puts(___show_BOX(a)); }

static NUMBER ___P(NUMBER a, NUMBER b) { return a + b; } // Add cannot produce NaN.
static NUMBER ___M(NUMBER a, NUMBER b) { return a * b; }
static NUMBER ___H(NUMBER a, NUMBER b) { return b - a; }
static NUMBER ___S(NUMBER a, NUMBER b) { return b / a; }
static NUMBER ___C(NUMBER a, NUMBER b) { return pow(b, a); }
static NUMBER ___modulo(NUMBER a, NUMBER b) { return b - a * floor(b / a); }
static NUMBER ___sqrt(NUMBER a) { return sqrt(a); }
static NUMBER ___random(NUMBER low, NUMBER high)
{
	if unlikely((high - low) < 0) goto invalid_range;
	else if (high - low < 1) return low;
	// This is not cryptographically secure btw.
	// Since RAND_MAX may only be 2^15, we need to do this:
	const long num
		= ((long)(short)rand())
		| ((long)(short)rand() << 15)
		| ((long)(short)rand() << 30)
		| ((long)(short)rand() << 45)
		| ((long)				rand() << 60);
	const double r = low + (NUMBER)(num % (unsigned long)(high - low));
	return r;
invalid_range:
	throw_error_fmt("Invalid range %.14g..%.14g", low, high);
	#ifdef __TINYC__
	return 0;
	#endif
}

static void ___clear(void) { stack.top=stack.start; }

static BOOLEAN ___true(void)  { return 1; }
static BOOLEAN ___false(void) { return 0; }
static BOOLEAN ___or(BOOLEAN a, BOOLEAN b) { return a || b; }
static BOOLEAN ___and(BOOLEAN a, BOOLEAN b)   { return a && b; }
static BOOLEAN ___xor(BOOLEAN a, BOOLEAN b) { return a ^ b;  }
static BOOLEAN ___not(BOOLEAN a)               { return !a;     }
static BOOLEAN ___EE(ANY a, ANY b)  { return !compare_objects(a,b); }
static BOOLEAN ___XE(ANY a, ANY b) { return compare_objects(a,b); }
static BOOLEAN ___G(NUMBER a, NUMBER b)  { return a < b; }
static BOOLEAN ___L(NUMBER a, NUMBER b)  { return a > b; }
static BOOLEAN ___GE(NUMBER a, NUMBER b) { return a <= b; }
static BOOLEAN ___LE(NUMBER a, NUMBER b) { return a >= b; }
static BOOLEAN ___numberQ(ANY a)  { return a.type==number; }
static BOOLEAN ___listQ(ANY a)    { return a.type==list;   }
static BOOLEAN ___stringQ(ANY a)  { return a.type==string; }
static BOOLEAN ___blockQ(ANY a)   { return a.type==block;  }
static BOOLEAN ___booleanQ(ANY a) { return a.type==boolean;}
static BOOLEAN ___symbolQ(ANY a)  { return a.type==symbol; }
static BOOLEAN ___ioQ(ANY a)      { return a.type==io; }
static BOOLEAN ___integerQ(ANY a) { return ___numberQ(a) && unbox_NUMBER(a) == floor(unbox_NUMBER(a)); }
static BOOLEAN ___zeroQ(ANY a)    { return ___numberQ(a) && unbox_NUMBER(a) == 0; }

static NUMBER  ___numberX(NUMBER a)  { return a; }
static LIST    ___listX(LIST a)      { return a; }
static STRING  ___stringX(STRING a)  { return a; }
static ANY     ___anyX(ANY a)        { return a; }
static BLOCK   ___blockX(BLOCK a)    { return a; }
static BOOLEAN ___booleanX(BOOLEAN a){ return a; }
static SYMBOL  ___symbolX(SYMBOL a)  { return a; }
static IO      ___ioX(IO a)          { return a; }

static BOOLEAN ___match(ANY patt, ANY obj) { return match_objects(patt,obj); }

static ANY ___first_LIST(LIST lst)
{
	// Returns the first element of a list. O(1).
	if unlikely(!lst) throw_error("empty list is invalid");
	return lst->object;
}

static LIST ___rest_LIST(LIST lst)
{
	// Returns the tail portion of a list. O(1).
	if unlikely(!lst) throw_error("empty list is invalid");
	return lst->next;
}

static STRING ___first_STRING(STRING str)
{
	if unlikely(!*str) throw_error("empty string is invalid");
	return gc_strndup((char*)str, mblen(str, MB_CUR_MAX));
}

static STRING ___rest_STRING(STRING str)
{
	if unlikely(!*str) throw_error("empty string is invalid");
	return str + mblen(str, MB_CUR_MAX);
}

static ANY ___first(ANY a)
{
	switch(a.type)
	{
		case list:   return ___first_LIST(a.list);
		case string: return box_STRING(___first_STRING(a.string));
		default: type_error("string or list", a);
	}
#ifdef __TINYC__
	return (cognate_object){0};
#endif
}

static ANY ___rest(ANY a)
{
	switch(a.type)
	{
		case list:   return box_LIST(___rest_LIST(a.list));
		case string: return box_STRING(___rest_STRING(a.string));
		default: type_error("string or list", a);
	}
#ifdef __TINYC__
	return (cognate_object){0};
#endif
}

static LIST ___push(ANY a, LIST b)
{
	// Pushes an object from the stack onto the list's first element. O(1).
	// TODO: Better name? Inconsistent with List where pushing to the stack adds to the END.
	cognate_list* lst = gc_malloc (sizeof *lst);
	*lst = (cognate_list) {.object = a, .next = b};
	return lst;
}

static BOOLEAN ___emptyQ(LIST lst)
{
	// Returns true is a list or string is empty. O(1).
	// Can be used to to write a Length function.
	return !lst;
}

static LIST ___list(BLOCK expr)
{
	ANYPTR tmp_stack_start = stack.start;
	stack.start = stack.top;
	// Eval expr
	call_block(expr);
	// Move to a list.
	cognate_list* lst = NULL;
	size_t len = stack_length();
	for (size_t i = 0; i < len; ++i)
	{
		cognate_list* l = gc_malloc(sizeof *l);
		l->object = stack.start[i];
		l->next = lst;
		lst = l;
	}
	stack.top = stack.start;
	stack.start = tmp_stack_start;
	return lst;
}

static STRING ___join(STRING s1, STRING s2)
{
	size_t l1 = strlen(s1);
	size_t l2 = strlen(s2);
	char* result = gc_malloc(l1 + l2 + 1);
	strcpy(result, s1);
	strcpy(result+l1, s2);
 	return result;
}

static STRING ___substring(NUMBER startf, NUMBER endf, STRING str)
{
	// O(end).
	// Only allocates a new string if it has to.
	/* TODO: Would it be better to have a simpler and more minimalist set of string functions, like lists do?
	 * The only real difference between NULL terminated strings and linked lists is that appending to strings is harder.
	 * Maybe also a 'Join N Str1 Str2 Str3 ... StrN' function.
	 */
	size_t start	= startf;
	size_t end		= endf;
	if unlikely(start != startf || end != endf || start > end) goto invalid_range;
	size_t str_size = 0;
	end -= start;
	for (;start != 0; --start)
	{
		if unlikely(!*str) goto invalid_range;
		str += mblen(str, MB_CUR_MAX);
	}
	for (;end != 0; --end)
	{
		if unlikely(str[str_size] == '\0') goto invalid_range;
		str_size += mblen(str+str_size, MB_CUR_MAX);
	}
	if unlikely(str[str_size] == '\0')
	{
		// We don't need to make a new string here.
		return str;
	}
	return gc_strndup((char*)str, str_size + 1);
invalid_range:
	throw_error_fmt("Invalid range %.14g..%.14g", startf, endf);
	#ifdef __TINYC__
	return NULL;
	#endif
}


static STRING ___input(void)
{
	// Read user input to a string.
	assert_impure();
	size_t size = 0;
	char* buf;
	size_t chars = getline(&buf, &size, stdin);
	char* ret = gc_strndup(buf, chars-1); // Don't copy trailing newline.
	free(buf);
	return ret;
}

static NUMBER ___number(STRING str)
{
	// casts string to number.
	char* end;
	NUMBER num = strtod(str, &end);
	if (end == str || *end != '\0') goto cannot_parse;
	return num;
cannot_parse:
	throw_error_fmt("Cannot parse '%.32s' to a number", str);
	#ifdef __TINYC__
	return 0;
	#endif
}

static STRING ___path(void)
{
	assert_impure();
	char buf[FILENAME_MAX];
	if (!getcwd(buf, FILENAME_MAX))
		throw_error("cannot get working directory");
	char* ret = gc_strdup(buf);
	return ret;
}

static LIST ___stack(void)
{
	LIST lst = NULL;
	for (size_t i = 0; i + stack.start < stack.top; ++i)
	{
		cognate_list* tmp = gc_malloc (sizeof *tmp);
		tmp -> object = stack.start[i];
		tmp -> next = lst;
		lst = tmp;
	}
	return lst;
}

static LIST ___parameters(void)
{
	return cmdline_parameters; // TODO should be a variable, and allow mutation and stuff
}

static void ___stop(void)
{
	assert_impure();
	// Don't check stack length, because it probably wont be empty.
	exit(EXIT_SUCCESS);
}

void apply_regex(uint8_t* env)
{
	regex_t reg = *(regex_t*)env;
	STRING str = unbox_STRING(pop());
	const int found = regexec(&reg, str, 0, NULL, 0);
	if unlikely(found != 0 && found != REG_NOMATCH)
		throw_error_fmt("Regex failed matching string '%.32s'", str);
	push(box_BOOLEAN(!found));

}

static BLOCK ___regex(STRING reg_str)
{
	regex_t* reg = gc_flatmalloc(sizeof *reg);
	const int status = regcomp(reg, reg_str, REG_EXTENDED | REG_NEWLINE | REG_NOSUB);
	errno = 0; // Hmmm
	if unlikely(status)
	{
		char reg_err[256];
		regerror(status, reg, reg_err, 256);
		throw_error_fmt("Compile error (%s) in regex '%.32s'", reg_err, reg_str);
	}

	return (cognate_block){ .env = (void*)reg, .fn = apply_regex };
}

void match_regex(uint8_t* env)
{
	regex_t* reg = (regex_t*)env;
	STRING str = unbox_STRING(pop());
	size_t groups = reg->re_nsub + 1;
	regmatch_t matches[groups];
	const int found = regexec(reg, str, groups, matches, 0);
	if unlikely(found != 0 && found != REG_NOMATCH) throw_error_fmt("Regex failed matching string '%.32s'", str);

	if (found == 0) {
		for (unsigned int g = 1; g < groups ; g++)
		{
			size_t from = matches[g].rm_so;
			if (from == (size_t)-1)
			{
				groups = g;
				break;
			}
		}

		for (unsigned int g = groups-1; g > 0; g--)
		{
			size_t from = matches[g].rm_so;
			size_t to = matches[g].rm_eo;

			char* item = gc_strndup((char*)str, to);
			push(box_STRING(item + from));
		}
	}
	push(box_BOOLEAN(found == 0));
}

static BLOCK ___regexHmatch(STRING reg_str)
{
	regex_t* reg = gc_flatmalloc(sizeof *reg);
	const int status = regcomp(reg, reg_str, REG_EXTENDED | REG_NEWLINE);
	errno = 0;
	if unlikely(status)
	{
		char reg_err[256];
		regerror(status, reg, reg_err, 256);
		throw_error_fmt("Compile error (%s) in regex '%.32s'", reg_err, reg_str);
	}

	return (cognate_block){ .env = (void*)reg, .fn = match_regex };
}


static NUMBER ___ordinal(STRING str)
{
	if unlikely(!str[0] || strlen(str) > (size_t)mblen(str, MB_CUR_MAX))
		throw_error_fmt("Invalid string '%.32s' (should be length 1)", str);
	wchar_t chr = 0;
	mbtowc(&chr, str, MB_CUR_MAX);
	return chr;
}

static STRING ___character(NUMBER d)
{
	const wchar_t i = d;
	char* const str = gc_flatmalloc (MB_CUR_MAX + 1);
	if unlikely(i != d || wctomb(str, i) == -1)
		throw_error_fmt("Cannot convert %.14g to UTF8 character", d);
	str[mblen(str, MB_CUR_MAX)] = '\0';
	return str;
}

static NUMBER ___floor(NUMBER a)
{
	return floor(a);
}

static NUMBER ___round(NUMBER a)
{
	return round(a);
}

static NUMBER ___ceiling(NUMBER a)
{
	return ceil(a);
}

static NUMBER ___abs(NUMBER a)
{
	return fabs(a);
}

static void ___error(STRING str)
{
	throw_error(str);
}

static void ___wait(NUMBER seconds)
{
	assert_impure();
	sleep(seconds);
}

/*
static BLOCK ___precompute(BLOCK blk)
{
	ANYPTR tmp_stack_start = stack.start;
	stack.start = stack.top;
	blk();
	const size_t len = stack_length();
	if (!len) return Block_copy(^{});
	ANYPTR ret_data = gc_malloc(len * sizeof *ret_data);
	for (size_t i = 0; i < len; ++i)
		ret_data[len] = stack.start[i];
	stack.top = stack.start;
	stack.start = tmp_stack_start;
	return Block_copy(^{
		for (size_t i = 0; i < len; ++i)
			push(ret_data[i]);
	});
}
*/

static STRING ___show(ANY o)
{
	if (o.type == string) return o.string;
	if (o.type == symbol) return o.symbol;
	show_object(o, show_buffer, NULL);
	return show_buffer;
}

static LIST ___split(STRING sep, STRING str)
{
	if (!*sep) throw_error("Seperator cannot be empty");
	LIST lst = NULL;
	size_t len = strlen(sep);
	char* found;
	while ((found = strstr(str, sep)))
	{
		found = strstr(str, sep);
		if (found != str)
		{
			char* item = gc_flatmalloc(found - str + 1);
			memcpy(item, str, found - str);
			item[found - str] = '\0';
			cognate_list* node = gc_malloc(sizeof *node);
			node->object = box_STRING(item);
			node->next = lst;
			lst = node;
		}
		str = found + len;
	}
	if (*str)
	{
		cognate_list* node = gc_malloc(sizeof *node);
		node->object = box_STRING(str);
		node->next = lst;
		lst = node;
	}
	cognate_list* prev = NULL;
	cognate_list* curr = (cognate_list*)lst;
	while (curr)
	{
		cognate_list* next = (cognate_list*)curr->next;
		curr->next = prev;
		prev = curr;
		curr = next;
	}
	return prev;
}

static STRING ___uppercase(STRING str)
{
	char* converted = gc_strdup((char*)str);
	int len = 0;
	for (char* c = converted; *c; c += len)
	{
		wchar_t chr = 0;
		len = mblen(c, MB_CUR_MAX);
		mbtowc(&chr, c, len);
		chr = towupper(chr);
		wctomb(c, chr);
	}
	return converted;
}

static STRING ___lowercase(STRING str)
{
	char* converted = gc_strdup((char*)str);
	int len = 0;
	for (char* c = converted; *c; c += len)
	{
		wchar_t chr = 0;
		len = mblen(c, MB_CUR_MAX);
		mbtowc(&chr, c, len);
		chr = towlower(chr);
		wctomb(c, chr);
	}
	return converted;
}

/*
static BLOCK ___remember(BLOCK b)
{
	// Only works for 1 -> 1 functions
	struct memolist {
		struct memolist* next;
		ANY input;
		ANY output;
	};
	__block struct memolist* memo = NULL;
	return Block_copy(^{
		ANY a = pop();
		for (struct memolist* l = memo ; l ; l = l->next)
			if (l->input == a)
			{
				push(l->output);
				return;
			}
		ANY* temp = stack.start;
		stack.start = stack.top;
		push(a);
		b();
		stack.start = temp;
		struct memolist* new = gc_malloc(sizeof *new);
		new->input = a;
		new->output = peek();
		new->next = memo;
		memo = new;
	});
}
*/

/*
static BLOCK ___pure(BLOCK b)
{
	return Block_copy(^{
		pure = 1;
		b();
		pure = 0;
	});
}
*/


static BOX ___box(ANY a) // boxes seem to break the GC sometimes TODO
{
	ANY* b = gc_malloc(sizeof *b);
	*b = a;
	return b;
}

static ANY ___unbox(BOX b)
{
	return *b;
}

static void ___set(BOX b, ANY a)
{
	*b = a;
}

/* math */

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// helper for math functions
static inline NUMBER radians_to_degrees(NUMBER a)
{
	return a * (180 / M_PI);
}

static inline NUMBER degrees_to_radians(NUMBER a)
{
	return a * (M_PI / 180);
}

static NUMBER ___sind(NUMBER a)
{
	double rad = degrees_to_radians(a);
	double sinrad = sin(rad);
	return sinrad;
}

static NUMBER ___cosd(NUMBER a)
{
	double rad = degrees_to_radians(a);
	double cosrad = cos(rad);
	return cosrad;
}

static NUMBER ___tand(NUMBER a)
{
	double rad = degrees_to_radians(a);
	double tanrad = tan(rad);
	return tanrad;
}

static NUMBER ___sin(NUMBER a)
{
	return sin(a);
}

static NUMBER ___cos(NUMBER a)
{
	return cos(a);
}

static NUMBER ___tan(NUMBER a)
{
	return tan(a);
}

static NUMBER ___exp(NUMBER a)
{
	return exp(a);
}

static NUMBER ___log(NUMBER a, NUMBER b)
{
	/* This uses the following formula:
	   log_x(y) =
	   	    log_e(y) / log_e(x)
	*/
	const double top = log(b);
	const double bottom = log(a);
	return top / bottom;
}

static NUMBER ___ln(NUMBER a)
{
	return log(a);
}


static NUMBER ___asind(NUMBER a)
{
	return radians_to_degrees(asin(a));
}

static NUMBER ___acosd(NUMBER a)
{
	return radians_to_degrees(acos(a));
}

static NUMBER ___atand(NUMBER a)
{
	return radians_to_degrees(atan(a));
}

static NUMBER ___asin(NUMBER a)
{
	return asin(a);
}

static NUMBER ___acos(NUMBER a)
{
	return acos(a);
}

static NUMBER ___atan(NUMBER a)
{
  	return atan(a);
}

static NUMBER ___sinhd(NUMBER a)
{
	return sinh(degrees_to_radians(a));
}

static NUMBER ___coshd(NUMBER a)
{
	return cosh(degrees_to_radians(a));
}

static NUMBER ___tanhd(NUMBER a)
{
	return tanh(degrees_to_radians(a));
}

static NUMBER ___sinh(NUMBER a)
{
	return sinh(a);
}

static NUMBER ___cosh(NUMBER a)
{
	return cosh(a);
}

static NUMBER ___tanh(NUMBER a)
{
	return tanh(a);
}

static IO ___open(SYMBOL m, STRING path)
{
	assert_impure();
	char* mode;
	if (m == SYMread) mode = "r";
	else if (m == SYMwrite) mode = "w";
	else if (m == SYMappend) mode = "a";
	else if (m == SYMreadHappend) mode = "a+";
	else if (m == SYMreadHwrite) mode = "w+";
	else if (m == SYMreadHwriteHexisting) mode = "r+";
	else throw_error("Expected one of \\read, \\write, \\append, \\read-write, \\read-append, \\read-write-existing");
	FILE* fp = fopen(path, mode);
	if unlikely(!fp) throw_error_fmt("Cannot open file '%s'", path);
	IO io = gc_malloc(sizeof *io);
	io->path = path;
	io->mode = mode;
	io->file = fp;
	return io;
}

static STRING ___readHfile(IO io)
{
	assert_impure();
	// Read a file to a string.
	FILE *fp = io->file;
	fseek(fp, 0, SEEK_SET); // seek to beginning
	if unlikely(!io->mode) throw_error_fmt("File '%s' is not open", io->path);
	if unlikely(fp == NULL) throw_error_fmt("Cannot open file '%s'", io->path);
	struct stat st;
	fstat(fileno(fp), &st);
	char* const text = gc_flatmalloc (st.st_size + 1);
	if (fread(text, sizeof(char), st.st_size, fp) != (unsigned long)st.st_size)
		throw_error_fmt("Error reading file '%s'", io->path);
	text[st.st_size] = '\0'; // Remove trailing eof.
	return text;
}

static STRING ___readHline(IO io)
{
	assert_impure();
	char* buf = (char*)(space[z] + alloc[z]); // use the top of memory as a buffer like Show does
	fgets(buf, INT_MAX, io->file);
	return gc_strdup(buf); // this can only GC once so won't overwrite the buffer.
}

static void ___close(IO io)
{
	assert_impure();
	fclose(io->file);
	io->file = NULL;
}

static BOOLEAN ___openQ(IO io)
{
	return (BOOLEAN)io->file;
}

static STRING ___fileHname(IO io)
{
	return io->path;
}

static STRING ___fileHmode(IO io)
{
	return io->mode; // TODO symbol
}

static void ___write(STRING s, IO io)
{
	fputs(s, io->file);
}

static void ___seek(SYMBOL ref, NUMBER n, IO io)
{
	int pos;
	if (ref == SYMstart) pos = SEEK_SET;
	else if (ref == SYMend) pos = SEEK_END;
	else if (ref == SYMcurrent) pos = SEEK_CUR;
	else throw_error_fmt("Expected one of \\start, \\end, \\current");
	long offset = n;
	if unlikely(offset != n || fseek(io->file, offset, pos))
		throw_error_fmt("Can't seek to position %.14g relative to %s", n, ref);
}

static void invalid_jump(uint8_t* env)
{
	throw_error("Cannot resume expired continuation");
}

static void oh_no(uint8_t* env)
{
	longjmp(*(jmp_buf*)env, 1);
}

__attribute__((returns_twice))
static void ___begin(BLOCK f)
{
	jmp_buf b;
	if (!setjmp(b))
	{
		BLOCK a = (BLOCK) {.fn=oh_no, .env=(void*)&b};
		push(box_BLOCK(a));
		call_block(f);
		a.fn = invalid_jump;
	}
}

static LIST ___empty (void)
{
	return NULL;
}

static TABLE ___table (BLOCK expr)
{
	ANYPTR tmp_stack_start = stack.start;
	stack.start = stack.top;
	// Eval expr
	call_block(expr);
	// Move to a table.
	TABLE d = NULL;
	size_t len = stack_length();
	if unlikely(len & 1) throw_error("Table initialiser must be key-value pairs");
	for (size_t i = 0; i < len; i += 2)
	{
		ANY key = stack.start[i+1];
		ANY value = stack.start[i];
		d = ___insert(key, value, d);
	}
	stack.top = stack.start;
	stack.start = tmp_stack_start;
	return d;
}

static TABLE ___insert(ANY key, ANY value, TABLE d)
{
	if unlikely(key.type == block || key.type == box) throw_error_fmt("Can't index a table with %s", ___show(key));
	cognate_table* D = gc_malloc(sizeof *D);
	if (!d)
	{
		D->left = NULL;
		D->right = NULL;
		D->key = key;
		D->value = value;
		D->level = 1;
		return D;
	}
	ptrdiff_t diff = compare_objects(d->key, key);
	if (diff == 0)
	{
		D->left = d->left;
		D->right = d->right;
		D->key = key;
		D->value = value;
		D->level = d->level;
		return D;
	}
	else if (diff > 0)
	{
		D->key = d->key;
		D->value = d->value;
		D->level = d->level;
		D->right = d->right;
		D->left = ___insert(key, value, d->left);
	}
	else //if (diff < 0)
	{
		D->key = d->key;
		D->value = d->value;
		D->level = d->level;
		D->left = d->left;
		D->right = ___insert(key, value, d->right);
	}

	// Perform skew and then split. The conditionals that determine whether or
   // not a rotation will occur or not are inside of the procedures, as given
   // above.
	D = table_skew(D);
	D = table_split(D);

	return D;
}

static ANY ___D(ANY key, TABLE d)
{
	if unlikely(key.type == block || key.type == box) throw_error_fmt("Can't index a table with %s", ___show(key));
	while (d)
	{
		ptrdiff_t diff = compare_objects(d->key, key);
		if (diff == 0) return d->value;
		else if (diff > 0) d = d->left;
		else d = d->right;
	}

	throw_error_fmt("%s is not in table", ___show(key));
	#ifdef __TINYC__
	return (cognate_object){0};
	#endif
}

static BOOLEAN ___has(ANY key, TABLE d)
{
	if unlikely(key.type == block || key.type == box) throw_error_fmt("Can't index a table with %s", ___show(key));
	while (d)
	{
		ptrdiff_t diff = compare_objects(d->key, key);
		if (diff == 0) return true;
		else if (diff > 0) d = d->left;
		else d = d->right;
	}

	return false;
}

static TABLE ___remove(ANY X, TABLE T)
{
	// input: X, the value to delete, and T, the root of the tree from which it should be deleted.
   // output: T, balanced, without the value X.

	// TODO
	return NULL;
}

static LIST values_helper(TABLE T, LIST L)
{
	if (!T) return L;
	return values_helper(T->left, ___push(T->value, values_helper(T->right, L)));
}

static LIST ___values(TABLE T)
{
	return values_helper(T, NULL);
}

static LIST keys_helper(TABLE T, LIST L)
{
	if (!T) return L;
	return keys_helper(T->left, ___push(T->key, keys_helper(T->right, L)));
}

static LIST ___keys(TABLE T)
{
	return keys_helper(T, NULL);
}

static NUMBER ___length_TABLE(TABLE T)
{
	if (!T) return 0;
	return 1 + ___length_TABLE(T->left) + ___length_TABLE(T->right);
}

static NUMBER ___length_LIST(LIST l)
{
	size_t len = 0;
	while (l)
	{
		len++;
		l = l->next;
	}
	return (NUMBER)len;
}

static NUMBER ___length_STRING(STRING str)
{
	size_t len = 0;
	for (; *str ; str += mblen(str, MB_CUR_MAX), ++len);
	return len;
}

static NUMBER ___length(ANY a)
{
	switch(a.type)
	{
		case list: return ___length_LIST(unbox_LIST(a));
		case string: return ___length_STRING(unbox_STRING(a));
		case table: return ___length_TABLE(unbox_TABLE(a));
		default: type_error("list or string or table", a);
	}
#ifdef __TINYC__
	return 0;
#endif
}

static STRING ___show_NUMBER(NUMBER a)
{
	show_number(a, show_buffer);
	return gc_strdup(show_buffer);
}

static STRING ___show_TABLE(TABLE a)
{
	show_table(a, show_buffer, NULL);
	return gc_strdup(show_buffer);
}

static STRING ___show_IO(IO a)
{
	show_io(a, show_buffer);
	return gc_strdup(show_buffer);
}

static STRING ___show_STRING(STRING s)
{
	return s;
}

static STRING ___show_BOOLEAN(BOOLEAN b)
{
	return b ? "True" : "False";
}

static STRING ___show_SYMBOL(SYMBOL s)
{
	return s;
}

static STRING ___show_BLOCK(BLOCK b)
{
	show_block(b, show_buffer);
	return gc_strdup(show_buffer);
}

static STRING ___show_BOX(BOX b)
{
	show_box(b, show_buffer, NULL);
	return gc_strdup(show_buffer);
}

static STRING ___show_LIST(LIST l)
{
	show_list(l, show_buffer, NULL);
	return gc_strdup(show_buffer);
}

static BOOLEAN ___numberQ_NUMBER(NUMBER _) { return true; }
static BOOLEAN ___numberQ_LIST(LIST _) { return false; }
static BOOLEAN ___numberQ_BOX(BOX _) { return false; }
static BOOLEAN ___numberQ_TABLE(TABLE _) { return false; }
static BOOLEAN ___numberQ_IO(IO _) { return false; }
static BOOLEAN ___numberQ_BOOLEAN(BOOLEAN _) { return false; }
static BOOLEAN ___numberQ_STRING(STRING _) { return false; }
static BOOLEAN ___numberQ_SYMBOL(SYMBOL _) { return false; }
static BOOLEAN ___numberQ_BLOCK(BLOCK _) { return false; }

static BOOLEAN ___listQ_NUMBER(NUMBER _) { return false; }
static BOOLEAN ___listQ_LIST(LIST _) { return true; }
static BOOLEAN ___listQ_BOX(BOX _) { return false; }
static BOOLEAN ___listQ_TABLE(TABLE _) { return false; }
static BOOLEAN ___listQ_IO(IO _) { return false; }
static BOOLEAN ___listQ_BOOLEAN(BOOLEAN _) { return false; }
static BOOLEAN ___listQ_STRING(STRING _) { return false; }
static BOOLEAN ___listQ_SYMBOL(SYMBOL _) { return false; }
static BOOLEAN ___listQ_BLOCK(BLOCK _) { return false; }

static BOOLEAN ___boxQ_NUMBER(NUMBER _) { return false; }
static BOOLEAN ___boxQ_LIST(LIST _) { return false; }
static BOOLEAN ___boxQ_BOX(BOX _) { return true; }
static BOOLEAN ___boxQ_TABLE(TABLE _) { return false; }
static BOOLEAN ___boxQ_IO(IO _) { return false; }
static BOOLEAN ___boxQ_BOOLEAN(BOOLEAN _) { return false; }
static BOOLEAN ___boxQ_STRING(STRING _) { return false; }
static BOOLEAN ___boxQ_SYMBOL(SYMBOL _) { return false; }
static BOOLEAN ___boxQ_BLOCK(BLOCK _) { return false; }

static BOOLEAN ___tableQ_NUMBER(NUMBER _) { return false; }
static BOOLEAN ___tableQ_LIST(LIST _) { return false; }
static BOOLEAN ___tableQ_BOX(BOX _) { return false; }
static BOOLEAN ___tableQ_TABLE(TABLE _) { return true; }
static BOOLEAN ___tableQ_IO(IO _) { return false; }
static BOOLEAN ___tableQ_BOOLEAN(BOOLEAN _) { return false; }
static BOOLEAN ___tableQ_STRING(STRING _) { return false; }
static BOOLEAN ___tableQ_SYMBOL(SYMBOL _) { return false; }
static BOOLEAN ___tableQ_BLOCK(BLOCK _) { return false; }

static BOOLEAN ___ioQ_NUMBER(NUMBER _) { return false; }
static BOOLEAN ___ioQ_LIST(LIST _) { return false; }
static BOOLEAN ___ioQ_BOX(BOX _) { return false; }
static BOOLEAN ___ioQ_TABLE(TABLE _) { return false; }
static BOOLEAN ___ioQ_IO(IO _) { return true; }
static BOOLEAN ___ioQ_BOOLEAN(BOOLEAN _) { return false; }
static BOOLEAN ___ioQ_STRING(STRING _) { return false; }
static BOOLEAN ___ioQ_SYMBOL(SYMBOL _) { return false; }
static BOOLEAN ___ioQ_BLOCK(BLOCK _) { return false; }

static BOOLEAN ___booleanQ_NUMBER(NUMBER _) { return false; }
static BOOLEAN ___booleanQ_LIST(LIST _) { return false; }
static BOOLEAN ___booleanQ_BOX(BOX _) { return false; }
static BOOLEAN ___booleanQ_TABLE(TABLE _) { return false; }
static BOOLEAN ___booleanQ_IO(IO _) { return false; }
static BOOLEAN ___booleanQ_BOOLEAN(BOOLEAN _) { return true; }
static BOOLEAN ___booleanQ_STRING(STRING _) { return false; }
static BOOLEAN ___booleanQ_SYMBOL(SYMBOL _) { return false; }
static BOOLEAN ___booleanQ_BLOCK(BLOCK _) { return false; }

static BOOLEAN ___stringQ_NUMBER(NUMBER _) { return false; }
static BOOLEAN ___stringQ_LIST(LIST _) { return false; }
static BOOLEAN ___stringQ_BOX(BOX _) { return false; }
static BOOLEAN ___stringQ_TABLE(TABLE _) { return false; }
static BOOLEAN ___stringQ_IO(IO _) { return false; }
static BOOLEAN ___stringQ_BOOLEAN(BOOLEAN _) { return false; }
static BOOLEAN ___stringQ_STRING(STRING _) { return true; }
static BOOLEAN ___stringQ_SYMBOL(SYMBOL _) { return false; }
static BOOLEAN ___stringQ_BLOCK(BLOCK _) { return false; }

static BOOLEAN ___symbolQ_NUMBER(NUMBER _) { return false; }
static BOOLEAN ___symbolQ_LIST(LIST _) { return false; }
static BOOLEAN ___symbolQ_BOX(BOX _) { return false; }
static BOOLEAN ___symbolQ_TABLE(TABLE _) { return false; }
static BOOLEAN ___symbolQ_IO(IO _) { return false; }
static BOOLEAN ___symbolQ_BOOLEAN(BOOLEAN _) { return false; }
static BOOLEAN ___symbolQ_STRING(STRING _) { return false; }
static BOOLEAN ___symbolQ_SYMBOL(SYMBOL _) { return true; }
static BOOLEAN ___symbolQ_BLOCK(BLOCK _) { return false; }

static BOOLEAN ___blockQ_NUMBER(NUMBER _) { return false; }
static BOOLEAN ___blockQ_LIST(LIST _) { return false; }
static BOOLEAN ___blockQ_BOX(BOX _) { return false; }
static BOOLEAN ___blockQ_TABLE(TABLE _) { return false; }
static BOOLEAN ___blockQ_IO(IO _) { return false; }
static BOOLEAN ___blockQ_BOOLEAN(BOOLEAN _) { return false; }
static BOOLEAN ___blockQ_STRING(STRING _) { return false; }
static BOOLEAN ___blockQ_SYMBOL(SYMBOL _) { return false; }
static BOOLEAN ___blockQ_BLOCK(BLOCK _) { return true; }

// ---------- ACTUAL PROGRAM ----------