#pragma once

namespace AutomataMod {

// positve X = move left
// positive Y = move up
// positive Z = move forward
struct Vector3f {
    float x;
    float y;
    float z;

    Vector3f(float x, float y, float z);
};

struct Volume {
    float minX, maxX;
    float minY, maxY;
    float minZ, maxZ;

    Volume(Vector3f root, float widthX, float heightY, float lengthZ);

    // Returns true if the given point is inside this volume
    bool contains(const Vector3f& p) const;
};

} // namespace AutomataMod