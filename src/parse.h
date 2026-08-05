#ifndef PARSE_H
#define PARSE_H

#include "lang.h"
#include "tagvec.h"

int parse_file(char* filepath, LangCache* cache, TSParser* parser, TSQueryCursor* cursor, TagVec* vec);

#endif
