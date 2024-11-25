//
// Created by mgrus on 20.12.2023.
//

#ifndef SIMPLE_KING_MATH3D_H
#define SIMPLE_KING_MATH3D_H

struct Vec2 {
    float x;
    float y;

};

struct Vec3 {
    float x;
    float y;
    float z;
};

struct Vec4 {
    float x;
    float y;
    float z;
    float w;
};

struct Mat4 {
    Vec4 col1;
    Vec4 col2;
    Vec4 col3;
    Vec4 col4;

    // Helper functions to simplify dot products
    Vec4 row1();
    Vec4 row2();
    Vec4 row3();
    Vec4 row4();
};

struct AABB {
    float minX, minY, minZ; // Minimum corner
    float maxX, maxY, maxZ; // Maximum corner
    void scaleBy(double x);

};

Vec3 vAdd(Vec3 a, Vec3 b);
Vec3 vSub(Vec3 a, Vec3 b);
Vec4 matVMul(Mat4 m, Vec4 v);
float vDot(Vec3 a, Vec3 b);
float vDot(Vec4 a, Vec4 b);
bool vEq(Vec3 a, Vec3 b);
bool vEq(Vec4 a, Vec4 b);

Mat4 matMul(Mat4 a, Mat4 b);
Mat4 translationMatrix(Vec3 position);
Mat4 scaleMatrix(Vec3 scale);
Mat4 orthoProjection(float left, float right, float bottom, float top, float near, float farPlane);
Mat4 perspectiveProjection(float left, float right, float bottom, float top, float near, float farPlane);

bool testVectorMath();
bool testMatrixMath();

class Ray;
// Intersects ray r = p + td, |d| = 1, with sphere s and, if intersecting,
// returns t value of intersection and intersection point q
bool rayIntersectsSphere(Ray ray, float radius, glm::vec3 sphereCenter, float &t, glm::vec3 &q);
bool rayIntersectsPlane(Ray ray, glm::vec3 planeNormal, glm::vec3 planePoint, glm::vec3* intersectionPoint);
bool pointInTri(glm::vec3 point, glm::vec3 A, glm::vec3 B, glm::vec3 C);

glm::quat CalculateRotation(const glm::vec3& start, const glm::vec3& dest);

AABB getMovedABBByLocation(AABB sourceAABB, glm::vec3 location) {
    AABB updatedAABB;
    updatedAABB.minX = location.x + sourceAABB.minX;
    updatedAABB.minY = location.y + sourceAABB.minY;
    updatedAABB.minZ = location.z + sourceAABB.minZ;
    updatedAABB.maxX = location.x + sourceAABB.maxX;
    updatedAABB.maxY = location.y + sourceAABB.maxY;
    updatedAABB.maxZ = location.z + sourceAABB.maxZ;
    return updatedAABB;
}

struct AABBCollisionResult {
    bool xCollision = false;
    bool yCollision = false;
    bool zCollision = false;
};

bool aabbsColliding(const AABB& a, const AABB& b);
AABBCollisionResult aabbsCollidingPerAxis(const AABB& a, const AABB& b);


struct Wall {
    glm::vec3 normal;
    glm::vec3 location;
    glm::vec3 rotation;
    float halfWidth = 0.5;

};


#endif //SIMPLE_KING_MATH3D_H
