# Fluffy

Fluffy is an interpreted programming language with syntax similar to Javascript

# Setup

Fluffy can be compiled in two different modes: main-mode, and repl. Each mode has
their own respective build file. The project is structured as a unity build (i.e.
everything is contained within a single translation unit), thus the compilation
command is as simple as `$CC build_main.c` or `$CC build_repl.c`

# Features

* Variables
* Data Types
  * Integers and Doubles
  * Strings
  * Lists
  * Functions
* Closures
* Rescursive function calling
* User defined classes
* Language defined classes
  * System (provides basic functionallity like printing, opening files, etc.)
  * File (reading and writing to files)
* Garbage collection  


# Learning the language

So far, the language is fairly simple and you can get a good idea of what you
can write by looking at the code in the examples folder
