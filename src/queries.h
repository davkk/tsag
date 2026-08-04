#ifndef QUERIES_H
#define QUERIES_H
#include <stddef.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverlength-strings"

static const char QUERY_cpp[] =
    "(function_definition declarator: (function_declarator declarator: (identifier) @name)) @kind.function\n"
    "(function_definition declarator: (function_declarator declarator: (field_identifier) @name)) @kind.function\n"
    "(function_definition declarator: (function_declarator declarator: (qualified_identifier name: (_) @name))) @kind.function\n"
    "(function_definition declarator: (pointer_declarator declarator: (function_declarator declarator: (identifier) @name))) @kind.function\n"
    "(function_definition declarator: (pointer_declarator declarator: (function_declarator declarator: (field_identifier) @name))) @kind.function\n"
    "(function_definition declarator: (pointer_declarator declarator: (function_declarator declarator: (qualified_identifier name: (_) @name)))) @kind.function\n"
    "(class_specifier name: (type_identifier) @name body: (_)) @kind.class\n"
    "(struct_specifier name: (type_identifier) @name body: (_)) @kind.struct\n"
    "(union_specifier name: (type_identifier) @name body: (_)) @kind.union\n"
    "(namespace_definition name: (namespace_identifier) @name) @kind.namespace\n"
    "(type_definition declarator: (type_identifier) @name) @kind.typedef\n"
    "(alias_declaration name: (type_identifier) @name) @kind.typedef\n"
    "(enum_specifier name: (type_identifier) @name body: (_)) @kind.enum\n"
    "(enumerator name: (identifier) @name) @kind.enumerator\n"
    "(preproc_def name: (identifier) @name) @kind.macro\n"
    "(preproc_function_def name: (identifier) @name) @kind.macro\n"
        "(field_declaration declarator: (field_identifier) @name) @kind.member\n"
    "(field_declaration declarator: (pointer_declarator declarator: (field_identifier) @name)) @kind.member\n"
    "(field_declaration declarator: (pointer_declarator declarator: (pointer_declarator declarator: (field_identifier) @name))) @kind.member\n"
    "(field_declaration declarator: (array_declarator declarator: (field_identifier) @name)) @kind.member\n"

    "(translation_unit (declaration declarator: (identifier) @name) @kind.variable)\n"
    "(translation_unit (declaration declarator: (init_declarator declarator: (identifier) @name)) @kind.variable)\n"
    "(translation_unit (declaration declarator: (array_declarator declarator: (identifier) @name)) @kind.variable)\n"
    "(translation_unit (declaration declarator: (init_declarator declarator: (array_declarator declarator: (identifier) @name))) @kind.variable)\n"
    "(preproc_ifdef (declaration declarator: (identifier) @name) @kind.variable)\n"
    "(preproc_ifdef (declaration declarator: (init_declarator declarator: (identifier) @name)) @kind.variable)\n"
    "(preproc_ifdef (declaration declarator: (array_declarator declarator: (identifier) @name)) @kind.variable)\n"
    "(preproc_ifdef (declaration declarator: (init_declarator declarator: (array_declarator declarator: (identifier) @name))) @kind.variable)\n"
    "(preproc_if (declaration declarator: (identifier) @name) @kind.variable)\n"
    "(preproc_if (declaration declarator: (init_declarator declarator: (identifier) @name)) @kind.variable)\n"
    "(preproc_if (declaration declarator: (array_declarator declarator: (identifier) @name)) @kind.variable)\n"
    "(preproc_if (declaration declarator: (init_declarator declarator: (array_declarator declarator: (identifier) @name))) @kind.variable)\n"
    "(namespace_definition body: (declaration_list (declaration declarator: (identifier) @name) @kind.variable))\n"
    "(namespace_definition body: (declaration_list (declaration declarator: (init_declarator declarator: (identifier) @name)) @kind.variable))\n"
    "(namespace_definition body: (declaration_list (declaration declarator: (array_declarator declarator: (identifier) @name)) @kind.variable))\n"
    "(namespace_definition body: (declaration_list (declaration declarator: (init_declarator declarator: (array_declarator declarator: (identifier) @name))) @kind.variable))\n"
    ;
