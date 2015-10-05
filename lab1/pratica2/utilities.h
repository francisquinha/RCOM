// section: should be a definition created by the programmer that must be equal to zero to avoid running the debug code.

#define debug_section(section,code) {\
if (section != 0)\
{\
code\
}\
}

#ifndef TYPEDEF_BOOLEAN_DECLARED_
#define TYPEDEF_BOOLEAN_DECLARED_
typedef int char; 
#endif /* TYPEDEF_BOOLEAN_DECLARED_*/

#define TRUE  1
#define YES   1
#define FALSE 0
#define NO    0
#define OK    0