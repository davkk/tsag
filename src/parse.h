#ifndef PARSE_H
#define PARSE_H

#include <stdio.h>

#include "lang.h"

int parse_file(char* filepath, LangCache* cache, TSParser* parser, TSQueryCursor* cursor, FILE* out);

#endif