static const char QUERY_c[] =
    "(function_definition declarator: (function_declarator declarator: (identifier) @name)) @kind.function\n"
    "(function_definition declarator: (function_declarator declarator: (field_identifier) @name)) @kind.function\n"
    "(function_definition declarator: (pointer_declarator declarator: (function_declarator declarator: (identifier) @name))) @kind.function\n"
    "(function_definition declarator: (pointer_declarator declarator: (function_declarator declarator: (field_identifier) @name))) @kind.function\n"
    "(struct_specifier name: (type_identifier) @name body: (_)) @kind.struct\n"
    "(union_specifier name: (type_identifier) @name body: (_)) @kind.union\n"
    "(type_definition declarator: (type_identifier) @name) @kind.typedef\n"
    "(enum_specifier name: (type_identifier) @name body: (_)) @kind.enum\n"
    "(enumerator name: (identifier) @name) @kind.enumerator\n"
    "(preproc_def name: (identifier) @name) @kind.macro\n"
    "(preproc_function_def name: (identifier) @name) @kind.macro\n"
        "(field_declaration declarator: (field_identifier) @name) @kind.member\n"
    "(field_declaration declarator: (pointer_declarator declarator: (field_identifier) @name)) @kind.member\n"
    "(field_declaration declarator: (pointer_declarator declarator: (pointer_declarator declarator: (field_identifier) @name))) @kind.member\n"
    "(field_declaration declarator: (array_declarator declarator: (field_identifier) @name)) @kind.member\n"

    "(translation_unit (declaration declarator: (identifier) @name) @kind.variable)\n"
    "(translation_unit (declaration declarator: (init_declarator declarator: (identifier) @name)) @kind.variable)\n"
    "(translation_unit (declaration declarator: (array_declarator declarator: (identifier) @name)) @kind.variable)\n"
    "(translation_unit (declaration declarator: (init_declarator declarator: (array_declarator declarator: (identifier) @name))) @kind.variable)\n"
    "(preproc_ifdef (declaration declarator: (identifier) @name) @kind.variable)\n"
    "(preproc_ifdef (declaration declarator: (init_declarator declarator: (identifier) @name)) @kind.variable)\n"
    "(preproc_ifdef (declaration declarator: (array_declarator declarator: (identifier) @name)) @kind.variable)\n"
    "(preproc_ifdef (declaration declarator: (init_declarator declarator: (array_declarator declarator: (identifier) @name))) @kind.variable)\n"
    "(preproc_if (declaration declarator: (identifier) @name) @kind.variable)\n"
    "(preproc_if (declaration declarator: (init_declarator declarator: (identifier) @name)) @kind.variable)\n"
    "(preproc_if (declaration declarator: (array_declarator declarator: (identifier) @name)) @kind.variable)\n"
    "(preproc_if (declaration declarator: (init_declarator declarator: (array_declarator declarator: (identifier) @name))) @kind.variable)\n"
    ;
static const char QUERY_python[] =
    "(function_definition name: (identifier) @name) @kind.function\n"
    "(class_definition name: (identifier) @name) @kind.class\n"
    "(class_definition body: (block (function_definition name: (identifier) @name) @kind.member))\n"
    "(class_definition body: (block (decorated_definition definition: (function_definition name: (identifier) @name) @kind.member)))\n"
    "(class_definition body: (block (if_statement (block (function_definition name: (identifier) @name) @kind.member))))\n"
    "(class_definition body: (block (if_statement (elif_clause (block (function_definition name: (identifier) @name) @kind.member)))))\n"
    "(class_definition body: (block (if_statement (else_clause (block (function_definition name: (identifier) @name) @kind.member)))))\n"
    "(class_definition body: (block (for_statement (block (function_definition name: (identifier) @name) @kind.member))))\n"
    "(class_definition body: (block (while_statement (block (function_definition name: (identifier) @name) @kind.member))))\n"
    "(class_definition body: (block (with_statement (block (function_definition name: (identifier) @name) @kind.member))))\n"
    "(class_definition body: (block (try_statement (block (function_definition name: (identifier) @name) @kind.member))))\n"
    "(class_definition body: (block (try_statement (except_clause (block (function_definition name: (identifier) @name) @kind.member)))))\n"
    "(class_definition body: (block (try_statement (else_clause (block (function_definition name: (identifier) @name) @kind.member)))))\n"
    "(class_definition body: (block (try_statement (finally_clause (block (function_definition name: (identifier) @name) @kind.member)))))\n"
    "(module (expression_statement (assignment left: (identifier) @name)) @kind.variable)\n"
    "(module (expression_statement (assignment left: (pattern_list (identifier) @name)) @kind.variable))\n"
    "(module (expression_statement (assignment left: (tuple_pattern (identifier) @name)) @kind.variable))\n"
    "(import_statement name: (aliased_import alias: (identifier) @name)) @kind.namespace\n"
    ;

