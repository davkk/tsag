#ifndef QUERIES_H
#define QUERIES_H
#include <stddef.h>

static const char QUERY_cpp[] =
    "(function_definition declarator: (function_declarator declarator: (identifier) @name)) @kind.function\n"
    "(function_definition declarator: (function_declarator declarator: (qualified_identifier name: (_) @name))) @kind.function\n"
    "(function_definition declarator: (pointer_declarator declarator: (function_declarator declarator: (identifier) @name))) @kind.function\n"
    "(function_definition declarator: (pointer_declarator declarator: (function_declarator declarator: (qualified_identifier name: (_) @name)))) @kind.function\n"
    "(class_specifier name: (type_identifier) @name body: (_)) @kind.class\n"
    "(struct_specifier name: (type_identifier) @name body: (_)) @kind.class\n"
    "(union_specifier name: (type_identifier) @name body: (_)) @kind.union\n"
    "(namespace_definition name: (namespace_identifier) @name) @kind.namespace\n"
    "(type_definition declarator: (type_identifier) @name) @kind.type\n"
    "(enum_specifier name: (type_identifier) @name body: (_)) @kind.type\n"
    "(enumerator name: (identifier) @name) @kind.enumerator\n"
    "(preproc_def name: (identifier) @name) @kind.macro\n"
    "(preproc_function_def name: (identifier) @name) @kind.macro\n"
    "(field_declaration declarator: (field_identifier) @name) @kind.field\n"
    "(declaration declarator: (identifier) @name) @kind.variable\n"
    "(declaration declarator: (init_declarator declarator: (identifier) @name)) @kind.variable\n"
    ;

static const char QUERY_c[] =
    "(function_definition declarator: (function_declarator declarator: (identifier) @name)) @kind.function\n"
    "(function_definition declarator: (pointer_declarator declarator: (function_declarator declarator: (identifier) @name))) @kind.function\n"
    "(struct_specifier name: (type_identifier) @name body: (_)) @kind.struct\n"
    "(union_specifier name: (type_identifier) @name body: (_)) @kind.union\n"
    "(type_definition declarator: (type_identifier) @name) @kind.type\n"
    "(enum_specifier name: (type_identifier) @name body: (_)) @kind.type\n"
    "(enumerator name: (identifier) @name) @kind.enumerator\n"
    "(preproc_def name: (identifier) @name) @kind.macro\n"
    "(preproc_function_def name: (identifier) @name) @kind.macro\n"
    "(field_declaration declarator: (field_identifier) @name) @kind.field\n"
    "(declaration declarator: (identifier) @name) @kind.variable\n"
    "(declaration declarator: (init_declarator declarator: (identifier) @name)) @kind.variable\n"
    ;

static const char QUERY_python[] =
    "(function_definition name: (identifier) @name) @kind.function\n"
    "(class_definition name: (identifier) @name) @kind.class\n"
    "(module (expression_statement (assignment left: (identifier) @name) @kind.variable))\n"
    "(module (expression_statement (assignment left: (pattern_list (identifier) @name)) @kind.variable))\n"
    "(module (expression_statement (assignment left: (tuple_pattern (identifier) @name)) @kind.variable))\n"
    "(decorated_definition definition: (function_definition name: (identifier) @name)) @kind.function\n"
    "(decorated_definition definition: (class_definition name: (identifier) @name)) @kind.class\n"
    ;

static const struct {
    const char* lang;
    const char* source;
    size_t source_len;
} QUERIES[] = {
    {"cpp", QUERY_cpp, sizeof(QUERY_cpp) - 1},
    {"c", QUERY_c, sizeof(QUERY_c) - 1},
    {"python", QUERY_python, sizeof(QUERY_python) - 1},
    {NULL, NULL, 0}
};

#endif
