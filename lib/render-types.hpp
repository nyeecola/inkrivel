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
        GLuint texture_bw_id;
        SDL_Surface *texture_bw_image;
};

enum FaceType {
    VERTEX_ONLY,
    VERTEX_NORMAL,
    VERTEX_TEXTURE,
    VERTEX_ALL
};

void getPaintResults(Model map, float percentages[3]) {
    int w = map.texture_image->w;
    int h = map.texture_image->h;
    int size = w * h;

    uint32_t *pixels = (uint32_t *) map.texture_image->pixels;

    percentages[0] = percentages[1] = 0;
    for (int i = 0; i < size; i++) {
        uint32_t pixel = pixels[i];
        if (pixel == 0xFF1FFF1F) percentages[0]++;
        else if (pixel == 0xFFFF1FFF) percentages[1]++;
    }

    percentages[0] = 100*percentages[0]/size;
    percentages[1] = 100*percentages[1]/size;
    percentages[2] = 100-percentages[0]-percentages[1];
}

class Models {
    public:
        Model green_character[AVAILABLE_CHARACTERS+1];
        Model pink_character[AVAILABLE_CHARACTERS+1];
        Model map;
};