static const char QUERY_lua[] =
    "(function_declaration name: (identifier) @name) @kind.function\n"
    "(function_declaration name: (dot_index_expression field: (identifier) @name)) @kind.function\n"
    "(function_declaration name: (method_index_expression method: (identifier) @name)) @kind.function\n"
    ;

static const char QUERY_javascript[] =
    "(function_declaration name: (identifier) @name) @kind.function\n"
    "(generator_function_declaration name: (identifier) @name) @kind.generator\n"
    "(class_declaration name: (identifier) @name) @kind.class\n"
    "(pair key: (property_identifier) @name) @kind.property\n"
    "(method_definition name: (property_identifier) @name) @kind.method\n"
    "(method_definition \"*\" name: (property_identifier) @name) @kind.generator\n"
    "(method_definition \"get\" name: (property_identifier) @name) @kind.getter\n"
    "(method_definition \"set\" name: (property_identifier) @name) @kind.setter\n"
    "(field_definition property: (property_identifier) @name) @kind.field\n"
            "(program [(lexical_declaration kind: \"const\" (variable_declarator name: (identifier) @name)) (export_statement (lexical_declaration kind: \"const\" (variable_declarator name: (identifier) @name)))]) @kind.constant\n"
    "(program [(lexical_declaration kind: \"let\" (variable_declarator name: (identifier) @name)) (export_statement (lexical_declaration kind: \"let\" (variable_declarator name: (identifier) @name)))]) @kind.variable\n"
    "(program [(variable_declaration (variable_declarator name: (identifier) @name)) (export_statement (variable_declaration (variable_declarator name: (identifier) @name)))]) @kind.variable\n"
    "(program [(lexical_declaration kind: \"const\" (variable_declarator name: (identifier) @name value: (array))) (export_statement (lexical_declaration kind: \"const\" (variable_declarator name: (identifier) @name value: (array))))]) @kind.variable\n"
    "(program [(lexical_declaration kind: \"const\" (variable_declarator name: (object_pattern (shorthand_property_identifier_pattern) @name))) (export_statement (lexical_declaration kind: \"const\" (variable_declarator name: (object_pattern (shorthand_property_identifier_pattern) @name))))]) @kind.constant\n"
    "(program [(lexical_declaration kind: \"const\" (variable_declarator name: (object_pattern (pair_pattern value: (identifier) @name)))) (export_statement (lexical_declaration kind: \"const\" (variable_declarator name: (object_pattern (pair_pattern value: (identifier) @name)))))]) @kind.constant\n"
    "(program [(lexical_declaration kind: \"const\" (variable_declarator name: (array_pattern (identifier) @name))) (export_statement (lexical_declaration kind: \"const\" (variable_declarator name: (array_pattern (identifier) @name))))]) @kind.constant\n"
    "(program [(lexical_declaration kind: \"let\" (variable_declarator name: (object_pattern (shorthand_property_identifier_pattern) @name))) (export_statement (lexical_declaration kind: \"let\" (variable_declarator name: (object_pattern (shorthand_property_identifier_pattern) @name))))]) @kind.variable\n"
    "(program [(lexical_declaration kind: \"let\" (variable_declarator name: (object_pattern (pair_pattern value: (identifier) @name)))) (export_statement (lexical_declaration kind: \"let\" (variable_declarator name: (object_pattern (pair_pattern value: (identifier) @name)))))]) @kind.variable\n"
    "(program [(lexical_declaration kind: \"let\" (variable_declarator name: (array_pattern (identifier) @name))) (export_statement (lexical_declaration kind: \"let\" (variable_declarator name: (array_pattern (identifier) @name))))]) @kind.variable\n"
    "(program [(variable_declaration (variable_declarator name: (object_pattern (shorthand_property_identifier_pattern) @name))) (export_statement (variable_declaration (variable_declarator name: (object_pattern (shorthand_property_identifier_pattern) @name))))]) @kind.variable\n"
    "(program [(variable_declaration (variable_declarator name: (object_pattern (pair_pattern value: (identifier) @name)))) (export_statement (variable_declaration (variable_declarator name: (object_pattern (pair_pattern value: (identifier) @name)))))]) @kind.variable\n"
    "(program [(variable_declaration (variable_declarator name: (array_pattern (identifier) @name))) (export_statement (variable_declaration (variable_declarator name: (array_pattern (identifier) @name))))]) @kind.variable\n"
    "(lexical_declaration (variable_declarator name: (identifier) @name value: (arrow_function))) @kind.function\n"
    "(lexical_declaration (variable_declarator name: (identifier) @name value: (function_expression))) @kind.function\n"
    "(variable_declaration (variable_declarator name: (identifier) @name value: (arrow_function))) @kind.function\n"
    "(variable_declaration (variable_declarator name: (identifier) @name value: (function_expression))) @kind.function\n"
    "(lexical_declaration kind: \"const\" (variable_declarator name: (identifier) @name value: (object (_)))) @kind.variable\n"
    "(lexical_declaration kind: \"let\" (variable_declarator name: (identifier) @name value: (object (_)))) @kind.variable\n"
    "(variable_declaration (variable_declarator name: (identifier) @name value: (object (_)))) @kind.variable\n"
    ;
