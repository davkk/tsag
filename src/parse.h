#ifndef PARSE_H
#define PARSE_H

#include <stdio.h>

#include "lang.h"
#include "tagvec.h"

int parse_file_emit(char* filepath, LangCache* cache, TSParser* parser, TSQueryCursor* cursor, FILE* out);
int parse_file(char* filepath, LangCache* cache, TSParser* parser, TSQueryCursor* cursor, TagVec* vec);

#endif
