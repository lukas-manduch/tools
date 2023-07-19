# Rasbi
Interpreter of lisp like language, that doesnt' have any dependencies on system
libraries.


## Properties
- Reimplementation of some functions of standard library (libc)
- All used syscalls are written in assembly
- Custom types
  - DB like associative arrays, that are fitted in one slice of memory
- Whole memory footprint is in one chunk of memory


## Inner workings explained
Code in rasbi is "namespaced" pretty well, with all functions concerning
certain topic being together, separated from other groups by big comments.

Some of these sections are:
- Types - functions operating on all types. See Types section below
- Sys/Platform - Low level functions, that are implemented per platform and
  architecture



### Types
Types that are used during runtime are all in union called `struct
ExpressionT`.  This type goes from simple integer types to as complex as
assoiative arrays and it is only type, that interpreter understands.

- Arrays
- Linked lists
- Dictionaries

Other types are used only in cicumstances, where interpreter cannot operate
anyways. E.g. during parsing, building ast or memory management.