static const char QUERY_typescript[] =
    "(function_declaration name: (identifier) @name) @kind.function\n"
    "(generator_function_declaration name: (identifier) @name) @kind.generator\n"
    "(class_declaration name: (type_identifier) @name) @kind.class\n"
    "(abstract_class_declaration name: (type_identifier) @name) @kind.class\n"
    "(interface_declaration name: (type_identifier) @name) @kind.interface\n"
    "(enum_declaration name: (identifier) @name) @kind.enum\n"
    "(internal_module name: (identifier) @name) @kind.namespace\n"
    "(type_alias_declaration name: (type_identifier) @name) @kind.alias\n"
    "(class_declaration body: (class_body (method_signature name: (property_identifier) @name))) @kind.method\n"
    "(abstract_class_declaration body: (class_body (method_signature name: (property_identifier) @name))) @kind.method\n"
    "(class_declaration body: (class_body (abstract_method_signature name: (property_identifier) @name))) @kind.method\n"
    "(abstract_class_declaration body: (class_body (abstract_method_signature name: (property_identifier) @name))) @kind.method\n"
    "(interface_declaration body: (interface_body (method_signature name: (property_identifier) @name))) @kind.method\n"
    "(interface_declaration body: (interface_body (property_signature name: (property_identifier) @name))) @kind.property\n"
    "(enum_declaration body: (enum_body (property_identifier) @name)) @kind.enumerator\n"
    "(class_declaration body: (class_body (method_definition name: (property_identifier) @name))) @kind.method\n"
    "(abstract_class_declaration body: (class_body (method_definition name: (property_identifier) @name))) @kind.method\n"
    "(class_declaration body: (class_body (method_definition \"*\" name: (property_identifier) @name))) @kind.generator\n"
    "(abstract_class_declaration body: (class_body (method_definition \"*\" name: (property_identifier) @name))) @kind.generator\n"
    "(public_field_definition name: (property_identifier) @name) @kind.property\n"
                "(lexical_declaration kind: \"const\" (variable_declarator name: (identifier) @name)) @kind.constant\n"
    "(lexical_declaration kind: \"const\" (variable_declarator name: (object_pattern (shorthand_property_identifier_pattern) @name))) @kind.constant\n"
    "(lexical_declaration kind: \"const\" (variable_declarator name: (object_pattern (pair_pattern value: (identifier) @name)))) @kind.constant\n"
    "(lexical_declaration kind: \"const\" (variable_declarator name: (array_pattern (identifier) @name))) @kind.constant\n"
    "(program [(lexical_declaration kind: \"let\" (variable_declarator name: (identifier) @name)) (export_statement (lexical_declaration kind: \"let\" (variable_declarator name: (identifier) @name)))]) @kind.variable\n"
    "(program [(variable_declaration (variable_declarator name: (identifier) @name)) (export_statement (variable_declaration (variable_declarator name: (identifier) @name)))]) @kind.variable\n"
    "(program [(lexical_declaration kind: \"let\" (variable_declarator name: (object_pattern (shorthand_property_identifier_pattern) @name))) (export_statement (lexical_declaration kind: \"let\" (variable_declarator name: (object_pattern (shorthand_property_identifier_pattern) @name))))]) @kind.variable\n"
    "(program [(lexical_declaration kind: \"let\" (variable_declarator name: (object_pattern (pair_pattern value: (identifier) @name)))) (export_statement (lexical_declaration kind: \"let\" (variable_declarator name: (object_pattern (pair_pattern value: (identifier) @name)))))]) @kind.variable\n"
    "(program [(lexical_declaration kind: \"let\" (variable_declarator name: (array_pattern (identifier) @name))) (export_statement (lexical_declaration kind: \"let\" (variable_declarator name: (array_pattern (identifier) @name))))]) @kind.variable\n"
    "(program [(variable_declaration (variable_declarator name: (object_pattern (shorthand_property_identifier_pattern) @name))) (export_statement (variable_declaration (variable_declarator name: (object_pattern (shorthand_property_identifier_pattern) @name))))]) @kind.variable\n"
    "(program [(variable_declaration (variable_declarator name: (object_pattern (pair_pattern value: (identifier) @name)))) (export_statement (variable_declaration (variable_declarator name: (object_pattern (pair_pattern value: (identifier) @name)))))]) @kind.variable\n"
    "(program [(variable_declaration (variable_declarator name: (array_pattern (identifier) @name))) (export_statement (variable_declaration (variable_declarator name: (array_pattern (identifier) @name))))]) @kind.variable\n"
    "(internal_module body: (statement_block [(lexical_declaration kind: \"let\" (variable_declarator name: (identifier) @name)) (export_statement (lexical_declaration kind: \"let\" (variable_declarator name: (identifier) @name)))])) @kind.variable\n"
    "(module body: (statement_block [(lexical_declaration kind: \"let\" (variable_declarator name: (identifier) @name)) (export_statement (lexical_declaration kind: \"let\" (variable_declarator name: (identifier) @name)))])) @kind.variable\n"
    "(internal_module body: (statement_block [(variable_declaration (variable_declarator name: (identifier) @name)) (export_statement (variable_declaration (variable_declarator name: (identifier) @name)))])) @kind.variable\n"
    "(module body: (statement_block [(variable_declaration (variable_declarator name: (identifier) @name)) (export_statement (variable_declaration (variable_declarator name: (identifier) @name)))])) @kind.variable\n"
    "(internal_module body: (statement_block [(lexical_declaration kind: \"let\" (variable_declarator name: (object_pattern (shorthand_property_identifier_pattern) @name))) (export_statement (lexical_declaration kind: \"let\" (variable_declarator name: (object_pattern (shorthand_property_identifier_pattern) @name))))])) @kind.variable\n"
    "(module body: (statement_block [(lexical_declaration kind: \"let\" (variable_declarator name: (object_pattern (shorthand_property_identifier_pattern) @name))) (export_statement (lexical_declaration kind: \"let\" (variable_declarator name: (object_pattern (shorthand_property_identifier_pattern) @name))))])) @kind.variable\n"
    "(internal_module body: (statement_block [(lexical_declaration kind: \"let\" (variable_declarator name: (object_pattern (pair_pattern value: (identifier) @name)))) (export_statement (lexical_declaration kind: \"let\" (variable_declarator name: (object_pattern (pair_pattern value: (identifier) @name)))))])) @kind.variable\n"
    "(module body: (statement_block [(lexical_declaration kind: \"let\" (variable_declarator name: (object_pattern (pair_pattern value: (identifier) @name)))) (export_statement (lexical_declaration kind: \"let\" (variable_declarator name: (object_pattern (pair_pattern value: (identifier) @name)))))])) @kind.variable\n"
    "(internal_module body: (statement_block [(lexical_declaration kind: \"let\" (variable_declarator name: (array_pattern (identifier) @name))) (export_statement (lexical_declaration kind: \"let\" (variable_declarator name: (array_pattern (identifier) @name))))])) @kind.variable\n"
    "(module body: (statement_block [(lexical_declaration kind: \"let\" (variable_declarator name: (array_pattern (identifier) @name))) (export_statement (lexical_declaration kind: \"let\" (variable_declarator name: (array_pattern (identifier) @name))))])) @kind.variable\n"
    "(internal_module body: (statement_block [(variable_declaration (variable_declarator name: (object_pattern (shorthand_property_identifier_pattern) @name))) (export_statement (variable_declaration (variable_declarator name: (object_pattern (shorthand_property_identifier_pattern) @name))))])) @kind.variable\n"
    "(module body: (statement_block [(variable_declaration (variable_declarator name: (object_pattern (shorthand_property_identifier_pattern) @name))) (export_statement (variable_declaration (variable_declarator name: (object_pattern (shorthand_property_identifier_pattern) @name))))])) @kind.variable\n"
    "(internal_module body: (statement_block [(variable_declaration (variable_declarator name: (object_pattern (pair_pattern value: (identifier) @name)))) (export_statement (variable_declaration (variable_declarator name: (object_pattern (pair_pattern value: (identifier) @name)))))])) @kind.variable\n"
    "(module body: (statement_block [(variable_declaration (variable_declarator name: (object_pattern (pair_pattern value: (identifier) @name)))) (export_statement (variable_declaration (variable_declarator name: (object_pattern (pair_pattern value: (identifier) @name)))))])) @kind.variable\n"
    "(internal_module body: (statement_block [(variable_declaration (variable_declarator name: (array_pattern (identifier) @name))) (export_statement (variable_declaration (variable_declarator name: (array_pattern (identifier) @name))))])) @kind.variable\n"
    "(module body: (statement_block [(variable_declaration (variable_declarator name: (array_pattern (identifier) @name))) (export_statement (variable_declaration (variable_declarator name: (array_pattern (identifier) @name))))])) @kind.variable\n"
    ;
