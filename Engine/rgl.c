#include "rgl.h"

#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>

#include "../Library/user.h"

static GLenum (APIENTRYP rglGetError)(void);

#define RGL_RETURN do { \
	int err = rglGetError(); \
	assert(err == GL_NO_ERROR); \
	return RGL_NO_ERROR; \
} while(0)

#define RGL_CHECK_ERROR do { \
	int err = rglGetError(); \
	if(err != GL_NO_ERROR) { \
		assert(err == GL_NO_ERROR); \
		return RGL_NO_ERROR; \
	} \
} while(0)

// ============================================================================
// Texture Object
static GLvoid (APIENTRYP rglDeleteTextures) (GLsizei n, const GLuint * textures);
static GLvoid (APIENTRYP rglGenTextures) (GLsizei n, GLuint * textures);
static GLvoid (APIENTRYP rglTextureImage1DEXT)(GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid * pixels);
static GLvoid (APIENTRYP rglTextureImage2DEXT)(GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid * pixels);
static GLvoid (APIENTRYP rglTextureImage3DEXT)(GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid * pixels);
static GLvoid (APIENTRYP rglTextureSubImage1DEXT)(GLuint texture, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid * pixels);
static GLvoid (APIENTRYP rglTextureSubImage2DEXT)(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * pixels);
static GLvoid (APIENTRYP rglTextureSubImage3DEXT)(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid * pixels);
static GLvoid (APIENTRYP rglCopyTextureImage1DEXT)(GLuint texture, GLenum target, GLint level, GLint internalformat, GLint x, GLint y, GLsizei width, GLint border);
static GLvoid (APIENTRYP rglCopyTextureImage2DEXT)(GLuint texture, GLenum target, GLint level, GLint internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
static GLvoid (APIENTRYP rglCopyTextureSubImage1DEXT)(GLuint texture, GLenum target, GLint level, GLint internalformat, GLint xoffset, GLint x, GLint y, GLsizei width);
static GLvoid (APIENTRYP rglCopyTextureSubImage2DEXT)(GLuint texture, GLenum target, GLint level, GLint internalformat, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
static GLvoid (APIENTRYP rglCopyTextureSubImage3DEXT)(GLuint texture, GLenum target, GLint level, GLint internalformat, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
static GLvoid (APIENTRYP rglBindMultiTextureEXT)(GLenum texunit, GLenum target, GLuint texture);
static GLvoid (APIENTRYP rglTextureParameterfEXT)(GLuint texture, GLenum target, GLenum pname, GLfloat param);
static GLvoid (APIENTRYP rglGenerateTextureMipmapEXT)(GLuint texture, GLenum target);

static RGLenum texture_object_del(struct texture_object *to) {
	rglDeleteTextures(1, &to->name);
	RGL_RETURN;
}

static RGLenum texture_object_set(struct texture_object *to, GLint level, GLenum format, GLenum type, const GLvoid *pixels, ...) {
	va_list argptr;
	GLenum target;
	GLsizei dim[3];
	int d;

	target = to->target;
	d = 0;

	dim[0] = 1;
	dim[1] = 1;
	dim[2] = 1;
	va_start(argptr, pixels);
	if(GL_TEXTURE_CUBE_MAP == to->target)
		target = va_arg(argptr, GLenum);
	switch(to->target) {
	case GL_TEXTURE_3D:
		dim[d++] = va_arg(argptr, GLsizei);
	case GL_TEXTURE_CUBE_MAP:
	case GL_TEXTURE_2D:
		dim[d++] = va_arg(argptr, GLsizei);
	case GL_TEXTURE_1D:
		dim[d++] = va_arg(argptr, GLsizei);
	}

	va_end(argptr);
	switch(to->target) {
	case GL_TEXTURE_1D:
		rglTextureImage1DEXT(to->name, to->target, level, to->internalformat, dim[0], to->border, format, type, pixels);
		break;
	case GL_TEXTURE_CUBE_MAP:
	case GL_TEXTURE_2D:
		rglTextureImage2DEXT(to->name, target, level, to->internalformat, dim[0], dim[1], to->border, format, type, pixels);
		break;
	case GL_TEXTURE_3D:
		rglTextureImage3DEXT(to->name, to->target, level, to->internalformat, dim[0], dim[1], dim[2], to->border, format, type, pixels);
		break;
	}

	if(0 == level) {
		to->width = dim[0];
		to->height = dim[1];
		to->depth = dim[2];
	}

	RGL_RETURN;
}

static RGLenum texture_object_set_sub(struct texture_object *to, GLint level, GLenum format, GLenum type, const GLvoid *pixels, ...) {
	va_list argptr;
	GLenum target;
	GLint off[3];
	GLsizei dim[3];
	int d, o;

	target = to->target;
	d = 0;
	o = 0;

	off[0] = 0;
	off[1] = 0;
	off[2] = 0;
	dim[0] = 1;
	dim[1] = 1;
	dim[2] = 1;
	va_start(argptr, pixels);
	if(GL_TEXTURE_CUBE_MAP == to->target)
		target = va_arg(argptr, GLenum);
	switch(to->target) {
	case GL_TEXTURE_3D:
		off[o++] = va_arg(argptr, GLsizei);
	case GL_TEXTURE_CUBE_MAP:
	case GL_TEXTURE_2D:
		off[o++] = va_arg(argptr, GLsizei);
	case GL_TEXTURE_1D:
		off[o++] = va_arg(argptr, GLsizei);
	}

	switch(to->target) {
	case GL_TEXTURE_3D:
		dim[d++] = va_arg(argptr, GLsizei);
	case GL_TEXTURE_CUBE_MAP:
	case GL_TEXTURE_2D:
		dim[d++] = va_arg(argptr, GLsizei);
	case GL_TEXTURE_1D:
		dim[d++] = va_arg(argptr, GLsizei);
	}

	va_end(argptr);
	switch(to->target) {
	case GL_TEXTURE_1D:
		rglTextureSubImage1DEXT(to->name, to->target, level, off[0], dim[0], format, type, pixels);
		break;
	case GL_TEXTURE_CUBE_MAP:
	case GL_TEXTURE_2D:
		rglTextureSubImage2DEXT(to->name, target, level, off[0], off[1], dim[0], dim[1], format, type, pixels);
		break;
	case GL_TEXTURE_3D:
		rglTextureSubImage3DEXT(to->name, to->target, level, off[0], off[1], off[2], dim[0], dim[1], dim[2], format, type, pixels);
		break;
	}

	RGL_RETURN;
}

static RGLenum texture_object_copy(struct texture_object *to, GLint level, GLint x, GLint y, GLsizei width, GLsizei height, ...) {
	va_list argptr;
	GLenum	target;

	target = to->target;

	va_start(argptr, height);
	if(GL_TEXTURE_CUBE_MAP == to->target)
		target = va_arg(argptr, GLenum);
	va_end(argptr);
	switch(to->target) {
	case GL_TEXTURE_1D:
		rglCopyTextureImage1DEXT(to->name, to->target, level, to->internalformat, x, y, width, to->border);
		break;
	case GL_TEXTURE_CUBE_MAP:
	case GL_TEXTURE_2D:
		rglCopyTextureImage2DEXT(to->name, target, level, to->internalformat, x, y, width, height, to->border);
		break;
	}

	RGL_RETURN;
}

static RGLenum texture_object_copy_sub(struct texture_object *to, GLint level, GLint x, GLint y, GLsizei width, GLsizei height, ...) {
	va_list argptr;
	GLenum	target;
	GLint	off[3];
	int		o;

	target = to->target;
	o = 0;

	off[0] = 0;
	off[1] = 0;
	off[2] = 0;
	va_start(argptr, height);
	if(GL_TEXTURE_CUBE_MAP == to->target)
		target = va_arg(argptr, GLenum);
	switch(to->target) {
	case GL_TEXTURE_3D:
		off[o++] = va_arg(argptr, GLsizei);
	case GL_TEXTURE_CUBE_MAP:
	case GL_TEXTURE_2D:
		off[o++] = va_arg(argptr, GLsizei);
	case GL_TEXTURE_1D:
		off[o++] = va_arg(argptr, GLsizei);
	}

	va_end(argptr);
	switch(to->target) {
	case GL_TEXTURE_1D:
		rglCopyTextureSubImage1DEXT(to->name, to->target, level, to->internalformat, off[0], x, y, width);
		break;
	case GL_TEXTURE_CUBE_MAP:
	case GL_TEXTURE_2D:
		rglCopyTextureSubImage2DEXT(to->name, target, level, to->internalformat, off[0], off[1], x, y, width, height);
		break;
	case GL_TEXTURE_3D:
		rglCopyTextureSubImage3DEXT(to->name, target, level, to->internalformat, off[0], off[1], off[2], x, y, width, height);
	}

	RGL_RETURN;
}

static RGLenum texture_object_bind(struct texture_object *to, GLenum texunit) {
	rglBindMultiTextureEXT(GL_TEXTURE0 + texunit, to->target, to->name);
	RGL_RETURN;
}

static RGLenum texture_object_parameterf(struct texture_object *to, GLenum pname, GLfloat param) {
	rglTextureParameterfEXT(to->name, to->target, pname, param);
	RGL_RETURN;
}

static RGLenum texture_object_generate_mipmap(struct texture_object *to) {
	rglGenerateTextureMipmapEXT(to->name, to->target);
	RGL_RETURN;
}

static struct texture_object_vtable to_vtable = {
	texture_object_del,
	texture_object_set,
	texture_object_set_sub,
	texture_object_copy,
	texture_object_copy_sub,
	texture_object_bind,
	texture_object_parameterf,
	texture_object_generate_mipmap
};

static RGLenum texture_object_init(GLboolean (*gpa)(void **f, const GLchar *n), GLboolean dsa) {
	return
		(
			GL_TRUE &&
			gpa((void **) &rglDeleteTextures, "glDeleteTextures") &&
			gpa((void **) &rglGenTextures, "glGenTextures") &&
			(
				(
					dsa &&
					(
						GL_TRUE &&
						gpa((void **)&rglTextureImage1DEXT, "glTextureImage1DEXT") &&
						gpa((void **)&rglTextureImage2DEXT, "glTextureImage2DEXT") &&
						gpa((void **)&rglTextureImage3DEXT, "glTextureImage3DEXT") &&
						gpa((void **)&rglTextureSubImage1DEXT, "glTextureSubImage1DEXT") &&
						gpa((void **)&rglTextureSubImage2DEXT, "glTextureSubImage2DEXT") &&
						gpa((void **)&rglTextureSubImage3DEXT, "glTextureSubImage3DEXT") &&
						gpa((void **)&rglCopyTextureImage1DEXT, "glCopyTextureImage1DEXT") &&
						gpa((void **)&rglCopyTextureImage2DEXT, "glCopyTextureImage2DEXT") &&
						gpa((void **)&rglCopyTextureSubImage1DEXT, "glCopyTextureSubImage1DEXT") &&
						gpa((void **)&rglCopyTextureSubImage2DEXT, "glCopyTextureSubImage2DEXT") &&
						gpa((void **)&rglCopyTextureSubImage3DEXT, "glCopyTextureSubImage3DEXT") &&
						gpa((void **)&rglBindMultiTextureEXT, "glBindMultiTextureEXT") &&
						gpa((void **)&rglTextureParameterfEXT, "glTextureParameterfEXT") &&
						gpa((void **)&rglGenerateTextureMipmapEXT, "glGenerateTextureMipmapEXT")
					)
				) || (GL_TRUE)
			)
		) ? RGL_NO_ERROR : RGL_NO_BINDING;
}

static RGLenum texture_object_new(struct texture_object *to, GLenum target) {
	to->target = target;
	to->vtable = &to_vtable;
	rglGenTextures(1, &to->name);
	RGL_RETURN;
}

RGLenum texture_object_1d_new(struct texture_object *to) {
	return texture_object_new(to, GL_TEXTURE_1D);
}

RGLenum texture_object_2d_new(struct texture_object *to) {
	return texture_object_new(to, GL_TEXTURE_2D);
}

RGLenum texture_object_3d_new(struct texture_object *to) {
	return texture_object_new(to, GL_TEXTURE_3D);
}

RGLenum texture_object_cube_map_new(struct texture_object *to) {
	return texture_object_new(to, GL_TEXTURE_CUBE_MAP);
}

// ============================================================================
// Framebuffer Object
static GLvoid (APIENTRYP rglDeleteFramebuffers)(GLsizei n, const GLuint * renderbuffers);
static GLvoid (APIENTRYP rglBindFramebuffer)(GLenum target, GLuint framebuffer);
static GLvoid (APIENTRYP rglGenFramebuffers)(GLsizei n, GLuint * renderbuffers);
static GLvoid (APIENTRYP rglNamedFramebufferTexture1DEXT)(GLuint framebuffer, GLenum attachment, GLenum target, GLuint texture, GLint level);
static GLvoid (APIENTRYP rglNamedFramebufferTexture2DEXT)(GLuint framebuffer, GLenum attachment, GLenum target, GLuint texture, GLint level);
static GLvoid (APIENTRYP rglNamedFramebufferTexture3DEXT)(GLuint framebuffer, GLenum attachment, GLenum target, GLuint texture, GLint level, GLint layer);
static GLvoid (APIENTRYP rglNamedFramebufferRenderbufferEXT)(GLuint framebuffer, GLenum attachment, GLenum target, GLuint renderbuffer);
static GLenum (APIENTRYP rglCheckNamedFramebufferStatusEXT)(GLuint framebuffer, GLenum target);

static GLvoid (APIENTRYP rglDeleteRenderbuffers)(GLsizei n, const GLuint * renderbuffers);
static GLvoid (APIENTRYP rglGenRenderbuffers)(GLsizei n, GLuint * renderbuffers);
static GLvoid (APIENTRYP rglNamedRenderbufferStorageEXT)(GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height);

static RGLenum framebuffer_object_del(struct framebuffer_object *fbo) {
	rglDeleteFramebuffers(1, &fbo->name);
	RGL_RETURN;
}

static RGLenum framebuffer_object_color_to(struct framebuffer_object *fbo, GLuint index, struct texture_object *to, GLint level, ...) {
	va_list argptr;
	GLenum target;
	GLint layer;

	target = to->target;
	layer = 0;

	va_start(argptr, level);
	if(GL_TEXTURE_CUBE_MAP == to->target)
		target = va_arg(argptr, GLenum);
	if(GL_TEXTURE_3D == to->target)
		layer = va_arg(argptr, GLint);
	va_end(argptr);
	switch(to->target) {
	case GL_TEXTURE_1D:
		rglNamedFramebufferTexture1DEXT(fbo->name, GL_COLOR_ATTACHMENT0 + index, target, to->name, level);
		break;
	case GL_TEXTURE_CUBE_MAP:
	case GL_TEXTURE_2D:
		rglNamedFramebufferTexture2DEXT(fbo->name, GL_COLOR_ATTACHMENT0 + index, target, to->name, level);
		break;
	case GL_TEXTURE_3D:
		rglNamedFramebufferTexture3DEXT(fbo->name, GL_COLOR_ATTACHMENT0 + index, target, to->name, level, layer);
		break;
	}

	RGL_RETURN;
}

static RGLenum framebuffer_object_depth_stencil_to(struct framebuffer_object *fbo, GLboolean depth, GLboolean stencil, struct texture_object *to, ...) {
	va_list argptr;
	GLenum target;
	GLint level, layer;
	GLenum attachment;

	target = to->target;
	layer = 0;
	attachment = depth && stencil ? GL_DEPTH_STENCIL_ATTACHMENT : depth ? GL_DEPTH_ATTACHMENT : stencil ? GL_STENCIL_ATTACHMENT : GL_FALSE;

	va_start(argptr, to);
	level = va_arg(argptr, GLint);
	if(GL_TEXTURE_CUBE_MAP == to->target)
		target = va_arg(argptr, GLenum);
	if(GL_TEXTURE_3D == to->target)
		layer = va_arg(argptr, GLint);
	va_end(argptr);
	switch(to->target) {
	case GL_TEXTURE_1D:
		rglNamedFramebufferTexture2DEXT(fbo->name, attachment, target, to->name, level);
		break;
	case GL_TEXTURE_CUBE_MAP:
	case GL_TEXTURE_2D:
		rglNamedFramebufferTexture2DEXT(fbo->name, attachment, target, to->name, level);
		break;
	case GL_TEXTURE_3D:
		rglNamedFramebufferTexture3DEXT(fbo->name, attachment, target, to->name, level, layer);
		break;
	}

	RGL_RETURN;
}

static RGLenum framebuffer_object_color_rbo(struct framebuffer_object *fbo, GLuint index, struct renderbuffer_object *rbo) {
	rglNamedFramebufferRenderbufferEXT(fbo->name, GL_COLOR_ATTACHMENT0 + index, GL_RENDERBUFFER, rbo->name);
	RGL_RETURN;
}

static RGLenum framebuffer_object_depth_stencil_rbo(struct framebuffer_object *fbo, GLboolean depth, GLboolean stencil, struct renderbuffer_object *rbo) {
	GLenum	attachment;

	attachment = depth && stencil ? GL_DEPTH_STENCIL_ATTACHMENT : depth ? GL_DEPTH_ATTACHMENT : stencil ? GL_STENCIL_ATTACHMENT : GL_FALSE;

	rglNamedFramebufferRenderbufferEXT(fbo->name, attachment, GL_RENDERBUFFER, rbo->name);
	RGL_RETURN;
}

static RGLenum framebuffer_object_bind_for_drawing(struct framebuffer_object *fbo) {
	GLenum status;

	status = rglCheckNamedFramebufferStatusEXT(fbo->name, GL_DRAW_FRAMEBUFFER);

	switch(status) {
	case GL_FRAMEBUFFER_COMPLETE:
		rglBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo->name);
		RGL_RETURN;
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
	case GL_FRAMEBUFFER_UNSUPPORTED:
		break;
	}

	return RGL_OPENGL_ERROR;
}

static RGLenum framebuffer_object_bind_for_reading(struct framebuffer_object *fbo) {
	GLenum status;

	status = rglCheckNamedFramebufferStatusEXT(fbo->name, GL_READ_FRAMEBUFFER);

	switch(status) {
	case GL_FRAMEBUFFER_COMPLETE:
		rglBindFramebuffer(GL_READ_FRAMEBUFFER, fbo->name);
		RGL_RETURN;
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
	case GL_FRAMEBUFFER_UNSUPPORTED:
		break;
	}

	return RGL_OPENGL_ERROR;
}

static struct framebuffer_object_vtable fbo_vtable = {
	framebuffer_object_del,
	framebuffer_object_color_to,
	framebuffer_object_depth_stencil_to,
	framebuffer_object_color_rbo,
	framebuffer_object_depth_stencil_rbo,
	framebuffer_object_bind_for_drawing,
	framebuffer_object_bind_for_reading
};

static RGLenum framebuffer_object_init(GLboolean (*gpa) (void **f, const GLchar *n), GLboolean dsa) {
	return
		(
			GL_TRUE &&
			gpa((void **) &rglDeleteFramebuffers, "glDeleteFramebuffers") &&
			gpa((void **) &rglBindFramebuffer, "glBindFramebuffer") &&
			gpa((void **) &rglGenFramebuffers, "glGenFramebuffers") &&
			(
				(
					dsa &&
					(
						GL_TRUE &&
						gpa((void **) &rglNamedFramebufferTexture1DEXT, "glNamedFramebufferTexture1DEXT") &&
						gpa((void **) &rglNamedFramebufferTexture2DEXT, "glNamedFramebufferTexture2DEXT") &&
						gpa((void **) &rglNamedFramebufferTexture3DEXT, "glNamedFramebufferTexture3DEXT") &&
						gpa((void **) &rglNamedFramebufferRenderbufferEXT, "glNamedFramebufferRenderbufferEXT") &&
						gpa((void **) &rglCheckNamedFramebufferStatusEXT, "glCheckNamedFramebufferStatusEXT")
					)
				) || (GL_TRUE)
			)
		) ? RGL_NO_ERROR : RGL_NO_BINDING;
}

RGLenum framebuffer_object_unbind_drawing(void) {
	rglBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	RGL_RETURN;
}

RGLenum framebuffer_object_unbind_reading(void) {
	rglBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	RGL_RETURN;
}

RGLenum framebuffer_object_new(struct framebuffer_object *fbo) {
	fbo->vtable = &fbo_vtable;
	rglGenFramebuffers(1, &fbo->name);
	RGL_RETURN;
}

static RGLenum renderbuffer_object_del(struct renderbuffer_object *rbo) {
	rglDeleteRenderbuffers(1, &rbo->name);
	RGL_RETURN;
}

static struct renderbuffer_object_vtable rbo_vtable = {
	renderbuffer_object_del
};

static RGLenum renderbuffer_object_init(GLboolean (*gpa) (void **f, const GLchar *n), GLboolean dsa) {
	return
		(
			GL_TRUE &&
			gpa((void **) &rglDeleteRenderbuffers, "glDeleteRenderbuffers") &&
			gpa((void **) &rglGenRenderbuffers, "glGenRenderbuffers") &&
			(
				(dsa && (GL_TRUE && gpa((void **) &rglNamedRenderbufferStorageEXT, "glNamedRenderbufferStorageEXT"))) ||
				(GL_TRUE)
			)
		) ? RGL_NO_ERROR : RGL_NO_BINDING;
}

RGLenum renderbuffer_object_new(struct renderbuffer_object *rbo, GLenum internalformat, GLsizei width, GLsizei height) {
	rbo->vtable = &rbo_vtable;
	rglGenRenderbuffers(1, &rbo->name);
	rglNamedRenderbufferStorageEXT(rbo->name, internalformat, width, height);
	RGL_RETURN;
}

// ============================================================================
// Buffer Object
static GLvoid (APIENTRYP rglDeleteBuffers) (GLsizei n, const GLuint * buffers);
static GLvoid (APIENTRYP rglBindBuffer) (GLenum target, GLuint buffer);
static GLvoid (APIENTRYP rglGenBuffers) (GLsizei n, GLuint * buffers);

/* EXT_direct_state_access */
static GLboolean (APIENTRYP rglUnmapNamedBufferEXT)(GLuint buffer);
static GLvoid (APIENTRYP rglFlushMappedNamedBufferRangeEXT)(GLuint buffer, GLintptr offset, GLsizeiptr length);
static GLvoid (APIENTRYP rglGetNamedBufferSubDataEXT)(GLuint buffer, GLintptr offset, GLsizeiptr length, GLvoid *data);
static GLvoid (APIENTRYP rglNamedBufferDataEXT)(GLuint buffer, GLsizeiptr length, const GLvoid * data, GLenum usage);
static GLvoid (APIENTRYP rglNamedBufferSubDataEXT)(GLuint buffer, GLintptr offset, GLsizeiptr length, const GLvoid *data);
static GLvoid * (APIENTRYP rglMapNamedBufferEXT)(GLuint buffer, GLenum access);
static GLvoid * (APIENTRYP rglMapNamedBufferRangeEXT)(GLuint buffer, GLintptr offset, GLsizeiptr length, GLenum access);
static GLvoid (APIENTRYP rglGetNamedBufferParameterivEXT)(GLuint buffer, GLenum pname, GLint *params);

/* classic selector taint */
static GLboolean (APIENTRYP rglUnmapBuffer) (GLenum target);
static GLvoid (APIENTRYP rglFlushMappedBufferRange) (GLenum target, GLintptr offset, GLsizeiptr length);
static GLvoid (APIENTRYP rglGetBufferSubData) (GLenum target, GLintptr offset, GLsizeiptr length, GLvoid * data);
static GLvoid (APIENTRYP rglBufferData) (GLenum target, GLsizeiptr length, GLvoid * data, GLenum usage);
static GLvoid (APIENTRYP rglBufferSubData) (GLenum target, GLintptr offset, GLsizeiptr length, GLvoid * data);
static GLvoid *(APIENTRYP rglMapBuffer) (GLenum target, GLenum access);
static GLvoid *(APIENTRYP rglMapBufferRange) (GLenum target, GLintptr offset, GLsizeiptr length, GLenum access);
static GLvoid (APIENTRYP rglGetBufferParameteriv) (GLenum target, GLenum pname, GLint * params);

static RGLenum buffer_object_map_unmap(struct buffer_object_map *bom) {
	if(rglUnmapNamedBufferEXT(bom->name) == GL_TRUE) {
		return RGL_NO_ERROR;
	}
	RGL_RETURN;
}

static RGLenum buffer_object_map_flush(struct buffer_object_map *bom) {
	rglFlushMappedNamedBufferRangeEXT(bom->name, bom->offset, bom->length);
	RGL_RETURN;
}

static RGLenum buffer_object_map_flush_range(struct buffer_object_map *bom, GLintptr offset, GLsizeiptr length) {
	rglFlushMappedNamedBufferRangeEXT(bom->name, offset, length);
	RGL_RETURN;
}

static struct buffer_object_map_vtable bom_vtable = {
	buffer_object_map_unmap,
	buffer_object_map_flush,
	buffer_object_map_flush_range
};

static RGLenum buffer_object_del(struct buffer_object *bo) {
	rglDeleteBuffers(1, &bo->name);
	RGL_RETURN;
}

static RGLenum buffer_object_get(struct buffer_object *bo, GLsizeiptr *length, GLvoid **data) {
	if(NULL == length || NULL == data) {
		return RGL_INVALID_ARGUMENT;
	}
	bo->vtable->get_length(bo, length);
	*data = malloc(*length * bo->type_size);
	if(*data == NULL) {
		return RGL_OUT_OF_MEMORY;
	}
	rglGetNamedBufferSubDataEXT(bo->name, 0, *length, *data);
	RGL_RETURN;
}

static RGLenum buffer_object_get_sub(struct buffer_object *bo, GLintptr offset, GLsizeiptr length, GLvoid **data) {
	if(NULL == data) {
		return RGL_INVALID_ARGUMENT;
	}
	*data = malloc(length * bo->type_size);
	if(*data == NULL) {
		return RGL_OUT_OF_MEMORY;
	}
	rglGetNamedBufferSubDataEXT(bo->name, offset, length, *data);
	RGL_RETURN;
}

static RGLenum buffer_object_set(struct buffer_object *bo, GLsizeiptr length, const GLvoid *data) {
	rglNamedBufferDataEXT(bo->name, length * bo->type_size, data, bo->usage);
	RGL_RETURN;
}

static RGLenum buffer_object_set_sub(struct buffer_object *bo, GLintptr offset, GLsizeiptr size, const GLvoid *data) {
	rglNamedBufferSubDataEXT(bo->name, offset, size * bo->type_size, data);
	RGL_RETURN;
}

static RGLenum buffer_object_map(struct buffer_object *bo, GLenum access, struct buffer_object_map *bom) {
	if(NULL == bom) {
		return RGL_INVALID_ARGUMENT;
	}
	bom->vtable = &bom_vtable;
	bom->name = bo->name;
	bom->offset = 0;
	bo->vtable->get_length(bo, &bom->length);
	bom->access = access;
	bom->pointer = rglMapNamedBufferEXT(bo->name, access);
	RGL_RETURN;
}

static RGLenum buffer_object_map_range(struct buffer_object *bo, GLintptr offset, GLsizeiptr length, GLenum access, struct buffer_object_map *bom) {
	if(NULL == bom) {
		return RGL_INVALID_ARGUMENT;
	}
	bom->vtable = &bom_vtable;
	bom->name = bo->name;
	bom->offset = offset;
	bom->length = length;
	bom->access = access;
	bom->pointer = rglMapNamedBufferRangeEXT(bo->name, offset, length * bo->type_size, access);
	RGL_RETURN;
}

static RGLenum buffer_object_local(struct buffer_object *bo, GLsizeiptr *length, GLvoid **ptr) {
	if(NULL == length || NULL == ptr) {
		return RGL_INVALID_ARGUMENT;
	}
	if(0 == *length) {
		bo->vtable->get_length(bo, length);
	}
	*ptr = malloc(*length * bo->type_size);
	if(*ptr == NULL) {
		return RGL_OUT_OF_MEMORY;
	}
	RGL_RETURN;
}

static RGLenum buffer_object_get_length(struct buffer_object *bo, GLsizeiptr *length) {
	GLint	i;

	if(length == NULL) {
		return RGL_INVALID_ARGUMENT;
	}

	rglGetNamedBufferParameterivEXT(bo->name, GL_BUFFER_SIZE, &i);
	*length = i / bo->type_size;
	RGL_RETURN;
}

static RGLenum buffer_object_set_length(struct buffer_object *bo, GLsizeiptr length) {
	rglNamedBufferDataEXT(bo->name, length * bo->type_size, NULL, bo->usage);
	RGL_RETURN;
}

static RGLenum buffer_object_as_element_array(struct buffer_object *bo) {
	rglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bo->name);
	RGL_RETURN;
}

static struct buffer_object_vtable bo_vtable = {
	buffer_object_del,
	buffer_object_get,
	buffer_object_get_sub,
	buffer_object_set,
	buffer_object_set_sub,
	buffer_object_map,
	buffer_object_map_range,
	buffer_object_local,
	buffer_object_get_length,
	buffer_object_set_length,
	buffer_object_as_element_array
};

static RGLenum buffer_object_init(GLboolean (*gpa)(void **f, const GLchar *n), GLboolean dsa) {
	return
		(
			GL_TRUE &&
			gpa((void **) &rglDeleteBuffers, "glDeleteBuffers") &&
			gpa((void **) &rglBindBuffer, "glBindBuffer") &&
			gpa((void **) &rglGenBuffers, "glGenBuffers") &&
			(
				(
					dsa &&
					(
						GL_TRUE &&
						gpa((void **) &rglUnmapNamedBufferEXT, "glUnmapNamedBufferEXT") &&
						gpa((void **) &rglFlushMappedNamedBufferRangeEXT, "glFlushMappedNamedBufferRangeEXT") &&
						gpa((void **) &rglGetNamedBufferSubDataEXT, "glGetNamedBufferSubDataEXT") &&
						gpa((void **) &rglNamedBufferDataEXT, "glNamedBufferDataEXT") &&
						gpa((void **) &rglNamedBufferSubDataEXT, "glNamedBufferSubDataEXT") &&
						gpa((void **) &rglMapNamedBufferEXT, "glMapNamedBufferEXT") &&
						gpa((void **) &rglMapNamedBufferRangeEXT, "glMapNamedBufferRangeEXT") &&
						gpa((void **) &rglGetNamedBufferParameterivEXT, "glGetNamedBufferParameterivEXT")
					)
				) ||
					(
						GL_TRUE &&
						gpa((void **) &rglUnmapBuffer, "glUnmapBuffer") &&
						gpa((void **) &rglFlushMappedBufferRange, "glFlushMappedBufferRange") &&
						gpa((void **) &rglGetBufferSubData, "glGetBufferSubData") &&
						gpa((void **) &rglBufferData, "glBufferData") &&
						gpa((void **) &rglBufferSubData, "glBufferSubData") &&
						gpa((void **) &rglMapBuffer, "glMapBuffer") &&
						gpa((void **) &rglMapBufferRange, "glMapBufferRange") &&
						gpa((void **) &rglGetBufferParameteriv, "glGetBufferParameteriv")
					)
			)
		) ? RGL_NO_ERROR : RGL_NO_BINDING;
}

RGLenum buffer_object_new(struct buffer_object *bo, GLsizeiptr type_size, GLenum usage) {
	bo->vtable = &bo_vtable;
	rglGenBuffers(1, &bo->name);
	bo->type_size = type_size;
	bo->usage = usage;
	RGL_RETURN;
}

// ============================================================================
// Vertex Array Object
static GLvoid (APIENTRYP rglDeleteVertexArrays)(GLsizei n, const GLuint * arrays);
static GLvoid (APIENTRYP rglBindVertexArray)(GLuint vaobj);
static GLvoid (APIENTRYP rglGenVertexArrays)(GLsizei n, GLuint * arrays);

/* EXT_direct_state_access */
static GLvoid (APIENTRYP rglGetVertexArrayIntegeri_vEXT)(GLuint vaobj, GLuint index, GLenum pname, GLint * param);
static GLvoid (APIENTRYP rglGetVertexArrayPointeri_vEXT)(GLuint vaobj, GLuint index, GLenum pname, GLvoid **param);
static GLvoid (APIENTRYP rglVertexArrayVertexAttribIOffsetEXT)(GLuint vaobj, GLuint buffer, GLuint index, GLint size, GLenum type, GLsizei stride, GLintptr offset);
static GLvoid (APIENTRYP rglVertexArrayVertexAttribOffsetEXT)(GLuint vaobj, GLuint buffer, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLintptr offset);
static GLvoid (APIENTRYP rglEnableVertexArrayAttribEXT)(GLuint vaobj, GLuint index);
static GLvoid (APIENTRYP rglDisableVertexArrayAttribEXT)(GLuint vaobj, GLuint index);

static GLvoid (APIENTRYP rglVertexAttribIPointer)(GLuint index, GLint size, GLenum type, GLsizei stride, GLintptr offset);
static GLvoid (APIENTRYP rglVertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLintptr offset);
static GLvoid (APIENTRYP rglEnableVertexAttribArray)(GLuint vaobj, GLuint index);
static GLvoid (APIENTRYP rglDisableVertexAttribArray)(GLuint vaobj, GLuint index);

static RGLenum vertex_array_object_del(struct vertex_array_object *vao) {
	rglDeleteVertexArrays(1, &vao->name);
	RGL_RETURN;
}

static RGLenum vertex_array_object_bind(struct vertex_array_object *vao) {
	rglBindVertexArray(vao->name);
	RGL_RETURN;
}

static RGLenum vertex_array_object_get(struct vertex_array_object *vao, GLuint index, struct vertex_array_connection *vac) {
	rglGetVertexArrayIntegeri_vEXT(vao->name, index, GL_VERTEX_ATTRIB_ARRAY_ENABLED, (int *) &vac->enabled);
	rglGetVertexArrayIntegeri_vEXT(vao->name, index, GL_VERTEX_ATTRIB_ARRAY_INTEGER, (int *) &vac->integer);
	rglGetVertexArrayIntegeri_vEXT(vao->name, index, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, (int *) &vac->buffer);
	rglGetVertexArrayIntegeri_vEXT(vao->name, index, GL_VERTEX_ATTRIB_ARRAY_SIZE, (int *) &vac->size);
	rglGetVertexArrayIntegeri_vEXT(vao->name, index, GL_VERTEX_ATTRIB_ARRAY_TYPE, (int *) &vac->type);
	rglGetVertexArrayIntegeri_vEXT(vao->name, index, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED, (int *) &vac->normalized);
	rglGetVertexArrayIntegeri_vEXT(vao->name, index, GL_VERTEX_ATTRIB_ARRAY_STRIDE, (int *) &vac->stride);
	rglGetVertexArrayPointeri_vEXT(vao->name, index, GL_VERTEX_ATTRIB_ARRAY_POINTER, (void **) &vac->offset);
	RGL_RETURN;
}

static RGLenum vertex_array_object_set(struct vertex_array_object *vao, GLuint index, struct vertex_array_connection *vac) {
	if(vac->integer == GL_TRUE) {
		rglVertexArrayVertexAttribIOffsetEXT(vao->name, vac->buffer, index, vac->size, vac->type, vac->stride, vac->offset);
		RGL_CHECK_ERROR;
	} else {
		rglVertexArrayVertexAttribOffsetEXT(vao->name, vac->buffer, index, vac->size, vac->type, vac->normalized, vac->stride, vac->offset);
		RGL_CHECK_ERROR;
	}

	rglEnableVertexArrayAttribEXT(vao->name, index);
	RGL_RETURN;
}

static RGLenum vertex_array_object_disable(struct vertex_array_object *vao, GLuint index) {
	rglDisableVertexArrayAttribEXT(vao->name, index);
	RGL_RETURN;
}

static struct vertex_array_object_vtable vao_vtable = {
	vertex_array_object_del,
	vertex_array_object_bind,
	vertex_array_object_get,
	vertex_array_object_set,
	vertex_array_object_disable
};

static RGLenum vertex_array_object_init(GLboolean (*gpa) (void **f, const GLchar *n), GLboolean dsa) {
	return
		(
			GL_TRUE &&
			gpa((void **) &rglDeleteVertexArrays, "glDeleteVertexArrays") &&
			gpa((void **) &rglBindVertexArray, "glBindVertexArray") &&
			gpa((void **) &rglGenVertexArrays, "glGenVertexArrays") &&
			(
				(
					dsa &&
					(
						GL_TRUE &&
						gpa((void **) &rglGetVertexArrayIntegeri_vEXT, "glGetVertexArrayIntegeri_vEXT") &&
						gpa((void **) &rglGetVertexArrayPointeri_vEXT, "glGetVertexArrayPointeri_vEXT") &&
						gpa((void **) &rglVertexArrayVertexAttribIOffsetEXT, "glVertexArrayVertexAttribIOffsetEXT") &&
						gpa((void **) &rglVertexArrayVertexAttribOffsetEXT, "glVertexArrayVertexAttribOffsetEXT") &&
						gpa((void **) &rglEnableVertexArrayAttribEXT, "glEnableVertexArrayAttribEXT") &&
						gpa((void **) &rglDisableVertexArrayAttribEXT, "glDisableVertexArrayAttribEXT")
					)
				) ||
					(
						GL_TRUE /* && gpa((void**)&rglGetVertexArrayIntegeri_v, "glGetVertexArrayIntegeri_v") && gpa((void**)&rglGetVertexArrayPointeri_v, "glGetVertexArrayPointeri_v") */ &&
						gpa((void **) &rglVertexAttribIPointer, "glVertexAttribIPointer") &&
						gpa((void **) &rglVertexAttribPointer, "glVertexAttribPointer") &&
						gpa((void **) &rglEnableVertexAttribArray, "glEnableVertexAttribArray") &&
						gpa((void **) &rglDisableVertexAttribArray, "glDisableVertexAttribArray")
					)
			)
		) ? RGL_NO_ERROR : RGL_NO_BINDING;
}

RGLenum vertex_array_object_unbind(void) {
	rglBindVertexArray(0);
	RGL_RETURN;
}

RGLenum vertex_array_object_new(struct vertex_array_object *vao) {
	rglGenVertexArrays(1, &vao->name);
	vao->vtable = &vao_vtable;
	RGL_RETURN;
}

// ============================================================================
// Shader Object
static GLuint (APIENTRYP rglCreateShader)(GLenum type);
static GLvoid (APIENTRYP rglDeleteShader)(GLuint shader);
static GLvoid (APIENTRYP rglShaderSource)(GLuint shader, GLsizei count, const GLchar **string, const GLint * length);
static GLvoid (APIENTRYP rglCompileShader)(GLuint shader);
static GLvoid (APIENTRYP rglGetShaderiv)(GLuint shader, GLenum pname, GLint * params);
static GLvoid (APIENTRYP rglGetShaderInfoLog)(GLuint shader, GLsizei maxLength, GLsizei * length, GLchar * infoLog);

static RGLenum shader_object_del(struct shader_object *so) {
	rglDeleteShader(so->name);
	RGL_RETURN;
}

static RGLenum shader_object_source(struct shader_object *so, const GLchar *src) {
	GLint compile_status;

	rglShaderSource(so->name, 1, &src, NULL);
	RGL_CHECK_ERROR;
	rglCompileShader(so->name);
	RGL_CHECK_ERROR;
	rglGetShaderiv(so->name, GL_COMPILE_STATUS, &compile_status);
	return compile_status == GL_TRUE ? RGL_NO_ERROR : RGL_COMPILE_ERROR;
}

static RGLenum shader_object_info_log(struct shader_object *so, GLchar **str) {
	GLsizei length;

	rglGetShaderiv(so->name, GL_INFO_LOG_LENGTH, &length);
	*str = malloc(length + 1);
	if(*str == NULL) {
		return RGL_OUT_OF_MEMORY;
	}
	rglGetShaderInfoLog(so->name, length, &length, *str);
	(*str)[length] = '\0';
	RGL_RETURN;
}

static struct shader_object_vtable so_vtable = {
	shader_object_del,
	shader_object_source,
	shader_object_info_log
};

RGLenum shader_object_init(GLboolean (*gpa) (void **f, const GLchar *n), GLboolean dsa) {
	return
		(
			GL_TRUE &&
			gpa((void **) &rglCreateShader, "glCreateShader") &&
			gpa((void **) &rglDeleteShader, "glDeleteShader") &&
			gpa((void **) &rglShaderSource, "glShaderSource") &&
			gpa((void **) &rglCompileShader, "glCompileShader") &&
			gpa((void **) &rglGetShaderiv, "glGetShaderiv") &&
			gpa((void **) &rglGetShaderiv, "glGetShaderiv") &&
			gpa((void **) &rglGetShaderInfoLog, "glGetShaderInfoLog")
		) ? RGL_NO_ERROR : RGL_NO_BINDING;
}

static RGLenum shader_object_new(struct shader_object *so, GLenum type) {
	so->vtable = &so_vtable;
	so->name = rglCreateShader(type);
	so->type = type;
	RGL_RETURN;
}

RGLenum vertex_shader_object_new(struct shader_object *so) {
	return shader_object_new(so, GL_VERTEX_SHADER);
}

RGLenum fragment_shader_object_new(struct shader_object *so) {
	return shader_object_new(so, GL_FRAGMENT_SHADER);
}

// ============================================================================
// Program Object
static GLuint (APIENTRYP rglCreateProgram)(void);
static GLvoid (APIENTRYP rglDeleteProgram)(GLuint program);
static GLvoid (APIENTRYP rglAttachShader)(GLuint program, GLuint shader);
static GLvoid (APIENTRYP rglDetachShader)(GLuint program, GLuint shader);
static GLvoid (APIENTRYP rglBindAttribLocation)(GLuint program, GLuint index, const GLchar * name);
static GLvoid (APIENTRYP rglBindFragDataLocation)(GLuint program, GLuint colorNumber, const GLchar * name);
static GLvoid (APIENTRYP rglLinkProgram)(GLuint program);
static GLvoid (APIENTRYP rglGetProgramiv)(GLuint program, GLenum pname, GLint * values);
static GLvoid (APIENTRYP rglGetProgramInfoLog)(GLuint program, GLsizei maxLength, GLsizei * length, GLchar * infoLog);
static GLint (APIENTRYP rglGetUniformLocation)(GLuint program, const GLchar * name);
static GLvoid (APIENTRYP rglUseProgram)(GLuint program);

/* EXT_direct_state_access */
static GLvoid (APIENTRYP rglProgramUniform1fEXT)(GLuint program, GLint location, GLfloat v0);
static GLvoid (APIENTRYP rglProgramUniform2fEXT)(GLuint program, GLint location, GLfloat v0, GLfloat v1);
static GLvoid (APIENTRYP rglProgramUniform3fEXT)(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
static GLvoid (APIENTRYP rglProgramUniform4fEXT)(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
static GLvoid (APIENTRYP rglProgramUniform1iEXT)(GLuint program, GLint location, GLint v0);
static GLvoid (APIENTRYP rglProgramUniform2iEXT)(GLuint program, GLint location, GLint v0, GLint v1);
static GLvoid (APIENTRYP rglProgramUniform3iEXT)(GLuint program, GLint location, GLint v0, GLint v1, GLint v2);
static GLvoid (APIENTRYP rglProgramUniform4iEXT)(GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
static GLvoid (APIENTRYP rglProgramUniform3fvEXT)(GLuint program, GLint location, GLsizei count, const GLfloat *value);
static GLvoid (APIENTRYP rglProgramUniformMatrix3fvEXT)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
static GLvoid (APIENTRYP rglProgramUniformMatrix4fvEXT)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

/* classic selector taint */
static GLvoid (APIENTRYP rglUniform1f)(GLint location, GLfloat v0);
static GLvoid (APIENTRYP rglUniform2f)(GLint location, GLfloat v0, GLfloat v1);
static GLvoid (APIENTRYP rglUniform3f)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
static GLvoid (APIENTRYP rglUniform4f)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
static GLvoid (APIENTRYP rglUniform1i)(GLint location, GLint v0);
static GLvoid (APIENTRYP rglUniform2i)(GLint location, GLint v0, GLint v1);
static GLvoid (APIENTRYP rglUniform3i)(GLint location, GLint v0, GLint v1, GLint v2);
static GLvoid (APIENTRYP rglUniform4i)(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
static GLvoid (APIENTRYP rglUniform3fv)(GLint location, GLsizei count, const GLfloat * value);
static GLvoid (APIENTRYP rglUniformMatrix3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
static GLvoid (APIENTRYP rglUniformMatrix4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

static RGLenum program_object_del(struct program_object *po) {
	rglDeleteProgram(po->name);
	RGL_RETURN;
}

static RGLenum program_object_attach(struct program_object *po, size_t count, struct shader_object *sos) {
	size_t c;

	for(c = 0; c < count; c++)
		rglAttachShader(po->name, sos[c].name);
	po->dirty = GL_TRUE;
	RGL_RETURN;
}

static RGLenum program_object_detach(struct program_object *po, size_t count, struct shader_object *sos) {
	size_t c;

	for(c = 0; c < count; c++)
		rglDetachShader(po->name, sos[c].name);
	po->dirty = GL_TRUE;
	RGL_RETURN;
}

static RGLenum program_object_bind_attribute(struct program_object *po, const char *name, GLuint location) {
	rglBindAttribLocation(po->name, location, name);
	po->dirty = GL_TRUE;
	RGL_RETURN;
}

static RGLenum program_object_bind_fragdata(struct program_object *po, const char *name, GLuint location) {
	int le;

	le = po->dirty ? po->vtable->link(po) : RGL_NO_ERROR;

	if(RGL_NO_ERROR == le) {
		rglBindFragDataLocation(po->name, location, name);
		RGL_RETURN;
	}
	return le;
}

static RGLenum program_object_link(struct program_object *po) {
	GLint link_status;

	rglLinkProgram(po->name);
	po->dirty = GL_FALSE;
	rglGetProgramiv(po->name, GL_LINK_STATUS, &link_status);
	if(GL_FALSE == link_status) {
		GLsizei length;
		GLchar infoLog[1024];

		rglGetProgramInfoLog(po->name, 1024, &length, infoLog);
		return RGL_PROGRAM_LINK_ERROR;
	} else {
		RGL_RETURN;
	}
}

static RGLenum program_object_uniform_location(struct program_object *po, const char *name, GLuint *location) {
	if(NULL == location) {
		return RGL_INVALID_ARGUMENT;
	}

	if(po->dirty) {
		int le;

		if((le = po->vtable->link(po)) != RGL_NO_ERROR) {
			return le;
		}
	}

	*location = rglGetUniformLocation(po->name, name);
	RGL_RETURN;
}

static RGLenum program_object_uniform1f(struct program_object *po, const char *name, GLfloat v0) {
	GLuint loc;
	int le;

	if((le = po->vtable->uniform_location(po, name, &loc)) != RGL_NO_ERROR)
		return le;
	rglProgramUniform1fEXT(po->name, loc, v0);
	RGL_RETURN;
}

static RGLenum program_object_uniform2f(struct program_object *po, const char *name, GLfloat v0, GLfloat v1) {
	GLuint loc;
	int le;

	if((le = po->vtable->uniform_location(po, name, &loc)) != RGL_NO_ERROR)
		return le;
	rglProgramUniform2fEXT(po->name, loc, v0, v1);
	RGL_RETURN;
}

static RGLenum program_object_uniform3f(struct program_object *po, const char *name, GLfloat v0, GLfloat v1, GLfloat v2) {
	GLuint loc;
	int le;

	if((le = po->vtable->uniform_location(po, name, &loc)) != RGL_NO_ERROR)
		return le;
	rglProgramUniform3fEXT(po->name, loc, v0, v1, v2);
	RGL_RETURN;
}

static RGLenum program_object_uniform4f(struct program_object *po, const char *name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) {
	GLuint loc;
	int le;

	if((le = po->vtable->uniform_location(po, name, &loc)) != RGL_NO_ERROR)
		return le;
	rglProgramUniform4fEXT(po->name, loc, v0, v1, v2, v3);
	RGL_RETURN;
}

static RGLenum program_object_uniform1i(struct program_object *po, const char *name, GLint v0) {
	GLuint loc;
	int le;

	if((le = po->vtable->uniform_location(po, name, &loc)) != RGL_NO_ERROR)
		return le;
	rglProgramUniform1iEXT(po->name, loc, v0);
	RGL_RETURN;
}

static RGLenum program_object_uniform2i(struct program_object *po, const char *name, GLint v0, GLint v1) {
	GLuint loc;
	int le;

	if((le = po->vtable->uniform_location(po, name, &loc)) != RGL_NO_ERROR)
		return le;
	rglProgramUniform2iEXT(po->name, loc, v0, v1);
	RGL_RETURN;
}

static RGLenum program_object_uniform3i(struct program_object *po, const char *name, GLint v0, GLint v1, GLint v2) {
	GLuint loc;
	int le;

	if((le = po->vtable->uniform_location(po, name, &loc)) != RGL_NO_ERROR)
		return le;
	rglProgramUniform3iEXT(po->name, loc, v0, v1, v2);
	RGL_RETURN;
}

static RGLenum program_object_uniform4i(struct program_object *po, const char *name, GLint v0, GLint v1, GLint v2, GLint v3) {
	GLuint loc;
	int le;

	if((le = po->vtable->uniform_location(po, name, &loc)) != RGL_NO_ERROR)
		return le;
	rglProgramUniform4iEXT(po->name, loc, v0, v1, v2, v3);
	RGL_RETURN;
}

static RGLenum program_object_uniform3fv(struct program_object *po, const char *name, size_t length, const GLfloat *value) {
	GLuint loc;
	int le;

	if((le = po->vtable->uniform_location(po, name, &loc)) != RGL_NO_ERROR)
		return le;
	rglProgramUniform3fvEXT(po->name, loc, length / 3, value);
	RGL_RETURN;
}

static RGLenum program_object_uniformMatrix3(struct program_object *po, const char *name, const GLfloat *value) {
	GLuint loc;
	int le;

	if((le = po->vtable->uniform_location(po, name, &loc)) != RGL_NO_ERROR)
		return le;
	rglProgramUniformMatrix3fvEXT(po->name, loc, 1, GL_FALSE, value);
	RGL_RETURN;
}

static RGLenum program_object_uniformMatrix(struct program_object *po, const char *name, const GLfloat *value) {
	GLuint loc;
	int le;

	if((le = po->vtable->uniform_location(po, name, &loc)) != RGL_NO_ERROR)
		return le;
	rglProgramUniformMatrix4fvEXT(po->name, loc, 1, GL_FALSE, value);
	RGL_RETURN;
}

static RGLenum program_object_use(struct program_object *po) {
	int le;

	le = po->dirty ? po->vtable->link(po) : RGL_NO_ERROR;

	if(RGL_NO_ERROR == le) {
		rglUseProgram(po->name);
		RGL_RETURN;
	}
	return le;
}

static struct program_object_vtable po_vtable = {
	program_object_del,
	program_object_attach,
	program_object_detach,
	program_object_bind_attribute,
	program_object_bind_fragdata,
	program_object_link,
	program_object_uniform_location,
	program_object_uniform1f,
	program_object_uniform2f,
	program_object_uniform3f,
	program_object_uniform4f,
	program_object_uniform1i,
	program_object_uniform2i,
	program_object_uniform3i,
	program_object_uniform4i,
	program_object_uniform3fv,
	program_object_uniformMatrix3,
	program_object_uniformMatrix,
	program_object_use
};

static RGLenum program_object_init(GLboolean (*gpa)(void **f, const GLchar *n), GLboolean dsa) {
	return
		(
			GL_TRUE &&
			gpa((void **) &rglCreateProgram, "glCreateProgram") &&
			gpa((void **) &rglDeleteProgram, "glDeleteProgram") &&
			gpa((void **) &rglAttachShader, "glAttachShader") &&
			gpa((void **) &rglDetachShader, "glDetachShader") &&
			gpa((void **) &rglBindAttribLocation, "glBindAttribLocation") &&
			gpa((void **) &rglBindFragDataLocation, "glBindFragDataLocation") &&
			gpa((void **) &rglLinkProgram, "glLinkProgram") &&
			gpa((void **) &rglGetProgramiv, "glGetProgramiv") &&
			gpa((void **) &rglGetProgramInfoLog, "glGetProgramInfoLog") &&
			gpa((void **) &rglGetUniformLocation, "glGetUniformLocation") &&
			gpa((void **) &rglUseProgram, "glUseProgram") &&
			(
				(
					dsa &&
					(
						GL_TRUE &&
						gpa((void **) &rglProgramUniform1fEXT, "glProgramUniform1fEXT") &&
						gpa((void **) &rglProgramUniform2fEXT, "glProgramUniform2fEXT") &&
						gpa((void **) &rglProgramUniform3fEXT, "glProgramUniform3fEXT") &&
						gpa((void **) &rglProgramUniform4fEXT, "glProgramUniform4fEXT") &&
						gpa((void **) &rglProgramUniform1iEXT, "glProgramUniform1iEXT") &&
						gpa((void **) &rglProgramUniform2iEXT, "glProgramUniform2iEXT") &&
						gpa((void **) &rglProgramUniform3iEXT, "glProgramUniform3iEXT") &&
						gpa((void **) &rglProgramUniform4iEXT, "glProgramUniform4iEXT") &&
						gpa((void **) &rglProgramUniform3fvEXT, "glProgramUniform3fvEXT") &&
						gpa((void **) &rglProgramUniformMatrix3fvEXT, "glProgramUniformMatrix3fvEXT") &&
						gpa((void **) &rglProgramUniformMatrix4fvEXT, "glProgramUniformMatrix4fvEXT")
					)
				) ||
					(
						GL_TRUE &&
						gpa((void **) &rglUniform1f, "glUniform1f") &&
						gpa((void **) &rglUniform2f, "glUniform2f") &&
						gpa((void **) &rglUniform3f, "glUniform3f") &&
						gpa((void **) &rglUniform4f, "glUniform4f") &&
						gpa((void **) &rglUniform1i, "glUniform1i") &&
						gpa((void **) &rglUniform2i, "glUniform2i") &&
						gpa((void **) &rglUniform3i, "glUniform3i") &&
						gpa((void **) &rglUniform4i, "glUniform4i") &&
						gpa((void **) &rglUniform3fv, "glUniform3fv") &&
						gpa((void **) &rglUniformMatrix3fv, "glUniformMatrix3fv") &&
						gpa((void **) &rglUniformMatrix4fv, "glUniformMatrix4fv")
					)
			)
		) ? RGL_NO_ERROR : RGL_NO_BINDING;
}

RGLenum program_object_new(struct program_object *po) {
	po->vtable = &po_vtable;
	po->name = rglCreateProgram();
	RGL_RETURN;
}

RGLenum program_object_unuse(void) {
	rglUseProgram(0);
	RGL_RETURN;
}

// ============================================================================

static GLvoid (APIENTRYP rglDrawArrays) (GLenum mode, GLint first, GLsizei count);
static GLvoid (APIENTRYP rglDrawElements) (GLenum mode, GLsizei count, GLenum type, const GLvoid * indices);

static RGLenum render_component_del(struct render_component *rc) {
	int err;
	err = rgl_a(&rc->vao, del);
	if(err != RGL_NO_ERROR) {
		return err;
	}
	err = rgl_a(&rc->vso, del);
	if(err != RGL_NO_ERROR) {
		return err;
	}
	err = rgl_a(&rc->fso, del);
	if(err != RGL_NO_ERROR) {
		return err;
	}
	return rgl_a(&rc->po, del);
}

static RGLenum render_component_bind_attribute(struct render_component *rc, const char *name, struct vertex_array_connection *vac) {
	GLuint location;
	int err;

	location = rc->attribute_count++;

	err = rgl_a(&rc->po, bind_attribute, name, location);
	if(err != RGL_NO_ERROR) {
		return err;
	}
	return rgl_a(&rc->vao, set, location, vac);
}

static RGLenum render_component_begin(struct render_component *rc) {
	int err;
	err = rgl_a(&rc->vao, bind);
	if(err != RGL_NO_ERROR) {
		return err;
	}
	return rgl_a(&rc->po, use);
}

static RGLenum render_component_arrays(struct render_component *rc, GLenum mode, GLint first, GLsizei count) {
	rglDrawArrays(mode, first, count);
	RGL_RETURN;
}

static RGLenum render_component_elements(struct render_component *rc, GLenum mode, GLsizei count, GLenum type, const GLvoid *indices) {
	rglDrawElements(mode, count, type, indices);
	RGL_RETURN;
}

static RGLenum render_component_end(struct render_component *rc) {
	int err = vertex_array_object_unbind();
	if(err != RGL_NO_ERROR) {
		return err;
	}
	return program_object_unuse();
}

static struct render_component_vtable rc_vtable = {
	render_component_del,
	render_component_bind_attribute,
	render_component_begin,
	render_component_arrays,
	render_component_elements,
	render_component_end
};

static RGLenum render_component_init(GLboolean (*gpa) (void **f, const GLchar *n), GLboolean dsa) {
	return
		(
			GL_TRUE &&
			gpa((void **) &rglDrawArrays, "glDrawArrays") &&
			gpa((void **) &rglDrawElements, "glDrawElements")
		) ? RGL_NO_ERROR : RGL_NO_BINDING;
}

RGLenum render_component_new(struct render_component *rc) {
	int err;
	rc->vtable = &rc_vtable;
	err = program_object_new(&rc->po);
	if(err != RGL_NO_ERROR) {
		return err;
	}
	err = vertex_shader_object_new(&rc->vso);
	if(err != RGL_NO_ERROR) {
		return err;
	}
	err = fragment_shader_object_new(&rc->fso);
	if(err != RGL_NO_ERROR) {
		return err;
	}
	err = rgl_a(&rc->po, attach, 1, &rc->vso);
	if(err != RGL_NO_ERROR) {
		return err;
	}
	err = rgl_a(&rc->po, attach, 1, &rc->fso);
	if(err != RGL_NO_ERROR) {
		return err;
	}
	err = vertex_array_object_new(&rc->vao);
	if(err != RGL_NO_ERROR) {
		return err;
	}
	rc->attribute_count = 0;
	return RGL_NO_ERROR;
}

// ============================================================================

RGLenum rgl_initialize(GLboolean (*gpa)(void **f, const GLchar *n), GLboolean dsa) {
	return
		(
			GL_TRUE &&
			gpa((void **) &rglGetError, "glGetError") &&
			RGL_NO_ERROR == texture_object_init(gpa, GL_TRUE) &&
			RGL_NO_ERROR == renderbuffer_object_init(gpa, GL_TRUE) &&
			RGL_NO_ERROR == framebuffer_object_init(gpa, GL_TRUE) &&
			RGL_NO_ERROR == buffer_object_init(gpa, GL_TRUE) &&
			RGL_NO_ERROR == vertex_array_object_init(gpa, GL_TRUE) &&
			RGL_NO_ERROR == shader_object_init(gpa, GL_TRUE) &&
			RGL_NO_ERROR == program_object_init(gpa, GL_TRUE) &&
			RGL_NO_ERROR == render_component_init(gpa, GL_TRUE)
		) ? RGL_NO_ERROR : RGL_NO_BINDING;
}

