// section: should be a definition created by the programmer that must be equal to zero to avoid running the debug code.

#define debug_section(section,code) {\
if (section != 0)\
{\
code\
}\
}


#define TRUE  1
#define YES   1
#define FALSE 0
#define NO    0
#define OK    0