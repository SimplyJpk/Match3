#pragma once
#include "IVec2.h"

// Simple class that just acts as a container for information we want to show in ImGUI.
// Pretty bad way to organise this information, but it is easier just to demonstrate some more information.

class ExtraInfoGUI
{
public:
   IVec2 last_cell_moved_from = IVec2(-1, -1);
   IVec2 last_cell_moved_to = IVec2(-1, -1);

   int game_high_score = 0;
   int game_score = 0;
   int moves_since_last_reset = 0;

   bool next_frame_restarts = false;
   float world_step_cooldown = 0.0f;

   void AddPoint()
   {
      game_score++;
   }

   void ClearMovedCells()
   {
      last_cell_moved_from = IVec2(-1, -1);
      last_cell_moved_to = IVec2(-1, -1);
   }

   void Clear()
   {
      // Highscore
      if (game_high_score < game_score)
         game_high_score = game_score;

      // Resets
      game_score = 0;
      moves_since_last_reset = 0;
      ClearMovedCells();
      next_frame_restarts = false;
      world_step_cooldown = 0.0f;
   }
};
