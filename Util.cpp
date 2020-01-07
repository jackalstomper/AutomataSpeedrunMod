#include "Util.hpp"
#include <algorithm>

namespace AutomataMod {

Vector3f::Vector3f(float x, float y, float z) : x(x), y(y), z(z) {}

Volume::Volume(Vector3f root, float widthX, float heightY, float lengthZ) {
    Vector3f p2(root.x + widthX, root.y + heightY, root.z + lengthZ);
    minX = std::min(root.x, p2.x);
    maxX = std::max(root.x, p2.x);
    minY = std::min(root.y, p2.y);
    maxY = std::max(root.y, p2.y);
    minZ = std::min(root.z, p2.z);
    maxZ = std::max(root.z, p2.z);
}

bool Volume::contains(const Vector3f& p) const {
    return (p.z >= minZ && p.z <= maxZ &&
        p.y >= minY && p.y <= maxY &&
        p.x >= minX && p.x <= maxX);
}

} // namespace AutomataMod
