#pragma once

#include "WalkMesh.hpp"

/**
Identify a position in the universe by
  pointer to a WalkMesh
  a WalkPoint within that Walkmesh
*/

struct MeshCoord {
  WalkMesh* walkmesh;
  WalkPoint walkpoint;
};