static const char QUERY_zig[] =
    "(function_declaration name: (identifier) @name) @kind.function\n"
    "(variable_declaration (identifier) @name) @kind.variable\n"
    "(container_field name: (identifier) @name) @kind.field\n"
    "(test_declaration (identifier) @name) @kind.function\n"
    "(test_declaration (string (string_content) @name)) @kind.function\n"
    "(variable_declaration (identifier) @name (struct_declaration)) @kind.type\n"
    "(variable_declaration (identifier) @name (enum_declaration)) @kind.type\n"
    "(variable_declaration (identifier) @name (union_declaration)) @kind.type\n"
    "(variable_declaration (identifier) @name (error_set_declaration)) @kind.type\n"
    ;

static const struct {
    const char* lang;
    const char* source;
    size_t source_len;
} QUERIES[] = {
    {"cpp", QUERY_cpp, sizeof(QUERY_cpp) - 1},
    {"c", QUERY_c, sizeof(QUERY_c) - 1},
    {"python", QUERY_python, sizeof(QUERY_python) - 1},
    {"lua", QUERY_lua, sizeof(QUERY_lua) - 1},
    {"javascript", QUERY_javascript, sizeof(QUERY_javascript) - 1},
    {"typescript", QUERY_typescript, sizeof(QUERY_typescript) - 1},
    {"zig", QUERY_zig, sizeof(QUERY_zig) - 1},
    {NULL, NULL, 0}
};

#pragma GCC diagnostic pop

#endif
