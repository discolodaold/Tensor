#ifndef NOS_XRES
#define NOS_XRES 800
#endif

#ifndef NOS_YRES
#define NOS_YRES 600
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>
#define GLAPI
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

#include <GL/gl.h>

void *nos_gpa(const char *);

#ifdef NOS_INPUT
enum nos_key {
	KEY_LEFT,
	KEY_RIGHT,
	KEY_UP,
	KEY_DOWN,
	KEY_SPACE,
	KEY_PGUP,
	KEY_PGDOWN,
	KEY_LSHIFT,
	KEY_RSHIFT,
	KEY_LCONTROL,
	KEY_RCONTROL,

	KEY_0,
	KEY_1,
	KEY_2,
	KEY_3,
	KEY_4,
	KEY_5,
	KEY_6,
	KEY_7,
	KEY_8,
	KEY_9,

	KEY_A,
	KEY_B,
	KEY_C,
	KEY_D,
	KEY_E,
	KEY_F,
	KEY_G,
	KEY_H,
	KEY_I,
	KEY_J,
	KEY_K,
	KEY_L,
	KEY_M,
	KEY_N,
	KEY_O,
	KEY_P,
	KEY_Q,
	KEY_R,
	KEY_S,
	KEY_T,
	KEY_U,
	KEY_V,
	KEY_W,
	KEY_X,
	KEY_Y,
	KEY_Z,

	KEY_LBUTTON,
	KEY_RBUTTON,
	KEY_MWHEELUP,
	KEY_MWHEELDOWN,

	KEY_NUM
};

extern void (*nos_keydown)(enum nos_key, unsigned int);
extern void (*nos_keyup)(enum nos_key);
int nos_key(enum nos_key);

void nos_mouse_delta(int *, int *);
void nos_mouse_pos(int *, int *);
#endif

#ifdef NOS_FONT
void nos_draw_text(float x, float y, const char *format, ...);
#endif

int nos_time(void);
void nos_sleep(long);

int nos_rand(void);
float nos_frand(void);
float nos_sfrand(void);

void nos_memset(void *, int, int);
void nos_memcpy(void *, void *, int);

#ifdef _MSC_VER
#define nos_sin(s, t) __asm fld dword ptr [s] __asm fsin __asm fstp dword ptr [t]
#define nos_cos(s, t) __asm fld dword ptr [s] __asm fcos __asm fstp dword ptr [t]
#define nos_sincos(s, t1, t2) __asm fld dword ptr [s] __asm fsincos __asm fstp dword ptr [t2] __asm fstp dword ptr [t1]
#define nos_tan(s, t) __asm fld dword ptr [s] __asm fptan __asm fstp st __asm fstp dword ptr [t]
#define nos_sqrt(s, t) __asm fld dword ptr [s] __asm fsqrt __asm fstp dword ptr [t]
#else
float _nos_sin(float);
float _nos_cos(float);
float _nos_tan(float);
float _nos_sqrt(float);

#define nos_sin(s, t) (t = _nos_sin(s))
#define nos_cos(s, t) (t = _nos_cos(s))
#define nos_sincos(s, t1, t2) (t1 = _nos_sin(s), t2 = _nos_cos(s))
#define nos_tan(s, t) (t = _nos_tan(s))
#define nos_sqrt(s, t) (t = _nos_sqrt(s))
#endif

void nos_vector_add(const float v1[3], const float v2[3], float out[3]);
void nos_vector_mul(const float v1[3], const float v2[3], float out[3]);
void nos_vector_scale(const float v[3], float s, float out[3]);
#define nos_vector_copy(v1, out) do { nos_vector_scale(v1, 1.0f, out); } while(0)

void nos_vector_mad(const float v1[3], float s, const float v2[3], float out[3]);
void nos_vector_lerp(const float v1[3], const float v2[3], float s, float out[3]);

void nos_vector_min(const float v1[3], const float v2[3], float out[3]);
void nos_vector_max(const float v1[3], const float v2[3], float out[3]);

float nos_vector_normalize_into(const float v[3], float out[3]);
#define nos_vector_normalize(v) nos_vector_normalize_into(v, v)
float nos_vector_dot(const float v1[3], const float v2[3]);
void nos_vector_cross(const float v1[3], const float v2[3], float out[3]);

void nos_matrix_identity(float m[16]);
void nos_matrix_perspective(float m[16], float fovy, float aspect, float zNear, float zFar);
void nos_matrix_translate(float m[16], float x, float y, float z);
void nos_matrix_rotate(float m[16], float angle, float x, float y, float z);
void nos_matrix_scale(float m[16], float x, float y, float z);
void nos_matrix_look_at(float m[16], float eyeX, float eyeY, float eyeZ, float centerX, float centerY, float centerZ, float upX, float upY, float upZ);
void nos_matrix_mult_vector(const float in[3], const float m[16], float out[3]);
