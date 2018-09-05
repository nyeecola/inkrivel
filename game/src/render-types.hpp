#pragma once

#include <GL/gl.h>
#include "vector.hpp"

#if 0
class Vertex {
public:
    float x;
    float y;
    float z;
};
#endif

class Normal {
public:
    float x;
    float y;
    float z;
};

class TextureCoord {
public:
    float x;
    float y;
};

class Face {
public:
    int vertices[3];
    int normals[3];
    int texture_coords[3];
};

class Model {
public:
    Vector *vertices;
    int num_vertices;
    Face *faces;
    int num_faces;
    Normal *normals;
    int num_normals;
    TextureCoord *texture_coords;
    int num_texture_coords;
    GLuint texture_id;
};

enum FaceType {
    VERTEX_ONLY,
    VERTEX_NORMAL,
    VERTEX_TEXTURE,
    VERTEX_ALL
};

