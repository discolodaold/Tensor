#ifndef RGL_H
#define RGL_H

#ifdef _WIN32
#include <windows.h>
#include <assert.h>
#endif

#include <GL/gl.h>
#include <GL/glext.h>

enum {
	RGL_NO_ERROR,
	RGL_PROGRAM_LINK_ERROR,
	RGL_NO_BINDING,
	RGL_OUT_OF_MEMORY,
	RGL_COMPILE_ERROR,
	RGL_OPENGL_ERROR,
	RGL_INVALID_ARGUMENT
};

typedef int RGLenum;

#define rgl_a(Obj, Call, ...) ((Obj)->vtable->Call(Obj, ## __VA_ARGS__))

RGLenum rgl_initialize(GLboolean (*gpa)(void **f, const GLchar *n), GLboolean dsa);

// ============================================================================

struct texture_object;

struct texture_object_vtable {
	RGLenum (*del)(struct texture_object *to);
	RGLenum (*set)(struct texture_object *to, GLint level, GLenum format, GLenum type, const GLvoid * data, ...);
	RGLenum (*set_sub)(struct texture_object *to, GLint level, GLenum format, GLenum type, const GLvoid * data, ...);
	RGLenum (*copy)(struct texture_object *to, GLint level, GLint x, GLint y, GLsizei width, GLsizei height, ...);
	RGLenum (*copy_sub)(struct texture_object *to, GLint level, GLint x, GLint y, GLsizei width, GLsizei height, ...);
	RGLenum (*bind)(struct texture_object *to, GLenum texunit);
	RGLenum (*parameterf)(struct texture_object *to, GLenum pname, GLfloat param);
	RGLenum (*generate_mipmap)(struct texture_object *to);
};

struct texture_object {
	const struct texture_object_vtable *vtable;
	GLuint  name;
	GLenum  target;
	GLint   internalformat;
	GLint   border;
	GLsizei width, height, depth;
};

RGLenum texture_object_1d_new(struct texture_object *to);
RGLenum texture_object_2d_new(struct texture_object *to);
RGLenum texture_object_3d_new(struct texture_object *to);
RGLenum texture_object_1d_array_new(struct texture_object *to);
RGLenum texture_object_2d_array_new(struct texture_object *to);
RGLenum texture_object_cube_map_new(struct texture_object *to);
RGLenum texture_object_2d_multisample_new(struct texture_object *to);
RGLenum texture_object_2d_multisample_array_new(struct texture_object *to);

// ============================================================================

struct renderbuffer_object;

struct renderbuffer_object_vtable {
	RGLenum (*del)(struct renderbuffer_object *rbo);
};

struct renderbuffer_object {
	const struct renderbuffer_object_vtable *vtable;
	GLuint  name;
	GLenum  internalformat;
	GLsizei width, height;
};

RGLenum renderbuffer_object_new(struct renderbuffer_object *rbo, GLenum internalformat, GLsizei width, GLsizei height);

// ============================================================================

struct framebuffer_object;

struct framebuffer_object_vtable {
	RGLenum (*del)(struct framebuffer_object *fbo);
	RGLenum (*color_to)(struct framebuffer_object *fbo, GLuint index, struct texture_object * to, GLint level, ...);
	RGLenum (*depth_stencil_to)(struct framebuffer_object *fbo, GLboolean depth, GLboolean stencil, struct texture_object *to, ...);
	RGLenum (*color_rbo)(struct framebuffer_object *fbo, GLuint index, struct renderbuffer_object * rbo);
	RGLenum (*depth_stencil_rbo)(struct framebuffer_object *fbo, GLboolean depth, GLboolean stencil, struct renderbuffer_object *rbo);
	RGLenum (*bind_for_drawing)(struct framebuffer_object *fbo);
	RGLenum (*bind_for_reading)(struct framebuffer_object *fbo);
};

struct framebuffer_object {
	struct framebuffer_object_vtable *vtable;
	GLuint name;
};

RGLenum framebuffer_object_unbind_drawing(void);
RGLenum framebuffer_object_unbind_reading(void);
RGLenum framebuffer_object_new(struct framebuffer_object *fbo);

// ============================================================================

struct buffer_object_map;

struct buffer_object_map_vtable {
	RGLenum (*unmap)(struct buffer_object_map *bom);
	RGLenum (*flush)(struct buffer_object_map *bom);
	RGLenum (*flush_range)(struct buffer_object_map *bom, GLintptr offset, GLsizeiptr length);
};

struct buffer_object_map {
	const struct buffer_object_map_vtable *vtable;
	GLuint     name;
	GLintptr   offset;
	GLsizeiptr length;
	GLenum     access;
	GLvoid     *pointer;
};

struct buffer_object {
	const struct buffer_object_vtable *vtable;
	GLuint  name;
	GLsizei type_size;
	GLenum  usage;
};

