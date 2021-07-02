#include "Match3.h"
#include "FloatExtensions.h"
#include "InputManager.h"

Match3::Match3(GameSettings* settings)
{
   game_settings = settings;

   srand(time(0));

   coloured_textures_ = new GLuint[CELL_TYPE_COUNT];

   // Textures for our blocks.
   glGenTextures(CELL_TYPE_COUNT, coloured_textures_);
   for (int i = 0; i < CELL_TYPE_COUNT; i++)
   {
      glBindTexture(GL_TEXTURE_2D, coloured_textures_[i]);

      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, cell_texture_size, cell_texture_size, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8,
                   &g_cell_colours[i]);
   }

   glUseProgram(game_settings->default_shader);
   glUniform1i(glGetUniformLocation(game_settings->default_shader, "ourTexture"), 0);

   glGenVertexArrays(1, &vao_);
   glGenBuffers(1, &vbo_);
   glGenBuffers(1, &ebo_);

   glBindVertexArray(vao_);

   glBindBuffer(GL_ARRAY_BUFFER, vbo_);
   glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

   // position attribute
   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
   glEnableVertexAttribArray(0);
   // texture coord attribute
   glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
   glEnableVertexAttribArray(1);
}

const GameRules* Match3::GetRules() const
{
   return &game_rules_;
}

void Match3::PrintWorldAsText() const
{
   printf("-----\n");
   if (game_rules_.world_size_total != 0)
   {
      for (int y = 0; y < game_rules_.world_height; y++) 
      {
         for (int x = 0; x < game_rules_.world_width; x++)
         {
            printf("%i ", world_data_[GetCellIndex(x, y)]);
         }
         printf("\n");
      }
   }
}

bool Match3::GeneratePlayField(Uint32 width, Uint32 height, Uint32 numTypes)
{
   // Reset GUI Info
   g_extraInfo.Clear();

   if (numTypes > CELL_TYPE_COUNT)
      numTypes = CELL_TYPE_COUNT;

   // Game Stuff
   game_rules_.world_width = width;
   game_rules_.world_height = height;
   game_rules_.world_size_total = width * height;
   game_rules_.cell_types_used = numTypes;

   if (world_data_ == nullptr) {
      world_data_ = new int[game_rules_.world_size_total]{0};
   }

   // Clear all tiles
   no_valid_moves_ = false;
   SetWorldCells(RANDOM);

   return true;
}

bool Match3::IsReadyForMove() const
{
   return is_ready_for_move_;
}

bool Match3::Step(const IVec2 from_cell, const IVec2 to_cell)
{
   world_update_cooldown_x_ = world_update_rate_ * 2.0f;
   g_extraInfo.ClearMovedCells();
   // Check cells being passed in are valid cells
   if (!(IsValidCell(from_cell.x, from_cell.y) || IsValidCell(to_cell.x, to_cell.y)))
      return false;

   // We only want to move 1 square, anything other than 1 should be an invalid move
   if (!FloatsEqual(IVec2::Distance(from_cell, to_cell), 1.0f))
      return false;

   // Dirty check for matches
   //TODO Improve this at some point, we shouldn't itterate over the whole world
   SwapCellValues(from_cell, to_cell);
   if (CheckForMatches())
   {
      is_ready_for_move_ = false;
      g_extraInfo.moves_since_last_reset++;

      if (g_print_ai_moves)
         PrintWorldAsText();
      return true;
   }
   SwapCellValues(from_cell, to_cell);
   return false;
}

/// <summary>
/// Attempts to Tick the game by one event, basically the game loop.
/// </summary>
void Match3::ProgressGame()
{
   // We try to move the cells down
   if (!StepCellsDown() && !ClearMatches())
   {
      is_ready_for_move_ = true;
      // Check for legal moves and return the first one found
      if (!AnyLegalMatchesExist())
      {
         // We give one step of pause to indicate a lack of moves before resetting.
         if (no_valid_moves_) {
            ResetWorld();
         }
         else
         {
            no_valid_moves_ = true;
            g_extraInfo.next_frame_restarts = true;
         }
      }
   }
   else
      is_ready_for_move_ = false;

   CreateCellsMissingInRow(0);
}

