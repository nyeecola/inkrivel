#pragma once
#include <math.h>

// Vector code modified from: http://www.flipcode.com/archives/Moving_Sphere_VS_Triangle_Collision.shtml
class Vector {
    public:

        inline Vector() {}

        inline Vector(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

        inline Vector operator - () const {
            return Vector(-x, -y, -z);
        }

        inline void operator -= (const Vector &_v) {
            x -= _v.x;
            y -= _v.y;
            z -= _v.z;
        }

        inline void operator += (const Vector &_v) {
            x += _v.x;
            y += _v.y;
            z += _v.z;
        }

        inline void operator *= (float _mul) {
            x *= _mul;
            y *= _mul;
            z *= _mul;
        }

        inline void operator *= (const Vector &_v) {
            x *= _v.x;
            y *= _v.y;
            z *= _v.z;
        }

        inline void operator /= (float _div) {
            float mul = 1.0f / _div;
            x *= mul;
            y *= mul;
            z *= mul;
        }

        inline Vector operator - (const Vector &_v) const {
            return Vector(x - _v.x, y - _v.y, z - _v.z);
        }

        inline Vector operator + (const Vector &_v) const {
            return Vector(x + _v.x, y + _v.y, z + _v.z);
        }

        inline Vector operator * (const Vector &_v) const {
            return Vector(x * _v.x, y * _v.y, z * _v.z);
        }

        inline Vector operator * (float _m) const {
            return Vector(x * _m, y * _m, z * _m);
        }

        inline Vector operator / (const Vector &_v) const {
            return Vector(x / _v.x, y / _v.y, z / _v.z);
        }

        inline Vector operator / (float _d) const {
            float m = 1.0f / _d;
            return Vector(x * m, y * m, z * m);
        }

        inline bool operator == (const Vector &_v) const {
            if (x == _v.x && y == _v.y && z == _v.z)
                return true;
            return false;
        }

        inline bool operator != (const Vector &_v) const {
            if (x != _v.x || y != _v.y || z != _v.z)
                return true;
            return false;
        }

        inline float operator [] (int _i) const {
            const float *val = &x;
            return val[_i];
        }

        inline float len() const {
            float len = x * x + y * y + z * z;
            return (float) sqrt(len);
        }

        inline float lenSq() const {
            return x * x + y * y + z * z;
        }

        inline float dot(const Vector &_v) const {
            return x * _v.x + y * _v.y + z * _v.z;
        }

        inline Vector cross(const Vector &_v) const {
            Vector n = {y*_v.z - z*_v.y,
                        z*_v.x - x*_v.z,
                        x*_v.y - y*_v.x};
            return n;
        }

        inline void normalize() {
            float ln = len();
            if (!ln)
                return;
            float div = 1.0f / ln;
            x *= div;
            y *= div;
            z *= div;
        }

        inline void positive() {
            if (x < 0) x = -x;
            if (y < 0) y = -y;
            if (z < 0) z = -z;
        }

        float x, y, z;
};

inline Vector operator * (float _m, const Vector &_v) {
    return Vector(_v.x * _m, _v.y * _m, _v.z * _m);
}
