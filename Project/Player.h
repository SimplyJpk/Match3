#pragma once
#include "GameObject.h"
#include "Match3.h"

class Player : public GameObject
{
public:
   void NewGame(Match3* match3);

   void Update(double delta) override;

   void GetValidMove();
   void MakeMove();

private:
   bool is_ready_ = false;

   IVec2 next_move_[2];
   Match3* match3_;
};