struct buffer_object_vtable {
	RGLenum (*del)(struct buffer_object *bo);
	RGLenum (*get)(struct buffer_object *bo, GLsizeiptr *length, GLvoid **data);
	RGLenum (*get_sub)(struct buffer_object *bo, GLintptr offset, GLsizeiptr length, GLvoid **data);
	RGLenum (*set)(struct buffer_object *bo, GLsizeiptr length, const GLvoid *data);
	RGLenum (*set_sub)(struct buffer_object *bo, GLintptr offset, GLsizeiptr length, const GLvoid *data);
	RGLenum (*map)(struct buffer_object *bo, GLenum access, struct buffer_object_map *bom);
	RGLenum (*map_range)(struct buffer_object *bo, GLintptr offset, GLsizeiptr length, GLenum access, struct buffer_object_map *bom);
	RGLenum (*local)(struct buffer_object *bo, GLsizeiptr *length, GLvoid **ptr);
	RGLenum (*get_length)(struct buffer_object *bo, GLsizeiptr *length);
	RGLenum (*set_length)(struct buffer_object *bo, GLsizeiptr length);
	RGLenum (*as_element_array)(struct buffer_object *bo);
};

RGLenum buffer_object_new(struct buffer_object *bo, GLsizeiptr type_size, GLenum usage);

// ============================================================================

struct vertex_array_connection {
	GLboolean enabled;
	GLboolean integer;
	GLuint    buffer;
	GLint     size;
	GLenum    type;
	GLboolean normalized;
	GLsizei   stride;
	GLintptr  offset;
};

struct vertex_array_object {
	const struct vertex_array_object_vtable *vtable;
	GLuint name;
};

struct vertex_array_object_vtable {
	RGLenum (*del) (struct vertex_array_object *vao);
	RGLenum (*bind) (struct vertex_array_object *vao);
	RGLenum (*get) (struct vertex_array_object *vao, GLuint index, struct vertex_array_connection *vac);
	RGLenum (*set) (struct vertex_array_object *vao, GLuint index, struct vertex_array_connection *vac);
	RGLenum (*disable) (struct vertex_array_object *vao, GLuint index);
};

RGLenum vertex_array_object_unbind(void);
RGLenum vertex_array_object_new(struct vertex_array_object *vao);

// ============================================================================

struct shader_object {
	const struct shader_object_vtable *vtable;
	GLuint name;
	GLenum type;
};

struct shader_object_vtable {
	RGLenum (*del)(struct shader_object *so);
	RGLenum (*source)(struct shader_object *so, const GLchar * src);
	RGLenum (*info_log)(struct shader_object *so, GLchar **str);
};

RGLenum vertex_shader_object_new(struct shader_object *so);
RGLenum fragment_shader_object_new(struct shader_object *so);

// ============================================================================

struct program_object {
	const struct program_object_vtable *vtable;
	GLuint    name;
	GLboolean dirty;
};

struct program_object_vtable {
	RGLenum (*del)(struct program_object *po);
	RGLenum (*attach)(struct program_object *po, size_t count, struct shader_object *sos);
	RGLenum (*detach)(struct program_object *po, size_t count, struct shader_object *sos);
	RGLenum (*bind_attribute)(struct program_object *po, const char *name, GLuint location);
	RGLenum (*bind_fragdata)(struct program_object *po, const char *name, GLuint location);
	RGLenum (*link)(struct program_object *po);
	RGLenum (*uniform_location)(struct program_object *po, const char *name, GLuint *location);
	RGLenum (*uniform1f)(struct program_object *po, const char *name, GLfloat v0);
	RGLenum (*uniform2f)(struct program_object *po, const char *name, GLfloat v0, GLfloat v1);
	RGLenum (*uniform3f)(struct program_object *po, const char *name, GLfloat v0, GLfloat v1, GLfloat v2);
	RGLenum (*uniform4f)(struct program_object *po, const char *name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
	RGLenum (*uniform1i)(struct program_object *po, const char *name, GLint v0);
	RGLenum (*uniform2i)(struct program_object *po, const char *name, GLint v0, GLint v1);
	RGLenum (*uniform3i)(struct program_object *po, const char *name, GLint v0, GLint v1, GLint v2);
	RGLenum (*uniform4i)(struct program_object *po, const char *name, GLint v0, GLint v1, GLint v2, GLint v3);
	RGLenum (*uniform3fv)(struct program_object *po, const char *name, size_t length, const GLfloat *value);
	RGLenum (*uniformMatrix3)(struct program_object *po, const char *name, const GLfloat *value);
	RGLenum (*uniformMatrix)(struct program_object *po, const char *name, const GLfloat *value);
	RGLenum (*use)(struct program_object *po);
};

RGLenum program_object_new(struct program_object *po);
RGLenum program_object_unuse(void);

// ============================================================================

struct render_component {
	const struct render_component_vtable *vtable;
	struct program_object      po;
	struct shader_object       vso;
	struct shader_object       fso;
	struct vertex_array_object vao;
	GLuint                     attribute_count;
};

struct render_component_vtable {
	RGLenum (*del)(struct render_component *rc);
	RGLenum (*bind_attribute)(struct render_component *rc, const char *name, struct vertex_array_connection *vac);
	RGLenum (*begin)(struct render_component *rc);
	RGLenum (*arrays)(struct render_component *rc, GLenum mode, GLint first, GLsizei count);
	RGLenum (*elements)(struct render_component *rc, GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
	RGLenum (*end)(struct render_component *rc);
};

RGLenum render_component_new(struct render_component *rc);

#endif
