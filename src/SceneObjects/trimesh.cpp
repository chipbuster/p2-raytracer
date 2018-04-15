#include "trimesh.h"
#include <assert.h>
#include <float.h>
#include <string.h>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <glm/gtx/norm.hpp>
#include <iostream>
#include "../mathutil.h"
#include "../ui/TraceUI.h"
extern TraceUI *traceUI;
extern bool debugMode;

std::ostream &operator<<(std::ostream &os, const glm::dvec3 x);

using std::cout;
using std::endl;

using namespace std;

Trimesh::~Trimesh()
{
    for (auto m : materials)
        delete m;
    for (auto f : faces)
        delete f;
}

// must add vertices, normals, and materials IN ORDER
void Trimesh::addVertex(const glm::dvec3 &v)
{
    vertices.emplace_back(v);
}

void Trimesh::addMaterial(Material *m)
{
    vertMats = true;
    materials.emplace_back(m);
}

void Trimesh::addNormal(const glm::dvec3 &n)
{
    normals.emplace_back(n);
}

// Returns false if the vertices a,b,c don't all exist
bool Trimesh::addFace(int a, int b, int c)
{
    int vcnt = vertices.size();

    if (a >= vcnt || b >= vcnt || c >= vcnt)
        return false;

    TrimeshFace *newFace = new TrimeshFace(scene, new Material(*this->material),
                                           this, a, b, c);
    newFace->setTransform(this->transform);
    if (!newFace->degen) {
        faces.push_back(newFace);
    } else {
        delete newFace;
    }

    // Don't add faces to the scene's object list so we can cull by bounding
    // box
    return true;
}

// Check to make sure that if we have per-vertex materials or normals
// they are the right number.
const char *Trimesh::doubleCheck()
{
    if (!materials.empty() && materials.size() != vertices.size())
        return "Bad Trimesh: Wrong number of materials.";
    if (!normals.empty() && normals.size() != vertices.size())
        return "Bad Trimesh: Wrong number of normals.";

    return 0;
}

bool Trimesh::intersectLocal(ray &r, isect &i) const
{
    bool have_one = false;
    for (auto face : faces) {
        isect cur;
        if (face->intersectLocal(r, cur)) {
            if (!have_one || (cur.getT() < i.getT())) {
                i = cur;
                have_one = true;
            }
        }
    }
    if (!have_one)
        i.setT(1000.0);
    return have_one;
}

bool TrimeshFace::intersect(ray &r, isect &i) const
{
    return intersectLocal(r, i);
}

std::vector<Geometry*> Trimesh::getGeomElements(){
    std::vector<Geometry*> rval;
    for(size_t i = 0; i < faces.size(); i++){
        rval.push_back(faces[i]);
    }
    return rval;
}

// Intersect ray r with the triangle abc.  If it hits returns true,
// and put the parameter in t and the barycentric coordinates of the
// intersection in u (alpha) and v (beta).

