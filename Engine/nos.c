#include "nos.h"

#include <stdio.h>

int nos_init(void);
int nos_frame(void);
void nos_uninit(void);

#ifdef NOS_INPUT
static int mouse_old_x, mouse_old_y;
static int mouse_x, mouse_y;

void (*nos_keydown)(enum nos_key, unsigned int) = NULL;
void (*nos_keyup)(enum nos_key) = NULL;

void nos_mouse_delta(int *x, int *y) {
	if(x) {
		*x = mouse_old_x - mouse_x;
	}
	if(y) {
		*y = mouse_old_y - mouse_y;
	}
	mouse_old_x = mouse_x;
	mouse_old_y = mouse_y;
}

void nos_mouse_pos(int *x, int *y) {
	if(x) {
		*x = mouse_x;
	}
	if(y) {
		*y = mouse_y;
	}
}
#endif

#ifdef NOS_FONT
#include <stdarg.h>

int font_list;

void nos_draw_text(float x, float y, const char *format, ...) {
	static char buffer[1024];
	va_list ap;
	size_t len;

	va_start(ap, format);
	len = vsprintf(buffer, format, ap);
	va_end(ap);

	glPushAttrib(GL_LIST_BIT);
	glListBase(font_list);
	glRasterPos2f(x, y);
	glCallLists(len, GL_UNSIGNED_BYTE, buffer);
	glPopAttrib();
}
#endif

#ifdef _WIN32
#include <mmsystem.h>
#include <math.h>

// WGL_ARB_create_context
#define WGL_CONTEXT_MAJOR_VERSION_ARB	0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB	0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB		0x2093
#define WGL_CONTEXT_FLAGS_ARB			0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB	0x9126

#define WGL_CONTEXT_DEBUG_BIT_ARB				0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB	0x0002

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB			0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB	0x00000002

#define ERROR_INVALID_VERSION_ARB		0x2095
#define ERROR_INVALID_PROFILE_ARB		0x2096

typedef HGLRC (APIENTRY * GLCREATECONTEXTATTRIBSARBPROC)(HDC, HGLRC, const int *);

static HWND hWnd;

void *nos_gpa(const char *name) {
	return wglGetProcAddress(name);
}

int nos_time(void) {
	return timeGetTime();
}

void nos_sleep(long miliseconds) {
    Sleep(miliseconds);
}

#ifdef NOS_INPUT
enum nos_key convert_vk(WPARAM wParam) {
	int i, conv = 0;
	switch(wParam) {
	case VK_LEFT:
		conv = KEY_LEFT;
		break;
	case VK_RIGHT:
		conv = KEY_RIGHT;
		break;
	case VK_UP:
		conv = KEY_UP;
		break;
	case VK_PRIOR:
		conv = KEY_PGUP;
		break;
	case VK_NEXT:
		conv = KEY_PGDOWN;
		break;
	case VK_DOWN:
		conv = KEY_DOWN;
		break;
	case VK_SPACE:
		conv = KEY_SPACE;
		break;
	case VK_RSHIFT:
		conv = KEY_RSHIFT;
		break;
	case VK_RCONTROL:
		conv = KEY_RCONTROL;
		break;
	case VK_LSHIFT:
		conv = KEY_LSHIFT;
		break;
	case VK_LCONTROL:
		conv = KEY_LCONTROL;
		break;
	}

	for(i = KEY_A; i <= KEY_Z; i++) {
		if(wParam == (WPARAM)('A'+i-KEY_A)) {
			conv = i;
		}
		if(wParam == (WPARAM)('a'+i-KEY_A)) {
			conv = i;
		}
	}

	return (enum nos_key)conv;
}
#endif

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL (WM_MOUSELAST+1)  // message that will be supported by the OS
#endif

static LRESULT CALLBACK _wnd_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if(uMsg == WM_SYSCOMMAND && (wParam == SC_SCREENSAVE || wParam == SC_MONITORPOWER))
		return 0;

	if(uMsg == WM_CLOSE || uMsg == WM_DESTROY) {
		PostQuitMessage(0);
        return 0;
	}

