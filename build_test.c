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

/* project includes */
#include "src/defines.h"
#include "src/stretchy_array.h"
#include "src/number.h"
#include "src/token.h"
#include "src/ast.h"
#include "src/lexer.h"
#include "src/parser.h"

/* project c files */
#include "src/ast.c"
#include "src/lexer.c"
#include "src/parser.c"

/* test files */
#include "test/testLog.c"
#include "test/lexerTests.c"
#include "test/parserTests.c"
#include "test/main.c"
