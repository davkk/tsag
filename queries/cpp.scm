; Functions
(function_definition declarator: (function_declarator declarator: (identifier) @name)) @kind.function
(function_definition declarator: (function_declarator declarator: (qualified_identifier name: (_) @name))) @kind.function

; Classes, Structs, Unions
(class_specifier name: (type_identifier) @name) @kind.class
(struct_specifier name: (type_identifier) @name) @kind.class
(union_specifier name: (type_identifier) @name) @kind.union

; Namespaces
(namespace_definition name: (namespace_identifier) @name) @kind.namespace

; Type aliases
(type_definition declarator: (type_identifier) @name) @kind.type

; Enums
(enum_specifier name: (type_identifier) @name) @kind.type
(enumerator name: (identifier) @name) @kind.enumerator

; Macros
(preproc_def name: (identifier) @name) @kind.macro
(preproc_function_def name: (identifier) @name) @kind.macro

; Field declarations
(field_declaration declarator: (field_identifier) @name) @kind.field

; Variables
(declaration declarator: (identifier) @name) @kind.variable
(declaration declarator: (init_declarator declarator: (identifier) @name)) @kind.variable
