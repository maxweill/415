# Code Generator (A simple compiler)
Parser and code generator written in flex/bison for a Pascal-lite language.

The language can contain integers and booleans, single dimensional arrays (0 indexed) of either type.

Operations available are while-do, if-then-else, assignement, print, and compounds of these.

Operators can be arithmetic, logical, or relational.

Control flow consists of while-do loops and if-then-else statements.

All compiled code is outputted int ILOC.

## Usage
Generate an executable by typing 

```make```

The executable requires input on stdin, so it should be formatted like

```./codegen < demo-code```
