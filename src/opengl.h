#pragma once

#include "km_defines.h"

// X Macro trickery for declaring required OpenGL functions
// The general FUNC macro has the form
//		FUNC( return type, function name, arg1, arg2, arg3, arg4, ... )
// This macro will be used for:
//	- Declaring the functions
//	- Declaring pointers to the functions in struct OpenGLFunctions
//	- Loading the functions in platform layers
//	- More stuff, probably, as time goes on
#define GL_FUNCTIONS_BASE \
	FUNC(void,				glViewport,		GLint x, GLint y, \
                                            GLsizei width, GLsizei height) \
	FUNC(const GLubyte*,	glGetString,	GLenum name) \
	FUNC(void,				glClear,		GLbitfield mask) \
	FUNC(void,				glClearColor,	GLclampf r, GLclampf g, \
                                            GLclampf b, GLclampf a) \
	FUNC(void,				glClearDepth,	GLdouble depth)

#define GL_FUNCTIONS_ALL \
	FUNC(void,	glEnable, GLenum cap) \
	FUNC(void,	glDisable, GLenum cap) \
	FUNC(void,	glBlendFunc, GLenum sfactor, GLenum dfactor) \
	FUNC(void,	glDepthFunc, GLenum func) \
	FUNC(void,	glDepthRange, GLdouble near, GLdouble far) \
\
	FUNC(GLuint, glCreateShader, GLenum type) \
	FUNC(GLuint, glCreateProgram) \
	FUNC(void,	glShaderSource, GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length) \
	FUNC(void,	glCompileShader, GLuint shader) \
	FUNC(void,	glGetShaderiv, GLuint shader, GLenum pname, GLint* params) \
	FUNC(void,	glGetShaderInfoLog, GLuint shader, GLsizei bufSize, GLsizei *length, GLchar* infoLog) \
	FUNC(void,	glGetProgramInfoLog, GLuint program, GLsizei bufSize, GLsizei *length, GLchar* infoLog) \
	FUNC(void,	glAttachShader, GLuint program, GLuint shader) \
	FUNC(void,	glLinkProgram, GLuint program) \
	FUNC(void,	glGetProgramiv, GLuint program, GLenum pname, GLint *params) \
	FUNC(void,	glDetachShader, GLuint program, GLuint shader) \
	FUNC(void,	glDeleteProgram, GLuint program) \
	FUNC(void,	glDeleteShader, GLuint shader) \
\
	FUNC(void,	glGenBuffers, GLsizei n, GLuint* buffers) \
	FUNC(void,	glBindBuffer, GLenum target, GLuint buffer) \
	FUNC(void,	glBufferData, GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage) \
	FUNC(void,	glBindVertexArray, GLuint array) \
	FUNC(void,	glDeleteVertexArrays, GLsizei n, const GLuint* arrays) \
	FUNC(void,	glGenVertexArrays, GLsizei n, GLuint* arrays) \
	FUNC(void,	glEnableVertexAttribArray, GLuint index) \
	FUNC(void,	glVertexAttribPointer, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer) \
\
	FUNC(void,	glUseProgram, GLuint program) \
	FUNC(GLint,	glGetUniformLocation, GLuint program, const GLchar* name) \
	FUNC(void,	glUniform1f, GLint location, GLfloat v0) \
	FUNC(void,	glUniform2f, GLint location, GLfloat v0, GLfloat v1) \
	FUNC(void,	glUniform3f, GLint location, GLfloat v0, GLfloat v1, GLfloat v2) \
	FUNC(void,	glUniform4f, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) \
	FUNC(void,	glUniform1i, GLint location, GLint v0) \
	FUNC(void,	glUniform2i, GLint location, GLint v0, GLint v1) \
	FUNC(void,	glUniform3i, GLint location, GLint v0, GLint v1, GLint v2) \
	FUNC(void,	glUniform4i, GLint location, GLint v0, GLint v1, GLint v2, GLint v3) \
	FUNC(void,	glUniform1fv, GLint location, GLsizei count, const GLfloat *value) \
	FUNC(void,	glUniform2fv, GLint location, GLsizei count, const GLfloat *value) \
	FUNC(void,	glUniform3fv, GLint location, GLsizei count, const GLfloat *value) \
	FUNC(void,	glUniform4fv, GLint location, GLsizei count, const GLfloat *value) \
	FUNC(void,	glUniform1iv, GLint location, GLsizei count, const GLint *value) \
	FUNC(void,	glUniform2iv, GLint location, GLsizei count, const GLint *value) \
	FUNC(void,	glUniform3iv, GLint location, GLsizei count, const GLint *value) \
	FUNC(void,	glUniform4iv, GLint location, GLsizei count, const GLint *value) \
	FUNC(void,	glUniformMatrix2fv, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) \
	FUNC(void,	glUniformMatrix3fv, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) \
	FUNC(void,	glUniformMatrix4fv, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) \
	FUNC(void,	glDrawArrays, GLenum mode, GLint first, GLsizei count) \
	FUNC(void,	glDrawElements, GLenum mode, GLsizei count, GLenum type, const void *indices)

// Generate function declarations
#define FUNC(returntype, name, ...) typedef returntype name##Func ( __VA_ARGS__ );
GL_FUNCTIONS_BASE
GL_FUNCTIONS_ALL
#undef FUNC

struct OpenGLFunctions
{
	// Generate function pointers
#define FUNC(returntype, name, ...) name##Func* name;
	GL_FUNCTIONS_BASE
	GL_FUNCTIONS_ALL
#undef FUNC
};