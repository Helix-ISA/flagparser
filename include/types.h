#ifndef HX_TYPES_H
#define HX_TYPES_H

typedef char 			s8;
typedef short 			s16;
typedef int 			s32;
typedef long int 		s64;

typedef unsigned char 		u8;
typedef unsigned short 		u16;
typedef unsigned int 		u32;
typedef unsigned long int 	u64;

typedef char			b8;
typedef int			b32;

typedef float			f32;
typedef double			f64;

#define false 0
#define true 1

#define success 1
#define failure 0

#if defined(_WIN32) || defined(_WIN64)
	#define PWINDOWS
#elif defined(__linux__)
	#define PLINUX
#elif defined(__APPLE__) || defined(__MACH__)
	#define PMAC
#else
	#error "Unsupported platform!"
#endif

#if defined(__clang__)
	#define HAPI __attribute((visibility("default")))
#else
	#define HAPI
#endif

#endif
