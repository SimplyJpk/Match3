#pragma once
#include "IVec2.h"

enum CellTypes
{
   EMPTY = 0,
   RED = 1,
   GREEN = 2,
   BLUE = 3,
   YELLOW = 4,
   WHITE = 5,
   CELL_TYPE_COUNT
};

class CellData
{
public:
   float Move_Cooldown_Time = 0.5f;
   CellTypes CellType = EMPTY;
   float MoveCooldown = 0.0f;

   void CoolDown() { MoveCooldown += Move_Cooldown_Time; }

   CellData(const CellTypes type)
   {
      CellType = type;
   }
   CellData() = default;
};

