module;

#include <algorithm>

export module Util;

namespace AutomataMod {

// positve X = move left
// positive Y = move up
// positive Z = move forward
export struct Vector3f {
    float x;
    float y;
    float z;

    Vector3f(float x, float y, float z) : x(x), y(y), z(z) {}
};

export struct Volume {
    float minX, maxX;
    float minY, maxY;
    float minZ, maxZ;

    Volume(Vector3f root, float widthX, float heightY, float lengthZ) {
        Vector3f p2(root.x + widthX, root.y + heightY, root.z + lengthZ);
        minX = std::min(root.x, p2.x);
        maxX = std::max(root.x, p2.x);
        minY = std::min(root.y, p2.y);
        maxY = std::max(root.y, p2.y);
        minZ = std::min(root.z, p2.z);
        maxZ = std::max(root.z, p2.z);
    }

    // Returns true if the given point is inside this volume
    bool contains(const Vector3f& p) const {
        return (p.z >= minZ && p.z <= maxZ &&
            p.y >= minY && p.y <= maxY &&
            p.x >= minX && p.x <= maxX);
    }
};

} // namespace AutomataMod