#pragma once

#include <sys/time.h>

uint64_t getTimestamp() {
    struct timeval tv;

    gettimeofday(&tv, NULL);

    unsigned long long t =
        (unsigned long long)(tv.tv_sec) * 1000 +
        (unsigned long long)(tv.tv_usec) / 1000;

    return t;
}

// https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
bool rayIntersectsTriangle(Map map, Vector rayOrigin, Vector rayVector, Face* inTriangle, Vector& outIntersectionPoint) {
    const float EPSILON = 0.0000001;
    Vector vertex0 = MAP_SCALE * map.model.vertices[inTriangle->vertices[0]];
    Vector vertex1 = MAP_SCALE * map.model.vertices[inTriangle->vertices[1]];
    Vector vertex2 = MAP_SCALE * map.model.vertices[inTriangle->vertices[2]];
    Vector edge1, edge2, h, s, q;
    float a,f,u,v;
    edge1 = vertex1 - vertex0;
    edge2 = vertex2 - vertex0;
    h = rayVector.cross(edge2);
    a = edge1.dot(h);
    if (a > -EPSILON && a < EPSILON)
        return false;
    f = 1/a;
    s = rayOrigin - vertex0;
    u = f * (s.dot(h));
    if (u < 0.0 || u > 1.0)
        return false;
    q = s.cross(edge1);
    v = f * rayVector.dot(q);
    if (v < 0.0 || u + v > 1.0)
        return false;
    // At this stage we can compute t to find out where the intersection point is on the line.
    float t = f * edge2.dot(q);
    if (t > EPSILON) { // ray intersection
        outIntersectionPoint = rayOrigin + rayVector * t;
        return true;
    }
    else // This means that there is a line intersection but not a ray intersection.
        return false;
}

// Sphere-Triangle collision from: http://realtimecollisiondetection.net/blog/?p=103
bool sphereCollidesTriangle(Vector sphere_center, float sphere_radius, Vector triangle0, Vector triangle1, Vector triangle2) {
    Vector A = triangle0 - sphere_center;
    Vector B = triangle1 - sphere_center;
    Vector C = triangle2 - sphere_center;
    float rr = sphere_radius * sphere_radius;
    Vector V = (B - A).cross(C - A);
    float d = A.dot(V);
    float e = V.dot(V);

    bool sep1 = d*d > rr*e;

    float aa = A.dot(A);
    float ab = A.dot(B);
    float ac = A.dot(C);
    float bb = B.dot(B);
    float bc = B.dot(C);
    float cc = C.dot(C);

    bool sep2 = (aa > rr) && (ab > aa) && (ac > aa);
    bool sep3 = (bb > rr) && (ab > bb) && (bc > bb);
    bool sep4 = (cc > rr) && (ac > cc) && (bc > cc);

    Vector AB = B - A;
    Vector BC = C - B;
    Vector CA = A - C;

    float d1 = ab - aa;
    float d2 = bc - bb;
    float d3 = ac - cc;

    float e1 = AB.dot(AB);
    float e2 = BC.dot(BC);
    float e3 = CA.dot(CA);

    Vector Q1 = A*e1 - d1*AB;
    Vector Q2 = B*e2 - d2*BC;
    Vector Q3 = C*e3 - d3*CA;
    Vector QC = C*e1 - Q1;
    Vector QA = A*e2 - Q2;
    Vector QB = B*e3 - Q3;

    bool sep5 = (Q1.dot(Q1) > rr * e1 * e1) && (Q1.dot(QC) > 0);
    bool sep6 = (Q2.dot(Q2) > rr * e2 * e2) && (Q2.dot(QA) > 0);
    bool sep7 = (Q3.dot(Q3) > rr * e3 * e3) && (Q3.dot(QB) > 0);

    bool separated = sep1 || sep2 || sep3 || sep4 || sep5 || sep6 || sep7;

    return !separated;
}

// https://www.gamedev.net/forums/topic/566295-normal-to-a-quaternion/
Quat getRotationQuat(const Vector& from, const Vector& to) {
     Quat result;
     Vector H = from + to;
     H.normalize();

     result.w = from.dot(H);
     result.x = from.y*H.z - from.z*H.y;
     result.y = from.z*H.x - from.x*H.z;
     result.z = from.x*H.y - from.y*H.x;
     return result;
}

// https://gamedev.stackexchange.com/questions/23743/whats-the-most-efficient-way-to-find-barycentric-coordinates
// Compute barycentric coordinates (u, v, w) for
// point p with respect to triangle (a, b, c)
inline void barycentric(Vector p, Vector a, Vector b, Vector c, float &u, float &v, float &w)
{
    Vector v0 = b - a, v1 = c - a, v2 = p - a;
    float d00 = v0.dot(v0);
    float d01 = v0.dot(v1);
    float d11 = v1.dot(v1);
    float d20 = v2.dot(v0);
    float d21 = v2.dot(v1);
    float denom = d00 * d11 - d01 * d01;
    v = (d11 * d20 - d01 * d21) / denom;
    w = (d00 * d21 - d01 * d20) / denom;
    u = 1.0f - v - w;
}

