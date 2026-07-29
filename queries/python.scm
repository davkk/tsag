; Functions
(function_definition name: (identifier) @name) @kind.function

; Classes
(class_definition name: (identifier) @name) @kind.class

; Module-level assignments
(module (expression_statement (assignment left: (identifier) @name) @kind.variable))
(module (expression_statement (assignment left: (pattern_list (identifier) @name)) @kind.variable))
(module (expression_statement (assignment left: (tuple_pattern (identifier) @name)) @kind.variable))

; Decorated definitions (function / class under a decorator)
(decorated_definition
  definition: (function_definition name: (identifier) @name)) @kind.function
(decorated_definition
  definition: (class_definition name: (identifier) @name)) @kind.class
