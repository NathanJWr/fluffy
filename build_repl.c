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

/* debug includes */
#include "src/leak_checker.h"

/* project includes */
#include "src/defines.h"
#include "src/stretchy_array.h"
#include "src/number.h"
#include "src/token.h"
#include "src/ast.h"
#include "src/object.h"
#include "src/environment.h"
#include "src/garbage_collector.h"
#include "src/lexer.h"
#include "src/parser.h"
#include "src/evaluate.h"

/* project c files */
#include "src/garbage_collector.c"
#include "src/ast.c"
#include "src/parser.c"
#include "src/lexer.c"
#include "src/object.c"
#include "src/environment.c"
#include "src/evaluate.c"
#include "src/repl.c"