// returns true if collides
// if returns false, paint_face and paint_pos have no meaning
bool projectileCollidesWithMap(Map map, Projectile projectile, Vector& paint_pos, int& paint_face) {
    bool intersects = false;

    float min_intersection_mag = 100000000.0f;

    for (int i = 0; i < map.model.num_faces; i++) {
        Face *cur = &map.model.faces[i];

        Vector vertex0 = MAP_SCALE * map.model.vertices[cur->vertices[0]];
        Vector vertex1 = MAP_SCALE * map.model.vertices[cur->vertices[1]];
        Vector vertex2 = MAP_SCALE * map.model.vertices[cur->vertices[2]];

        Vector v1 = vertex2 - vertex0;
        Vector v2 = vertex1 - vertex0;
        Vector normal = v1.cross(v2);
        normal.normalize();
        if (normal.z < 0) {
            normal = normal * -1;
        }

        Vector dir = projectile.velocity + Vector(0, 0, -GRAVITY);
        Vector intersection;
        bool intersect = rayIntersectsTriangle(map, projectile.pos, dir,
                                               cur, intersection);
        if (intersect) {
            Vector tmp_v = intersection - projectile.pos;
            float mag = tmp_v.len();
            if (mag < dir.len()) {
                intersects = true;

                if (mag < min_intersection_mag) {
                    paint_pos = intersection;
                    paint_face = i;
                    min_intersection_mag = mag;
                }
            }
        }
    }

    return intersects;
}

void collidesWithMap(Map map, Character& player, Vector& normal_sum, Vector& max_z, Vector& paint_max_z, int& paint_face) {
    Vector next_pos = player.pos + player.dir;
    Vector rotation_points[4] = {{-200, -200, -200}, {-200, -200, -200},
                                 {-200, -200, -200}, {-200, -200, -200}};
    Vector rotation_normals[4] = {{-200, -200, -200}, {-200, -200, -200},
                                  {-200, -200, -200}, {-200, -200, -200}};

    for (int i = 0; i < map.model.num_faces; i++) {
        Face *cur = &map.model.faces[i];

        Vector vertex0 = MAP_SCALE * map.model.vertices[cur->vertices[0]];
        Vector vertex1 = MAP_SCALE * map.model.vertices[cur->vertices[1]];
        Vector vertex2 = MAP_SCALE * map.model.vertices[cur->vertices[2]];

        Vector v1 = vertex2 - vertex0;
        Vector v2 = vertex1 - vertex0;
        Vector normal = v1.cross(v2);
        normal.normalize();
        if (normal.z < 0) {
            normal = normal * -1;
        }

        Vector up = {0, 0, 1};

        float cosine = normal.dot(up);

        float angle = acos(cosine) * 180 / M_PI;

        // walls
        if (angle > 60 &&
                (vertex0.z > next_pos.z + 0.03 ||
                 vertex1.z > next_pos.z + 0.03 ||
                 vertex2.z > next_pos.z + 0.03)) {
            bool collides = sphereCollidesTriangle(next_pos,
                                                   player.hit_radius,
                                                   vertex0, vertex1, vertex2);

            if (collides) {
                Vector v = player.pos - vertex0;
                float d = v.dot(normal);
                Vector collision = d*normal;

                Vector reaction_v = player.dir + collision;
                player.dir.x = reaction_v.x * player.dir.x > 0 ? reaction_v.x : 0;
                player.dir.y = reaction_v.y * player.dir.y > 0 ? reaction_v.y : 0;
                player.dir.z = reaction_v.z * player.dir.z > 0 ? reaction_v.z : 0;
            }
        }

        // floors
        else {
            Vector sky[4];
            sky[0] = {player.pos.x + player.hit_radius, player.pos.y, 200};
            sky[1] = {player.pos.x - player.hit_radius, player.pos.y, 200};
            sky[2] = {player.pos.x, player.pos.y + player.hit_radius, 200};
            sky[3] = {player.pos.x, player.pos.y - player.hit_radius, 200};

            Vector ground = {0, 0, -1};

            Vector intersect_v;
            for (int j = 0; j < 4; j++) {
                bool intersect = rayIntersectsTriangle(map, sky[j], ground, cur, intersect_v);
                if (intersect) {
                    // only false if slime is out of bounds
                    if (rotation_points[j].z < intersect_v.z) {
                        rotation_points[j] = intersect_v;
                        rotation_normals[j] = normal;
                    }
                }
            }

            // DEBUG: this makes the slime paint the ground where it walks
            Vector sky_slime = {player.pos.x, player.pos.y, 200};
            bool intersect = rayIntersectsTriangle(map, sky_slime, ground,
                                                   cur, intersect_v);
            if (intersect) {
                if (max_z.z < intersect_v.z) { // dont move this out of collidesWithMap
                    max_z = intersect_v;
                }
                if (intersect_v.z > paint_max_z.z) {
                    paint_max_z = intersect_v;
                    paint_face = i;
                }
            }
        }
    }
    for (int j = 0; j < 4; j++) {
        normal_sum += rotation_normals[j];
    }
    normal_sum.normalize();
}

