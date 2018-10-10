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

        void getPaintResults(float percentages[]) {
            int text_h;
            int text_w;

            glBindTexture(GL_TEXTURE_2D, this->map.texture_id);

            glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &text_w);
            glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &text_h);

            GLuint texture[text_h*text_w];

            glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, texture);

            glBindTexture(GL_TEXTURE_2D, 0);

            long green_pixels = 0;
            for (int i = 0; i < text_h*text_w; i++) {
                if (texture[i] == 0x1FFF1FFF) green_pixels++;
            }

            percentages[0] = 100*((float) green_pixels)/(text_h*text_w);
            percentages[1] = 100-percentages[0];
        }
};