/// <summary>
/// Moves any cells that have an EMPTY cell below them down.
/// </summary>
/// <returns>Returns true if any changes are made</returns>
bool Match3::StepCellsDown()
{
   bool isChanged = false;
   for (int y = game_rules_.world_height - 1; y >= 0; y--)
   {
      for (int x = 0; x < game_rules_.world_width; x++)
      {
         if (world_data_[GetCellIndex(x, y)] != EMPTY && world_data_[GetCellIndex(x, y + 1)] == EMPTY)
         {
            world_data_[GetCellIndex(x, y + 1)] = world_data_[GetCellIndex(x, y)];
            world_data_[GetCellIndex(x, y)] = EMPTY;
            isChanged = true;
         }
      }
   }
   return isChanged;
}

/// <summary> Generates new cells inside of a row, only used for 'dropping' new cells. </summary>
/// <returns>True if any changes are made</returns>
bool Match3::CreateCellsMissingInRow(int row = 0)
{
   bool isChanged = false;
   for (int x = 0; x < game_rules_.world_width; x++)
   {
      if (world_data_[GetCellIndex(x, row)] == EMPTY)
      {
         world_data_[GetCellIndex(x, row)] = GetNewRandomCell();
         isChanged = true;
      }
   }
   return isChanged;
}

/// <summary> Sets all cells to the type passed in, if RANDOM is passed in, all cells are set to a random type </summary>
void Match3::SetWorldCells(CellTypes type)
{
   for (int y = 0; y < game_rules_.world_height; y++)
   {
      for (int x = 0; x < game_rules_.world_width; x++)
      {
         world_data_[GetCellIndex(x, y)] = (type == RANDOM ? GetNewRandomCell() : type);
      }
   }
}

/// <summary>
/// Resets the game by clearing all cells
/// </summary>
void Match3::ResetWorld()
{
   // Reset GUI Info
   g_extraInfo.Clear();
   no_valid_moves_ = false;
   SetWorldCells(EMPTY);
}

/// <summary> Returns true if any 'legal' matches exist.
/// Note: This could be much better, it could be made into a pretty tight loop given it is all index lookups, but it wouldn't be very readable.
/// Note: This also has a huge bias towards Vertical moves.</summary>
bool Match3::AnyLegalMatchesExist(IVec2 move[])
{
   // Check for valid moves
   // Vertical Moves
   for (int y = 0; y < game_rules_.world_height; y++)
   {
      for (int x = 0; x < game_rules_.world_width; x++)
      {
         // Check A and C
         if (!IsValidCell(x - 1, y) || !IsValidCell(x + 1, y))
            continue;

         // Cell to left
         const int cellA = GetCellIndex(x - 1, y);
         // Cell to right
         const int cellC = GetCellIndex(x + 1, y);

         // Moving Down from Above
         if (IsValidCell(x, y - 1))
         {
            const int movingCellsIndex = GetCellIndex(x, y - 1);
            if (
               IsMatch(cellA, movingCellsIndex, cellC) ||
               IsValidCell(x - 2, y) && IsMatch(GetCellIndex(x - 2, y), movingCellsIndex, cellA) ||
               IsValidCell(x + 2, y) && IsMatch(GetCellIndex(x + 2, y), movingCellsIndex, cellC))
            {
               if (move != nullptr) {
                  move[CellMove::TO] = IVec2(x, y);
                  move[CellMove::FROM] = IVec2(x, y - 1);
               }
               return true;
            }
         }
         // Middle from Bottom
         if (IsValidCell(x, y + 1))
         {
            const int movingCellsIndex = GetCellIndex(x, y + 1);
            if (
               IsMatch(cellA, movingCellsIndex, cellC) || // Middle
               IsValidCell(x - 2, y) && IsMatch(GetCellIndex(x - 2, y), movingCellsIndex, cellA) || // Right End
               IsValidCell(x + 2, y) && IsMatch(GetCellIndex(x + 2, y), movingCellsIndex, cellC)) // Left End
            {
               if (move != nullptr) {
                  move[CellMove::TO] = IVec2(x, y);
                  move[CellMove::FROM] = IVec2(x, y + 1);
               }
               return true;
            }
         }
      }
   }
         // Horizontal
   for (int y = 0; y < game_rules_.world_height; y++)
   {
      for (int x = 0; x < game_rules_.world_width; x++)
      {
         // Check A and C
         if (!IsValidCell(x, y - 1) || !IsValidCell(x, y + 1))
            continue;

         // Cell Above
         const int cellA = GetCellIndex(x, y - 1);
         // Cell Below
         const int cellC = GetCellIndex(x, y + 1);

         if (IsValidCell(x - 1, y))
         {
            const int movingCellsIndex = GetCellIndex(x - 1, y);
            if (
               IsMatch(cellA, movingCellsIndex, cellC) ||
               IsValidCell(x, y - 2) && IsMatch(GetCellIndex(x, y - 2), movingCellsIndex, cellA) ||
               IsValidCell(x, y + 2) && IsMatch(GetCellIndex(x, y + 2), movingCellsIndex, cellC))
            {
               if (move != nullptr) {
                  move[CellMove::TO] = IVec2(x, y);
                  move[CellMove::FROM] = IVec2(x - 1, y);
               }
               return true;
            }
         }

         if (IsValidCell(x + 1, y))
         {
            const int movingCellsIndex = GetCellIndex(x + 1, y);
            if (IsMatch(cellA, GetCellIndex(x + 1, y), cellC) ||
               IsValidCell(x, y - 2) && IsMatch(GetCellIndex(x, y - 2), movingCellsIndex, cellA) ||
               IsValidCell(x, y + 2) && IsMatch(GetCellIndex(x, y + 2), movingCellsIndex, cellC))
            {
               if (move != nullptr) {
                  move[CellMove::TO] = IVec2(x, y);
                  move[CellMove::FROM] = IVec2(x + 1, y);
               }
               return true;
            }
         }
      }
   }
   return false;
}

