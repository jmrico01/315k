#pragma once

#include "opengl.h"

#define GL_FALSE					0
#define GL_TRUE						1

#define GL_BYTE						0x1400
#define GL_UNSIGNED_BYTE			0x1401
#define GL_SHORT					0x1402
#define GL_UNSIGNED_SHORT			0x1403
#define GL_INT						0x1404
#define GL_UNSIGNED_INT				0x1405
#define GL_FLOAT					0x1406
#define GL_DOUBLE					0x140A

#define GL_LINES					0x0001
#define GL_LINE_LOOP				0x0002
#define GL_LINE_STRIP				0x0003
#define GL_TRIANGLES				0x0004

#define GL_DEPTH_BUFFER_BIT			0x00000100
#define GL_STENCIL_BUFFER_BIT		0x00000400
#define GL_COLOR_BUFFER_BIT			0x00004000

#define GL_VENDOR					0x1F00
#define GL_RENDERER					0x1F01
#define GL_VERSION					0x1F02
#define GL_EXTENSIONS				0x1F03
#define GL_SHADING_LANGUAGE_VERSION	0x8B8C

#define GL_FRAGMENT_SHADER			0x8B30
#define GL_VERTEX_SHADER			0x8B31
#define GL_COMPILE_STATUS			0x8B81
#define GL_LINK_STATUS				0x8B82
#define GL_INFO_LOG_LENGTH			0x8B84

#define GL_ARRAY_BUFFER				0x8892
#define GL_ELEMENT_ARRAY_BUFFER		0x8893
#define GL_STREAM_DRAW				0x88E0
#define GL_STREAM_READ				0x88E1
#define GL_STREAM_COPY				0x88E2
#define GL_STATIC_DRAW				0x88E4
#define GL_STATIC_READ				0x88E5
#define GL_STATIC_COPY				0x88E6
#define GL_DYNAMIC_DRAW				0x88E8
#define GL_DYNAMIC_READ				0x88E9
#define GL_DYNAMIC_COPY				0x88EA

#define GL_BLEND					0x0BE2
#define GL_CULL_FACE				0x0B44
#define GL_CULL_FACE_MODE			0x0B45
#define GL_FRONT_FACE				0x0B46
#define GL_DEPTH_TEST				0x0B71

#define GL_NEVER					0x0200
#define GL_LESS						0x0201
#define GL_EQUAL					0x0202
#define GL_LEQUAL					0x0203
#define GL_GREATER					0x0204
#define GL_NOTEQUAL					0x0205
#define GL_GEQUAL					0x0206
#define GL_ALWAYS					0x0207
#define GL_ZERO						0
#define GL_ONE						1
#define GL_SRC_COLOR				0x0300
#define GL_ONE_MINUS_SRC_COLOR		0x0301
#define GL_SRC_ALPHA				0x0302
#define GL_ONE_MINUS_SRC_ALPHA		0x0303
#define GL_DST_ALPHA				0x0304
#define GL_ONE_MINUS_DST_ALPHA		0x0305
#define GL_DST_COLOR				0x0306
#define GL_ONE_MINUS_DST_COLOR		0x0307

typedef void	GLvoid;

typedef bool	GLboolean;

typedef char	GLchar;
typedef int8	GLbyte;
typedef int16	GLshort;
typedef int32	GLint;
typedef int64	GLint64;

typedef uint8	GLubyte;
typedef uint16	GLushort;
typedef uint32	GLuint;
typedef uint64	GLuint64;

typedef uint32	GLenum;
typedef uint32	GLbitfield;

typedef float32	GLfloat;
typedef float32	GLclampf;
typedef float64	GLdouble;
typedef float64	GLclampd;

typedef uint32	GLsizei;
typedef size_t	GLsizeiptr;

// TODO maybe there's a better way of doing this...
#define FUNC(returntype, name, ...) extern name##Func* name;
    GL_FUNCTIONS_BASE
    GL_FUNCTIONS_ALL
#undef FUNC