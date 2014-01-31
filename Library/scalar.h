#ifndef CORE_SCALAR_H
#define CORE_SCALAR_H

#define	BOGUS_RANGE	18000

typedef float vec;

typedef unsigned char      u1;
typedef signed char        s1;

typedef unsigned short     u2;
typedef signed short       s2;

typedef unsigned int       u4;
typedef signed int         s4;

#ifdef _MSC_VER
typedef signed __int64     s8;
typedef unsigned __int64   u8;
#else
typedef signed long long   s8;
typedef unsigned long long u8;
#endif

typedef enum {False, True} Bool;

#ifdef _MSC_VER
#define vec_sin(s, t) __asm fld dword ptr [s] __asm fsin __asm fstp dword ptr [t]
#define vec_cos(s, t) __asm fld dword ptr [s] __asm fcos __asm fstp dword ptr [t]
#define vec_sincos(s, t1, t2) __asm fld dword ptr [s] __asm fsincos __asm fstp dword ptr [t2] __asm fstp dword ptr [t1]
#define vec_tan(s, t) __asm fld dword ptr [s] __asm fptan __asm fstp st __asm fstp dword ptr [t]
#define vec_sqrt(s, t) __asm fld dword ptr [s] __asm fsqrt __asm fstp dword ptr [t]
#else
float _vec_sin(float);
float _vec_cos(float);
float _vec_tan(float x);
float _vec_sqrt(float);

#define vec_sin(s, t) (t = _vec_sin(s))
#define vec_cos(s, t) (t = _vec_cos(s))
#define vec_sincos(s, t1, t2) (t1 = _vec_sin(s), t2 = _vec_cos(s))
#define vec_tan(s, t) (t = _vec_tan(s))
#define vec_sqrt(s, t) (t = _vec_sqrt(s))
#endif

#endif
