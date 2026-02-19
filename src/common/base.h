#include <stdio.h>
#include <stdarg.h>

#define panic(...) { printf(__VA_ARGS__); exit(EXIT_FAILURE); }

#ifndef max
	#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef min
	#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

// TODO
// #define print(...) _Generic((FIRST_ARG(__VA_ARGS__)),
// 	string: prints,
// 	default: printf,
// )(__VA_ARGS__)