// Assumption: ray direction is normalized
bool TrimeshFace::intersectLocal(ray &r, isect &i) const
{
    /* Ignore transform data because TA said so
    TransformNode* trans = this->transform;
    glm::dvec3 example = trans->localToGlobalCoords(r.getPosition());
    */

    // Need to pick a point on face to determine whether the ray intersects
    // the plane of the triangle. Might as well make it one of the vertices
    glm::dvec3 pInTri = parent->vertices[ids[0]];
    glm::dvec3 o = r.getPosition();
    glm::dvec3 v = r.getDirection();
    glm::dvec3 n = this->getNormal();

    assert(abs(glm::l2Norm(v) - 1) < RAY_EPSILON && "Ray direction not normed");

#ifndef NDEBUG
    // Sanity check on calculated normals
    glm::dvec3 ab =
            glm::normalize(parent->vertices[ids[1]] - parent->vertices[ids[0]]);
    glm::dvec3 bc =
            glm::normalize(parent->vertices[ids[2]] - parent->vertices[ids[1]]);
    glm::dvec3 ac =
            glm::normalize(parent->vertices[ids[0]] - parent->vertices[ids[2]]);

    assert(abs(glm::dot(ab, n)) < HALF_DEGREE_EPSILON &&
           "Normal not perpendicular to side!");
    assert(abs(glm::dot(bc, n)) < HALF_DEGREE_EPSILON &&
           "Normal not perpendicular to side!");
    assert(abs(glm::dot(ac, n)) < HALF_DEGREE_EPSILON &&
           "Normal not perpendicular to side!");
#endif

    // If vTn is zero, we are hitting this plane edge-on and will not
    // intersect
    if (abs(glm::dot(v, n)) < HALF_DEGREE_EPSILON) {
        return false;
    }

    double t = glm::dot(pInTri - o, n) / glm::dot(v, n);

    // Intersection occurs behind the camera. Ignore.
    if (t < 0) {
        return false;
    }

    // Check to see if ray intersects with triangle or just plane of
    // triangle
    glm::dvec3 isectPtWorld = o + t * v;

    auto p0 = parent->vertices[ids[0]];
    auto p1 = parent->vertices[ids[1]];
    auto p2 = parent->vertices[ids[2]];
    auto p3 = isectPtWorld;

    bool inTriangle = true;

    for (int j = 0; j < 3; j++) {
        glm::dvec3 baseToPoint = isectPtWorld - parent->vertices[ids[j]];
        glm::dvec3 triangleSide =
                parent->vertices[ids[(j + 1) % 3]] - parent->vertices[ids[j]];

        double sig = glm::dot(n, glm::cross(triangleSide, baseToPoint));
        if (sig < 0) {
            inTriangle = false;
        }
    }
    if (!inTriangle) {
        return false;
    }

    // Intersection confirmed!  Add necessary properties to the intersection
    i.setT(t);
    i.setObject(this);

    // Find barycentric coordinates using explicit solver: formulas cribbed
    // from http://blackpawn.com/texts/pointinpoly/

    glm::dvec3 v0 = parent->vertices[ids[2]] - parent->vertices[ids[0]];
    glm::dvec3 v1 = parent->vertices[ids[1]] - parent->vertices[ids[0]];
    glm::dvec3 v2 = isectPtWorld - parent->vertices[ids[0]];

    double dot01 = glm::dot(v0, v1);
    double dot12 = glm::dot(v1, v2);
    double dot02 = glm::dot(v0, v2);
    double dot00 = glm::dot(v0, v0);
    double dot11 = glm::dot(v1, v1);
    double dot22 = glm::dot(v2, v2);
    double denom = dot00 * dot11 - dot01 * dot01;

    double ubary = (dot11 * dot02 - dot01 * dot12) / denom;
    double vbary = (dot00 * dot12 - dot01 * dot02) / denom;

    double alpha = 1 - ubary - vbary;
    double beta = vbary;
    double gamma = ubary;

    i.setBary(alpha, beta, gamma);

#ifndef NDEBUG
    glm::dvec3 reconstruct = alpha * parent->vertices[ids[0]] +
                             beta * parent->vertices[ids[1]] +
                             gamma * parent->vertices[ids[2]];

    assert(alpha > 0 && beta > 0 && gamma > 0);
    assert(glm::l2Norm(reconstruct - isectPtWorld) < RAY_EPSILON &&
           "Your barycentric coordinates don't yield the right results!");
#endif

    // If vertex normals are defined, use Phong normals. Else use flat.
    if (parent->vertNorms) {
        glm::dvec3 interpNormal = alpha * parent->normals[ids[0]] +
                                  beta * parent->normals[ids[1]] +
                                  gamma * parent->normals[ids[2]];

        i.setN(glm::normalize(interpNormal));
    } else {
        i.setN(this->normal);
    }

    // If vertex materials are defined, interpolate. Otherwise use material
    if (parent->vertMats) {
        Material mat;

        mat += alpha * *(parent->materials[ids[0]]);
        mat += beta * *(parent->materials[ids[1]]);
        mat += gamma * *(parent->materials[ids[2]]);

        i.setMaterial(mat);
    } else {
        i.setMaterial(parent->getMaterial());
    }

    return true;
}

// Once all the verts and faces are loaded, per vertex normals can be
// generated by averaging the normals of the neighboring faces.
void Trimesh::generateNormals()
{
    int cnt = vertices.size();
    normals.resize(cnt);
    std::vector<int> numFaces(cnt, 0);

    for (auto face : faces) {
        glm::dvec3 faceNormal = face->getNormal();

        for (int i = 0; i < 3; ++i) {
            normals[(*face)[i]] += faceNormal;
            ++numFaces[(*face)[i]];
        }
    }

    for (int i = 0; i < cnt; ++i) {
        if (numFaces[i])
            normals[i] /= numFaces[i];
    }

    vertNorms = true;
}
