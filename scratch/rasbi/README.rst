=====
Rasbi
=====

Interpreter of lisp like language, that doesnt' have any dependencies on system
libraries.



Properties
----------

- Reimplementation of some functions of standard library (libc)
- All used syscalls are written in assembly
- Custom types
  - DB like associative arrays, that are fitted in one slice of memory
- Whole memory footprint is in one chunk of memory
- Contains simple custom made unit test framework



Code organization
------------------------

Code in rasbi is "namespaced" pretty well, with all functions concerning
certain topic being together, separated from other groups by big comments.

Here is complete list of all sections with explanation.  Namespaced function
always begins with namespace name, e.g. functions if Type namespace begin with
type_.

- Type - functions operating on all types. See Types section below
- Sys/Platform - Low level functions, that are implemented per platform and
  architecture
- Runtime
- Interpreter
- Debug - functions used only in debug builds.  There are no rules for these
  functions, they can do anything - Global
- C - common functions that are usually found in c libraries
- Lib - common functions that usually are NOT in c libraries
- Parser - functions for parsing text to lispy structures
- Parser Ast - functions for processing parsed structures to real language
  objects i.e. recognize language structures like if or let
- Builtins - functions that are callable from inside rasbi



Types
~~~~~

Types that are used during runtime are all in union called `struct
ExpressionT`.  This type goes from simple integer types to as complex as
assoiative arrays and it is only type, that interpreter understands.

- Arrays
- Linked lists
- Dictionaries

  + Defragmentation
  + Db like design
  + Everything is fitted into single slice of memory

Other types are used only in cicumstances, where interpreter cannot operate
anyways. E.g. during parsing, building ast or memory management.

..
        Printf implementation was impossible, due to variadic arguments
        Getting argv must be written in assembly

	Custom memory manager implementation

	Assoca
