#pragma once
#include "IVec2.h"

/// <summary>
/// Simple container that we can pass to the player
/// </summary>
struct GameRules
{
   int world_width;
   int world_height;
   int world_size_total;
   int cell_types_used;
};
