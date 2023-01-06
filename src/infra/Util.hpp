#pragma once

#include <algorithm>
#include <infra/defs.hpp>

namespace AutomataMod {

// positve X = move left
// positive Y = move up
// positive Z = move forward
struct Vector3f {
	f32 x;
	f32 y;
	f32 z;

	Vector3f(f32 x, f32 y, f32 z);
};

struct Volume {
	f32 minX, maxX;
	f32 minY, maxY;
	f32 minZ, maxZ;

	Volume(Vector3f root, f32 widthX, f32 heightY, f32 lengthZ);

	// Returns true if the given point is inside this volume
	bool contains(const Vector3f &p) const;
};

} // namespace AutomataMod
