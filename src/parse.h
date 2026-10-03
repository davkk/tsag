#ifndef PARSE_H
#define PARSE_H

#include "lang.h"
#include "linevec.h"

int parse_file_lines(char* filepath, LangCache* cache, TSParser* parser, TSQueryCursor* cursor, LineVec* vec);

#endif