#ifdef NOS_INPUT
	if(uMsg == WM_MOUSEWHEEL) {
		if(((int)wParam) > 0) {
			nos_keydown(KEY_MWHEELUP, 0);
			nos_keyup(KEY_MWHEELUP);
		} else {
			nos_keydown(KEY_MWHEELDOWN, 0);
			nos_keyup(KEY_MWHEELDOWN);
		}
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	if(uMsg == WM_CHAR && nos_keydown) {
		nos_keydown(convert_vk(wParam), wParam);
	}

	if(uMsg == WM_KEYUP && nos_keyup) {
		nos_keyup(convert_vk(wParam));
	}

	if(uMsg == WM_LBUTTONDOWN && nos_keydown) {
		nos_keydown(KEY_LBUTTON, 0);
	}

	if(uMsg == WM_RBUTTONDOWN && nos_keydown) {
		nos_keydown(KEY_RBUTTON, 0);
	}

	if(uMsg == WM_LBUTTONUP && nos_keyup) {
		nos_keyup(KEY_LBUTTON);
	}

	if(uMsg == WM_RBUTTONUP && nos_keyup) {
		nos_keyup(KEY_RBUTTON);
	}

	if(uMsg == WM_MOUSEMOVE) {
		mouse_x = (int)(short)LOWORD(lParam);
		mouse_y = (int)(short)HIWORD(lParam);
	}
#endif

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

#ifdef NDEBUG
int _fltused = 0;

void entrypoint(void) {
	HINSTANCE hInstance = GetModuleHandle(0);
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
#endif
	HDC hDC;
	HGLRC hRC;

	// create window class
	{   WNDCLASS wc;

		nos_memset(&wc, 0, sizeof(WNDCLASS));
		wc.style         = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
		wc.lpfnWndProc   = _wnd_proc;
		wc.hInstance     = hInstance;
		wc.lpszClassName = "nos";
		if(!RegisterClass(&wc))
			goto close;
	}
	// create window
	{   RECT rec;
		DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
        DWORD dwStyle   = WS_VISIBLE | WS_CAPTION | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_SYSMENU;

		rec.left   = 0;
		rec.top    = 0;
		rec.right  = NOS_XRES;
		rec.bottom = NOS_YRES;

		AdjustWindowRect(&rec, dwStyle, 0);

		hWnd = CreateWindowEx(
			dwExStyle,
			"nos",
			"nos",
			dwStyle,
            (GetSystemMetrics(SM_CXSCREEN) - rec.right + rec.left) >> 1,
            (GetSystemMetrics(SM_CYSCREEN) - rec.bottom + rec.top) >> 1,
            rec.right - rec.left,
			rec.bottom - rec.top,
			0,
			0,
			hInstance,
			0);

		if(!hWnd )
			goto close_class;
	}
	// create 2.1 context
	{	unsigned int pixel_format;
		PIXELFORMATDESCRIPTOR pfd = {
			sizeof(PIXELFORMATDESCRIPTOR),
			1,
			PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
			PFD_TYPE_RGBA,
			32,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,
			32,             // zbuffer
			0,              // stencil!
			0,
			PFD_MAIN_PLANE,
			0, 0, 0, 0
		};

		if(!(hDC = GetDC(hWnd)))
			goto close_wnd;

		if(!(pixel_format = ChoosePixelFormat(hDC, &pfd)))
			goto close_dc;

		if(!SetPixelFormat(hDC, pixel_format, &pfd))
			goto close_dc;

		if(!(hRC = wglCreateContext(hDC)))
			goto close_dc;

		if(!wglMakeCurrent(hDC, hRC))
			goto close_rc;
	}
#ifdef NOS_OPENGL3
	// create 3.2 context
	{	int attribs[] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
			WGL_CONTEXT_MINOR_VERSION_ARB, 2,
			WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
			0
		};
		GLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = (GLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
		HGLRC new_hRC = wglCreateContextAttribsARB(hDC, 0, attribs);

		wglMakeCurrent(0, 0);
		wglDeleteContext(hRC);

		hRC = new_hRC;

		if(!wglMakeCurrent(hDC, hRC))
			goto close_rc;
	}
#endif
#ifdef NOS_FONT
	{	HFONT hFont;

		hFont = CreateFont(
			-12,	// logical height of font
			0,	// logical average character width
			0,	// angle of escapement
			0,	// base-line orientation angle
			FW_BOLD,	// font weight
			FALSE,	// italic attribute flag
			FALSE,	// underline attribute flag
			FALSE,	// strikeout attribute flag
			ANSI_CHARSET,	// character set identifier
			OUT_TT_PRECIS,	// output precision
			CLIP_DEFAULT_PRECIS,	// clipping precision
			ANTIALIASED_QUALITY,	// output quality
			FF_DONTCARE|DEFAULT_PITCH,	// pitch and family
			"Courier New" 	// pointer to typeface name string
		);

		if(!hFont) {
			goto close_rc;
		}

		SelectObject(hDC, hFont);

		if((font_list = glGenLists(256)) == 0) {
			goto close_rc;
		}

		if(!wglUseFontBitmaps(hDC, 0, 255, font_list)) {
			goto close_rc;
		}
	}
#endif
	// main loop
	{	int done = nos_init();
		MSG msg;

		while(!done) {
			if(PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
				if(msg.message == WM_QUIT) {
					done |= 1;
				} else {
#ifdef NOS_INPUT
					TranslateMessage(&msg);
#endif
					DispatchMessage(&msg);
				}
			}
			done |= nos_frame();
			SwapBuffers(hDC);
        }

		nos_uninit();
    }

close_rc:
    wglMakeCurrent(0, 0);
    wglDeleteContext(hRC);

close_dc:
	ReleaseDC(hWnd, hDC);

close_wnd:
	DestroyWindow(hWnd);

close_class:
    UnregisterClass("nos", hInstance);

close:
#ifdef NDEBUG
	ExitProcess(0);
#else
	return 0;
#endif
}
#elif defined(__unix__)
#include <stdio.h>
#include <stdlib.h>
#include <GL/glx.h>
#include <time.h>
#include <sys/time.h>

void *nos_gpa(const char *name) {
	return glXGetProcAddress(name);
}

int nos_time(void) {
    struct timeval tv[1];
    gettimeofday(tv, NULL);
    return (int)((tv->tv_sec / 1000) + (tv->tv_usec * 1000));
}

void nos_sleep(long miliseconds) {
    nanosleep(&(struct timespec){.tv_sec = 0, .tv_nsec = miliseconds * 1000000}, NULL);
}

#ifdef NOS_INPUT
enum nos_key convert_xk(int code) {
	int conv = 0;
	switch(code) {
	case XK_Left:
		conv = KEY_LEFT;
		break;
	case XK_Right:
		conv = KEY_RIGHT;
		break;
	case XK_Up:
		conv = KEY_UP;
		break;
	case XK_Prior:
		conv = KEY_PGUP;
		break;
	case XK_Next:
		conv = KEY_PGDOWN;
		break;
	case XK_Down:
		conv = KEY_DOWN;
		break;
	case XK_space:
		conv = KEY_SPACE;
		break;
	case XK_Shift_R:
		conv = KEY_RSHIFT;
		break;
	case XK_Control_R:
		conv = KEY_RCONTROL;
		break;
	case XK_Shift_L:
		conv = KEY_LSHIFT;
		break;
	case XK_Control_L:
		conv = KEY_LCONTROL;
		break;
	}

    if(code >= 'a' && code <= 'z') {
        return KEY_A + (code - 'a');
    }

	return (enum nos_key)conv;
}

enum nos_key convert_button(int button) {
    switch(button) {
    case Button1:
        return KEY_LBUTTON;
    case Button3:
        return KEY_RBUTTON;
    case Button4:
        return KEY_MWHEELUP;
    case Button5:
        return KEY_MWHEELDOWN;
    }
    return 0;
}
#endif

int main(int argc, char *argv[]) {
    Display *display;
    Window window;
    GLXContext context;

    // open a connection to X server
    {   display = XOpenDisplay(NULL);
        if(!display) {
            goto close;
        }
    }
    // create a window
    {   XVisualInfo *visual_info;
        XSetWindowAttributes swa;
        XEvent event;
        int buffer_attributes[] = {
            GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
            GLX_RENDER_TYPE,   GLX_RGBA_BIT,
            GLX_DOUBLEBUFFER,  True,  /* Request a double-buffered color buffer with */
            GLX_RED_SIZE,      1,     /* the maximum number of bits per component    */
            GLX_GREEN_SIZE,    1,
            GLX_BLUE_SIZE,     1,
            None
        };

        visual_info = glXChooseVisual(display, DefaultScreen(display), buffer_attributes);

        swa.background_pixel = 0;
        swa.border_pixel = 0;
        swa.colormap = XCreateColormap(display, RootWindow(display, visual_info->screen), visual_info->visual, AllocNone);
        swa.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask | ButtonMotionMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask;

        window = XCreateWindow(display, RootWindow(display, visual_info->screen), 0, 0, NOS_XRES, NOS_YRES, 0, visual_info->depth, InputOutput, visual_info->visual, CWBorderPixel | CWColormap | CWEventMask, &swa);

       /* set hints and properties */
       {
          XSizeHints sizehints;
          sizehints.width  = NOS_XRES;
          sizehints.height = NOS_YRES;
          sizehints.flags = USSize;
          XSetNormalHints(display, window, &sizehints);
          XSetStandardProperties(display, window, "nos", "nos", None, (char **)NULL, 0, &sizehints);
       }

        // Create a GLX context for OpenGL rendering
        context = glXCreateContext(display, visual_info, NULL, True);
        XFree(visual_info);

        if(context == NULL) {
            goto destroy_window;
        }

        // Map the window to the screen, and wait for it to appear
        XMapWindow(display, window);

        // Bind the GLX context to the Window
        glXMakeCurrent(display, window, context);
    }
#ifdef NOS_FONT
    {   XFontStruct *font = XLoadQueryFont(display, "fixed");
        int first = font->min_char_or_byte2;
        int last = font->max_char_or_byte2;
        glXUseXFont(font->fid, first, last-first+1, font_list+first);
    }
#endif
    // loop
	{	int done = nos_init();
		while(!done) {
            while(XPending(display) > 0) {
                XEvent event;
                XNextEvent(display, &event);
                switch(event.type) {
                case ConfigureNotify:
                    //reshape(event.xconfigure.width, event.xconfigure.height);
                    break;
#ifdef NOS_INPUT
                case ButtonPress:
                    if(nos_keydown) {
                        nos_keydown(convert_button(event.xbutton.button), 0);
                    }
                    break;
                case ButtonRelease:
                    if(nos_keyup) {
                        nos_keyup(convert_button(event.xbutton.button));
                    }
                    break;
                case MotionNotify:
                    mouse_x = event.xmotion.x;
                    mouse_y = event.xmotion.y;
                    break;
                case KeyPress:
                    if(nos_keydown) {
                        char buffer[10];
                        XLookupString(&event.xkey, buffer, 10, NULL, NULL);
                        nos_keydown(convert_xk(XLookupKeysym(&event.xkey, 0)), buffer[0]);
                    }
                    break;
                case KeyRelease:
                    if(nos_keyup) {
                        nos_keyup(convert_xk(XLookupKeysym(&event.xkey, 0)));
                    }
                    break;
#endif
                default:
                    break;
                }
            }
			done |= nos_frame();
            glXSwapBuffers(display, window);
        }

		nos_uninit();
    }

destroy_context:
   glXDestroyContext(display, context);

destroy_window:
   XDestroyWindow(display, window);

close_display:
   XCloseDisplay(display);

close:
    exit(EXIT_SUCCESS);
}
#endif

static int _seed;

int nos_rand(void) {
    _seed = _seed * 0x343FD + 0x269EC3;
    return (_seed >> 16) & 32767;
}

float nos_frand(void) {
	unsigned int a;
	float res;
    _seed = _seed * 0x343FD + 0x269EC3;
    a = (((unsigned int)_seed) >> 9) | 0x3f800000;
    res = (*((float*)&a)) - 1.0f;
    return res;
}

float nos_sfrand(void) {
	unsigned int a;
	float res;
    _seed = _seed * 0x343FD + 0x269EC3;
    a = (((unsigned int)_seed) >> 9) | 0x40000000;
    res = (*((float*)&a)) - 3.0f;
    return res;
}

void nos_memset(void *dst, int val, int len) {
#ifdef _MSC_VER
	__asm {
		mov edi, dst
		mov eax, val
		mov ecx, len
		rep stosb
	}
#else
	unsigned char *d = (unsigned char *)dst;
	while(len--) {
		*d++ = val;
	}
#endif
}

void nos_memcpy(void *dst, void *src, int len) {
#ifdef _MSC_VER
	__asm {
		mov edi, dst
		mov esi, src
		mov ecx, len
		rep movsb
	}
#else
	unsigned char *d = (unsigned char *)dst;
	unsigned char *s = (unsigned char *)src;
	while(len--) {
		*d++ = *s++;
	}
#endif
}

#define radians(X) (X * 3.1415f / 360.0f)

#ifdef _MSC_VER
#else
#include <math.h>
float _nos_sin(float x) {
	return sin(x);
}

float _nos_cos(float x) {
	return cos(x);
}

float _nos_tan(float x) {
	return tan(x);
}

float _nos_sqrt(float x) {
	return sqrt(x);
}
#endif

float nos_isqrt(float x) {
	long i;
	float x2, y;
	const float threehalfs = 1.5f;

	x2 = x * 0.5f;
	y  = x;
	i  = *(long *)&y;
	i  = 0x5f3759df - (i >> 1);
	y  = *(float *)&i;

	return y * (threehalfs - (x2 * y * y));
}

void nos_vector_add(const float v1[3], const float v2[3], float out[3]) {
	int i;
	for(i = 0; i < 3; ++i) {
		out[i] = v1[i] + v2[i];
	}
}

void nos_vector_mul(const float v1[3], const float v2[3], float out[3]) {
	int i;
	for(i = 0; i < 3; ++i) {
		out[i] = v1[i] * v2[i];
	}
}

void nos_vector_scale(const float v[3], float s, float out[3]) {
	int i;
	for(i = 0; i < 3; ++i) {
		out[i] = v[i]*s;
	}
}

void nos_vector_mad(const float v1[3], float s, const float v2[3], float out[3]) {
	int i;
	for(i = 0; i < 3; ++i) {
		out[i] = v1[i]*s + v2[i];
	}
}

void nos_vector_lerp(const float v1[3], const float v2[3], float s, float out[3]) {
	int i;
	for(i = 0; i < 3; ++i) {
		out[i] = v1[i]*s + v2[i]*(1.0f-s);
	}
}

void nos_vector_min(const float v1[3], const float v2[3], float out[3]) {
	int i;
	for(i = 0; i < 3; ++i) {
		out[i] = v1[i] < v2[i] ? v1[i] : v2[i];
	}
}

void nos_vector_max(const float v1[3], const float v2[3], float out[3]) {
	int i;
	for(i = 0; i < 3; ++i) {
		out[i] = v1[i] > v2[i] ? v1[i] : v2[i];
	}
}

float nos_vector_normalize_into(const float v[3], float out[3]) {
	float len = nos_vector_dot(v, v);
	nos_sqrt(len, len);
	if(len > 0.0f) {
		nos_vector_scale(v, 1.0f / len, out);
	}
	return len;
}

float nos_vector_dot(const float v1[3], const float v2[3]) {
	return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}

void nos_vector_cross(const float v1[3], const float v2[3], float out[3]) {
	int i;
	for(i = 0; i < 3; ++i) {
		out[i] = v1[(i + 1) % 3]*v2[(i + 2) % 3] - v1[(i + 2) % 3]*v2[(i + 1) % 3];
	}
}

void nos_matrix_identity(float m[16]) {
	m[0] = m[5] = m[10] = m[15] = 1.0f;
	m[1] = m[2] = m[3] = m[4] = m[6] = m[7] = m[8] = m[9] = m[11] = m[12] = m[13] = m[14] = 0.0f;
}

void nos_matrix_mult(const float m1[16], const float m2[16], float out[16]) {
	out[ 0] = m1[ 0]*m2[ 0] + m1[ 4]*m2[ 1] + m1[ 8]*m2[ 2] + m1[12]*m2[ 3];
	out[ 1] = m1[ 1]*m2[ 0] + m1[ 5]*m2[ 1] + m1[ 9]*m2[ 2] + m1[13]*m2[ 3];
	out[ 2] = m1[ 2]*m2[ 0] + m1[ 6]*m2[ 1] + m1[10]*m2[ 2] + m1[14]*m2[ 3];
	out[ 3] = m1[ 3]*m2[ 0] + m1[ 7]*m2[ 1] + m1[11]*m2[ 2] + m1[15]*m2[ 3];
	out[ 4] = m1[ 0]*m2[ 4] + m1[ 4]*m2[ 5] + m1[ 8]*m2[ 6] + m1[12]*m2[ 7];
	out[ 5] = m1[ 1]*m2[ 4] + m1[ 5]*m2[ 5] + m1[ 9]*m2[ 6] + m1[13]*m2[ 7];
	out[ 6] = m1[ 2]*m2[ 4] + m1[ 6]*m2[ 5] + m1[10]*m2[ 6] + m1[14]*m2[ 7];
	out[ 7] = m1[ 3]*m2[ 4] + m1[ 7]*m2[ 5] + m1[11]*m2[ 6] + m1[15]*m2[ 7];
	out[ 8] = m1[ 0]*m2[ 8] + m1[ 4]*m2[ 9] + m1[ 8]*m2[10] + m1[12]*m2[11];
	out[ 9] = m1[ 1]*m2[ 8] + m1[ 5]*m2[ 9] + m1[ 9]*m2[10] + m1[13]*m2[11];
	out[10] = m1[ 2]*m2[ 8] + m1[ 6]*m2[ 9] + m1[10]*m2[10] + m1[14]*m2[11];
	out[11] = m1[ 3]*m2[ 8] + m1[ 7]*m2[ 9] + m1[11]*m2[10] + m1[15]*m2[11];
	out[12] = m1[ 0]*m2[12] + m1[ 4]*m2[13] + m1[ 8]*m2[14] + m1[12]*m2[15];
	out[13] = m1[ 1]*m2[12] + m1[ 5]*m2[13] + m1[ 9]*m2[14] + m1[13]*m2[15];
	out[14] = m1[ 2]*m2[12] + m1[ 6]*m2[13] + m1[10]*m2[14] + m1[14]*m2[15];
	out[15] = m1[ 3]*m2[12] + m1[ 7]*m2[13] + m1[11]*m2[14] + m1[15]*m2[15];
}

void nos_matrix_perspective(float m[16], float fovy, float aspect, float zNear, float zFar) {
	float range = radians(fovy / 2.0f), left, right, bottom, top;

	nos_tan(range, range);
	range *= zNear;
	left = -range * aspect;
	right = range * aspect;
	bottom = -range;
	top = range;

	{	float m2[16] = {
			(2.0f * zNear) / (right - left), 0.0f, 0.0f, 0.0f,
			0.0f, (2.0f * zNear) / (top - bottom), 0.0f, 0.0f,
			0.0f, 0.0f, -(zFar + zNear) / (zFar - zNear), -1.0f,
			0.0f, 0.0f, -(2.0f * zFar * zNear) / (zFar - zNear), 0.0f
		}, m1[16];
		nos_memcpy(m1, m, sizeof(m1));
#ifndef MATRIX_FAST
		nos_matrix_mult(m1, m2, m);
#else
		m[ 0] *= m2[ 0];
		m[ 1] *= m2[ 0];
		m[ 2] *= m2[ 0];
		m[ 3] *= m2[ 0];
		m[ 4] *= m2[ 5];
		m[ 5] *= m2[ 5];
		m[ 6] *= m2[ 5];
		m[ 7] *= m2[ 5];
		m[ 8] = m1[ 8]*m2[10] - m[12];
		m[ 9] = m1[ 9]*m2[10] - m[13];
		m[10] = m1[10]*m2[10] - m[14];
		m[11] = m1[11]*m2[10] - m[15];
		m[12] = m1[ 8]*m2[14];
		m[13] = m1[ 9]*m2[14];
		m[14] = m1[10]*m2[14];
		m[15] = m1[11]*m2[14];
#endif
	}
}

void nos_matrix_translate(float m[16], float x, float y, float z) {
	m[12] += m[ 0]*x + m[ 4]*y + m[ 8]*z;
	m[13] += m[ 1]*x + m[ 5]*y + m[ 9]*z;
	m[14] += m[ 2]*x + m[ 6]*y + m[10]*z;
	m[15] += m[ 3]*x + m[ 7]*y + m[11]*z;
}

void nos_matrix_rotate(float m[16], float angle, float x, float y, float z) {
	float v[3] = {x, y, z}, axis[3], temp[3];
	float c, s;
	float m2[9], m1[16];

	angle = radians(angle);
	nos_sincos(angle, s, c);

	nos_vector_normalize_into(v, axis);
	temp[0] = (1 - c) * axis[0];
	temp[1] = (1 - c) * axis[1];
	temp[2] = (1 - c) * axis[2];

	m2[ 0] = c + temp[0] * axis[0];
	m2[ 1] =     temp[0] * axis[1] + s * axis[2];
	m2[ 2] =     temp[0] * axis[2] - s * axis[1];

	m2[ 3] =     temp[1] * axis[0] - s * axis[2];
	m2[ 4] = c + temp[1] * axis[1];
	m2[ 5] =     temp[1] * axis[2] + s * axis[0];

	m2[ 6] =     temp[2] * axis[0] + s * axis[1];
	m2[ 7] =     temp[2] * axis[1] - s * axis[0];
	m2[ 8] = c + temp[2] * axis[2];

	nos_memcpy(m1, m, sizeof(m1));
#ifndef MATRIX_FAST
	nos_matrix_mult(m1, m2, m);
#else
	m[ 0] = m1[ 0]*m2[ 0] + m1[ 4]*m2[ 1] + m1[ 8]*m2[ 2];
	m[ 1] = m1[ 1]*m2[ 0] + m1[ 5]*m2[ 1] + m1[ 9]*m2[ 2];
	m[ 2] = m1[ 2]*m2[ 0] + m1[ 6]*m2[ 1] + m1[10]*m2[ 2];
	m[ 3] = m1[ 3]*m2[ 0] + m1[ 7]*m2[ 1] + m1[11]*m2[ 2];
	m[ 4] = m1[ 0]*m2[ 3] + m1[ 4]*m2[ 4] + m1[ 8]*m2[ 5];
	m[ 5] = m1[ 1]*m2[ 3] + m1[ 5]*m2[ 4] + m1[ 9]*m2[ 5];
	m[ 6] = m1[ 2]*m2[ 3] + m1[ 6]*m2[ 4] + m1[10]*m2[ 5];
	m[ 7] = m1[ 3]*m2[ 3] + m1[ 7]*m2[ 4] + m1[11]*m2[ 5];
	m[ 8] = m1[ 0]*m2[ 6] + m1[ 4]*m2[ 7] + m1[ 8]*m2[ 8];
	m[ 9] = m1[ 1]*m2[ 6] + m1[ 5]*m2[ 7] + m1[ 9]*m2[ 8];
	m[10] = m1[ 2]*m2[ 6] + m1[ 6]*m2[ 7] + m1[10]*m2[ 8];
	m[11] = m1[ 3]*m2[ 6] + m1[ 7]*m2[ 7] + m1[11]*m2[ 8];
#endif
}

void nos_matrix_scale(float m[16], float x, float y, float z) {
	m[ 0] *= x;
	m[ 1] *= x;
	m[ 2] *= x;
	m[ 3] *= x;
	m[ 4] *= y;
	m[ 5] *= y;
	m[ 6] *= y;
	m[ 7] *= y;
	m[ 8] *= z;
	m[ 9] *= z;
	m[10] *= z;
	m[11] *= z;
}

void nos_matrix_look_at(float m[16], float eyeX, float eyeY, float eyeZ, float centerX, float centerY, float centerZ, float upX, float upY, float upZ) {
	float f[3] = {centerX - eyeX, centerY - eyeY, centerZ - eyeZ};
	float u[3] = {upX, upY, upZ};
	float s[3], m1[16];

	nos_vector_normalize(f);
	nos_vector_normalize(u);

	nos_vector_cross(f, u, s);
	nos_vector_normalize(s);

	nos_vector_cross(s, f, u);

	nos_memcpy(m1, m, sizeof(m1));
#ifndef MATRIX_FAST
	{	float m2[16] = {
			s[0], u[0], -f[0], 0.0f,
			s[1], u[1], -f[1], 0.0f,
			s[2], u[2], -f[2], 0.0f,
			0.0f, 0.0f, 0.0f,  1.0f
		};
		nos_matrix_mult(m1, m2, m);
	}
#else
	m[ 0] = m1[ 0]*s[0] + m1[ 4]*u[0] - m1[ 8]*f[0];
	m[ 1] = m1[ 1]*s[0] + m1[ 5]*u[0] - m1[ 9]*f[0];
	m[ 2] = m1[ 2]*s[0] + m1[ 6]*u[0] - m1[10]*f[0];
	m[ 3] = m1[ 3]*s[0] + m1[ 7]*u[0] - m1[11]*f[0];
	m[ 4] = m1[ 0]*s[1] + m1[ 4]*u[1] - m1[ 8]*f[1];
	m[ 5] = m1[ 1]*s[1] + m1[ 5]*u[1] - m1[ 9]*f[1];
	m[ 6] = m1[ 2]*s[1] + m1[ 6]*u[1] - m1[10]*f[1];
	m[ 7] = m1[ 3]*s[1] + m1[ 7]*u[1] - m1[11]*f[1];
	m[ 8] = m1[ 0]*s[2] + m1[ 4]*u[2] - m1[ 8]*f[2];
	m[ 9] = m1[ 1]*s[2] + m1[ 5]*u[2] - m1[ 9]*f[2];
	m[10] = m1[ 2]*s[2] + m1[ 6]*u[2] - m1[10]*f[2];
	m[11] = m1[ 3]*s[2] + m1[ 7]*u[2] - m1[11]*f[2];
#endif

	nos_matrix_translate(m, -eyeX, -eyeY, -eyeZ);
}

void nos_matrix_mult_vector(const float in[3], const float m[16], float out[3]) {
	float m1[4] = {in[0], in[1], in[2], in[3]};
	out[0] = m[ 0]*m1[0] + m[ 4]*m1[1] + m[ 8]*m1[2] + m[12]*m1[3];
	out[1] = m[ 1]*m1[0] + m[ 5]*m1[1] + m[ 9]*m1[2] + m[13]*m1[3];
	out[2] = m[ 2]*m1[0] + m[ 6]*m1[1] + m[10]*m1[2] + m[14]*m1[3];
	out[3] = m[ 3]*m1[0] + m[ 7]*m1[1] + m[11]*m1[2] + m[15]*m1[3];
}
