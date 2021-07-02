#include "Player.h"

#include "InputManager.h"

/// <summary>
/// </summary>
void Player::NewGame(Match3* match3)
{
   match3_ = match3;
}

void Player::Update(double delta)
{
      if (InputManager::Instance()->GetKeyDown(KeyCode::A))
         MakeMove();
}

void Player::GetValidMove()
{
   if (match3_->AnyLegalMatchesExist(next_move_))
   {
      is_ready_ = true;
   }
   else
   {
      is_ready_ = false;
   }
}

void Player::MakeMove()
{
   if (!match3_->IsReadyForMove())
      return;

   GetValidMove();

   if (is_ready_) {
      match3_->Step(next_move_[0], next_move_[1]);
      is_ready_ = false;
   }
}