bool Match3::IsMatch(int cell_a, int cell_b, int cell_c)
{
   return (world_data_[cell_a] == world_data_[cell_b] && world_data_[cell_a] == world_data_[cell_c]);
}

bool Match3::IsValidCell(const int x, const int y) const
{
   return (x >= 0 && x < game_rules_.world_width&& y >= 0 && y < game_rules_.world_height);
}
/// <summary> 
///  Searches world_data_ for >3 of a kind, and replaces them with Empty cells.
/// </summary>
/// <returns>True if any cells are changed</returns>
bool Match3::ClearMatches()
{
   bool isChanged = false;
   for (int y = 0; y < game_rules_.world_height; y++)
   {
      for (int x = 0; x < game_rules_.world_width; x++)
      {
         const short result = CheckMatches(x, y);
         if (result == MatchType::VERTICAL)
         {
            world_clear_array_.push_back(GetCellIndex(x, y));
            world_clear_array_.push_back(GetCellIndex(x, y - 1));
            world_clear_array_.push_back(GetCellIndex(x, y + 1));
         }
         else if (result == MatchType::HORIZONTAL)
         {
            world_clear_array_.push_back(GetCellIndex(x, y));
            world_clear_array_.push_back(GetCellIndex(x - 1, y));
            world_clear_array_.push_back(GetCellIndex(x + 1, y));
         }
      }
   }

   if (!world_clear_array_.empty()) {
      isChanged = true;
      g_extraInfo.ClearMovedCells();
      for (auto index : world_clear_array_)
      {
         // Lazy tracking, we will have duplicates
         if (world_data_[index] != EMPTY) {
            world_data_[index] = EMPTY;
            // Lazy score, we just add all the cells we remove.
            g_extraInfo.AddPoint();
         }
      }
      world_clear_array_.clear();
   }
   return isChanged;
}

/// <summary>
/// Due to time limits and my self-made limit to finish by Wednesday evening, I created this method to do a dirty check of the world for any matches.
/// This allows us to check if any exist without a new algorithm to check the outside cases (catch 1011 where 1 is moved to the right, my logic only checked for changes from the middle which is silly).
/// Relatively cheap due to the scope, but not what should be done.
/// </summary>
/// <returns>True if any matches are discovered</returns>
bool Match3::CheckForMatches()
{
   for (int y = 0; y < game_rules_.world_height; y++)
   {
      for (int x = 0; x < game_rules_.world_width; x++)
      {
         if (CheckMatches(x, y))
         {
            return true;
         }
      }
   }
   return false;
}

