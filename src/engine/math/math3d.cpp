//
// Created by mgrus on 20.12.2023.
//

#include "math3d.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

Vec3 vAdd(Vec3 a, Vec3 b) {
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}

Vec3 vSub(Vec3 a, Vec3 b) {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

float vDot(Vec3 a, Vec3 b) {
    return {a.x * b.x + a.y * b.y+ a.z * b.z};
}

bool testVectorMath() {
    bool testsOK = true;

    Vec3 a = {0, 0, 0};
    Vec3 b = {1, 2, 3};
    auto c = vAdd(a, b);
    testsOK = vEq(c, {1, 2, 3});

    c = vSub(a, b);
    testsOK = vEq(c, {-1, -2, -3});

    float d = vDot(a, b);
    testsOK = d == 0;

    Vec4 a4 = {0, 0, 0, 0};
    Vec4 b4 = {1, 2, 3, 4};
    testsOK = vDot(a4, b4) == 0;



    return testsOK;

}

bool vEq(Vec3 a, Vec3 b) {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

float vDot(Vec4 a, Vec4 b) {
    return {a.x * b.x + a.y * b.y+ a.z * b.z + a.w * b.w};
}

Mat4 matMul(Mat4 a, Mat4 b) {
    Mat4 c;
    float c11 = vDot(a.row1(), b.col1);
    float c12 = vDot(a.row1(), b.col2);
    float c13 = vDot(a.row1(), b.col3);
    float c14 = vDot(a.row1(), b.col4);

    float c21 = vDot(a.row2(), b.col1);
    float c22 = vDot(a.row2(), b.col2);
    float c23 = vDot(a.row2(), b.col3);
    float c24 = vDot(a.row2(), b.col4);

    float c31 = vDot(a.row3(), b.col1);
    float c32 = vDot(a.row3(), b.col2);
    float c33 = vDot(a.row3(), b.col3);
    float c34 = vDot(a.row3(), b.col4);

    float c41 = vDot(a.row4(), b.col1);
    float c42 = vDot(a.row4(), b.col2);
    float c43 = vDot(a.row4(), b.col3);
    float c44 = vDot(a.row4(), b.col4);

    c.col1 = {c11, c21, c31, c41};
    c.col2 = {c12, c22, c32, c42};
    c.col3 = {c13, c23, c33, c43};
    c.col4 = {c14, c24, c34, c44};

    return c;
}

bool testMatrixMath() {
    bool testsOK = true;
    Mat4 a = {{1, 0, 0, 0},
              {0, 1, 0, 0},
              {0, 0, 1, 0},
              {0, 0, 0, 1}};
    Mat4 b = {{3,  4,  5,  6},
              {7,  8,  9,  10},
              {11, 12, 13, 14},
              {15, 16, 17, 18}};

    Mat4 c = matMul(a, b);
    testsOK = vEq(b.col1, c.col1);
    testsOK = vEq(b.col2, c.col2);
    testsOK = vEq(b.col3, c.col3);
    testsOK = vEq(b.col4, c.col4);

    Vec4 v = {1, 2, 3, 4};
    Vec4 vr = matVMul(a, v);
    testsOK = vEq(vr, v);

    return testsOK;
}

bool vEq(Vec4 a, Vec4 b ) {
    return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

Mat4 translationMatrix(Vec3 position) {
    return  {{1, 0, 0,0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {position.x, position.y, position.z,1}};
}

Mat4 scaleMatrix(Vec3 scale) {
    return {{scale.x, 0, 0, 0}, {0, scale.y, 0, 0}, {0, 0, scale.z, 0}, {0, 0, 0, 1}};
}

Vec4 matVMul(Mat4 m, Vec4 v) {
    return {vDot(m.row1(), v), vDot(m.row2(), v), vDot(m.row3(), v), vDot(m.row4(), v)};
}

Mat4 orthoProjection(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
    return Mat4 {{2.0f/(right-left), 0, 0, 0}, {0, 2.0f/(top-bottom), 0, 0}, {0, 0, -2.0f/(farPlane-nearPlane), 0}, {-((left+right)/(right-left)), -((bottom+top)/(top-bottom)), -((nearPlane+farPlane)/(farPlane-nearPlane)), 1}};
}

Mat4 perspectiveProjection(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
    return Mat4{{(2.0f*nearPlane)/(right - left), 0, 0, 0}, {0, (2.0f*nearPlane)/(top-bottom), 0, 0}, {(right+left)/(right-left), (top+bottom)/(top-bottom), -((farPlane + nearPlane)/(farPlane - nearPlane)), -1}, {0,  0, -((2.0f*farPlane*nearPlane)/(farPlane-nearPlane)), 0}};
}

Vec4 Mat4::row1() {
    return {col1.x, col2.x, col3.x, col4.x};
}
Vec4 Mat4::row2() {
    return {col1.y, col2.y, col3.y, col4.y};
}
Vec4 Mat4::row3() {
    return {col1.z, col2.z, col3.z, col4.z};
}
Vec4 Mat4::row4() {
    return {col1.w, col2.w, col3.w, col4.w};
}

void AABB::scaleBy(double scale) {
    minX *= scale;
    minY *= scale;
    minZ *= scale;
    maxX *= scale;
    maxY *= scale;
    maxZ *= scale;
}

// Intersects ray r = p + td, |d| = 1, with sphere s and, if intersecting,
// returns t value of intersection and intersection point q
bool rayIntersectsSphere(Ray ray, float radius, glm::vec3 sphereCenter, float &t, glm::vec3 &q)
{
    auto point = ray.origin;
    auto d = ray.direction;
    glm::vec3 m = point - sphereCenter;
    float b = glm::dot(m, d);
    float c = glm::dot(m, m) - radius * radius;

    // Exit if r’s origin outside s (c > 0) and r pointing away from s (b > 0)
    if (c > 0.0f && b > 0.0f) return false;
    float discriminate = b * b - c;

    // A negative discriminant corresponds to ray missing sphere
    if (discriminate < 0.0f) return false;

    // Ray now found to intersect sphere, compute smallest t value of intersection
    t = -b - glm::sqrt(discriminate);

    // If t is negative, ray started inside sphere so clamp t to zero
    if (t < 0.0f) t = 0.0f;
    q = point + t * d;

    return true;
}

// Function to calculate rotation quaternion with edge case handling
glm::quat calculateRotation(const glm::vec3& start, const glm::vec3& dest) {
    glm::vec3 normalizedStart = glm::normalize(start);
    glm::vec3 normalizedDest = glm::normalize(dest);

    float dotProduct = glm::dot(normalizedStart, normalizedDest);

    // If vectors are nearly identical, return identity quaternion
    if (dotProduct > 0.9999f) {
        return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    }

    // If vectors are nearly opposite, find orthogonal rotation axis
    if (dotProduct < -0.9999f) {
        glm::vec3 orthogonal = glm::vec3(1.0f, 0.0f, 0.0f);
        if (glm::abs(glm::dot(normalizedStart, orthogonal)) > 0.999f) {
            orthogonal = glm::vec3(0.0f, 1.0f, 0.0f);
        }
        glm::vec3 rotationAxis = glm::normalize(glm::cross(normalizedStart, orthogonal));
        return glm::angleAxis(glm::radians(180.0f), rotationAxis);
    }

    // General case
    return glm::rotation(normalizedStart, normalizedDest);
}


AABBCollisionResult aabbsCollidingPerAxis(const AABB& a, const AABB& b) {
    AABBCollisionResult result;
    result.xCollision = (a.maxX >= b.minX) && (a.minX <= b.maxX);
    result.yCollision = (a.maxY >= b.minY) && (a.minY <= b.maxY);
    result.zCollision = (a.maxZ <= b.minZ) && (a.minZ >= b.maxZ);
    return result;
}

bool aabbsColliding(const AABB& a, const AABB& b) {
    bool overlapX = (a.maxX >= b.minX) && (a.minX <= b.maxX);
    bool overlapY = (a.maxY >= b.minY) && (a.minY <= b.maxY);
    bool overlapZ = (a.maxZ <= b.minZ) && (a.minZ >= b.maxZ);

    return overlapX && overlapY && overlapZ;
}

bool pointInTri(glm::vec3 point, glm::vec3 A, glm::vec3 B, glm::vec3 C) {
    auto v0 = C - A;
    auto v1 = B - A;
    auto v2 = point - A;

    float dot00 = glm::dot(v0, v0);
    float dot01 = glm::dot(v0, v1);
    float dot02 = glm::dot(v0, v2);
    float dot11 = glm::dot(v1, v1);
    float dot12 = glm::dot(v1, v2);

    float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
    float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

    // Check if point is in the triangle
    return (u >= 0) && (v >= 0) && (u + v <= 1);
}
