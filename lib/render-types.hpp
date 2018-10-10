#pragma once

#include <GL/gl.h>
#include <SDL2/SDL.h>
#include "vector.hpp"
#include "config.hpp"

class Quat {
    public:
        float w;
        float x;
        float y;
        float z;
};

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
        SDL_Surface *texture_image;
};

enum FaceType {
    VERTEX_ONLY,
    VERTEX_NORMAL,
    VERTEX_TEXTURE,
    VERTEX_ALL
};

class Models {
    public:
        Model green_character[AVAILABLE_CHARACTERS];
        Model pink_character[AVAILABLE_CHARACTERS];
        Model map;
};