/// <summary>
/// Checks if there is a match in either direction on this position.
/// </summary>
/// <returns>Returns 0 if no match, 1 if Vertical -1 if Horizontal</returns>
short Match3::CheckMatches(const int x, const int y)
{
   if (world_data_[GetCellIndex(x,y)] == EMPTY) return false;

   if ((IsValidCell(x, y - 1) && IsValidCell(x, y) && IsValidCell(x, y + 1)) && IsMatch(GetCellIndex(x, y - 1), GetCellIndex(x, y), GetCellIndex(x, y + 1)))
      return VERTICAL;
   if ((IsValidCell(x - 1, y) && IsValidCell(x, y) && IsValidCell(x + 1, y) && IsMatch(GetCellIndex(x - 1, y), GetCellIndex(x, y), GetCellIndex(x + 1, y))))
      return HORIZONTAL;

   return NO_MATCH;
}

void Match3::Start()
{
   GeneratePlayField(game_settings->world_size.x, game_settings->world_size.y, game_settings->cell_types_used);
   PrintWorldAsText();
}

bool Match3::Draw(Camera* camera)
{
   const IVec2 screenSize = game_settings->screen_size;

   const int cellSpacing = 32;
   const int cellExtraSpace = 16;

   const IVec2 movedFrom = g_extraInfo.last_cell_moved_from;
   const IVec2 movedTo = g_extraInfo.last_cell_moved_to;

   // All use same shader
   glUseProgram(game_settings->default_shader);
   for (int y = 0; y < game_rules_.world_height; y++)
   {
      for (int x = 0; x < game_rules_.world_width; x++)
      {
         glActiveTexture(GL_TEXTURE0);
         glBindTexture(GL_TEXTURE_2D, coloured_textures_[world_data_[GetCellIndex(x, y)]]);

         glm::mat4 model = glm::mat4(1.0f);
         //TODO Fix this
         glm::vec3 modelPosition = glm::vec3(cellExtraSpace + (cell_screen_size / 2) + ((cell_screen_size + cellSpacing) * x), screenSize.y - (cellExtraSpace + (cell_screen_size / 2) + ((cell_screen_size + cellSpacing) * y)), 1.0f);
         model = glm::translate(model, modelPosition);

         const float movedScaleMultiplier = ((x == movedFrom.x && y == movedFrom.y) || (x == movedTo.x && y == movedTo.y)) ? 1.5f : 1.0f;

         model = glm::scale(model, glm::vec3(cell_screen_size * movedScaleMultiplier, cell_screen_size * movedScaleMultiplier, 1.0f));

         const int modelLoc = glGetUniformLocation(game_settings->default_shader, "model");
         glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
         const int projLoc = glGetUniformLocation(game_settings->default_shader, "projection");
         glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(camera->GetProjection()));

         glBindVertexArray(vao_);
         glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
      }
   }

   return true;
}

void Match3::Update(double delta)
{
   world_update_cooldown_x_ -= delta;
   if (0.0 > world_update_cooldown_x_) {
      world_update_cooldown_x_ = world_update_rate_;
      ProgressGame();
   }

   if (InputManager::Instance()->GetKeyDown(KeyCode::Space))
      world_update_rate_ = (world_update_rate_ == 250 ? 50 : 250);

   // Update GUI Info
   g_extraInfo.world_step_cooldown = world_update_cooldown_x_;
}

// Swaps the values in world_data at the related CellIndexes
void Match3::SwapCellValues(IVec2 from_cell, IVec2 to_cell)
{
   const int temp = world_data_[GetCellIndex(from_cell.x, from_cell.y)];
   world_data_[GetCellIndex(from_cell.x, from_cell.y)] = world_data_[GetCellIndex(to_cell.x, to_cell.y)];
   world_data_[GetCellIndex(to_cell.x, to_cell.y)] = temp;

   // GUI Info
   g_extraInfo.last_cell_moved_from = from_cell;
   g_extraInfo.last_cell_moved_to = to_cell;
}

// Returns a value between 1 and the cell_types_used (inclusive)
int Match3::GetNewRandomCell() const
{
   return InclusiveRandom(1, game_rules_.cell_types_used);
}
