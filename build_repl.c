#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#define DEBUG_TYPES

/* standard includes */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stddef.h>
#include <limits.h>
#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include <assert.h>
#include <float.h>
#include <stdbool.h>

/* debug includes */
#include "src/leak_checker.h"

/* stretchy arrays */
#include "src/stretchy_array.h"

/* project includes */
#include "src/number.h"
#include "src/object_methods.h"
#include "src/object_string.h"
#include "src/object_array.h"
#include "src/environment.h"
#include "src/evaluate.h"
#include "src/garbage_collector.h"
#include "src/stretchy_array_gc.h"
#include "src/token.h"
#include "src/parser.h"
#include "src/ast.h"
#include "src/object.h"
#include "src/lexer.h"

/* project c files */
#include "src/garbage_collector.c"
#include "src/ast.c"
#include "src/parser.c"
#include "src/lexer.c"
#include "src/object_methods.c"
#include "src/object.c"
#include "src/environment.c"
#include "src/object_string.c"
#include "src/object_array.c"
#include "src/evaluate.c"
#include "src/repl.c"

