#include "Match3.h"
#include "FloatExtensions.h"

Match3::Match3(GameSettings* settings): g_world_width(0), g_world_height(0), g_world_total_size(0)
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

void Match3::PrintWorldAsText() const
{
   printf("-----\n");
   if (g_world_total_size != 0)
   {
      for (int y = 0; y < g_world_height; y++) 
      {
         for (int x = 0; x < g_world_width; x++)
         {
            printf("%i", world_data[GetCellIndex(x, y)]);
         }
         printf("\n");
      }
   }
}

bool Match3::GeneratePlayField(Uint32 width, Uint32 height, Uint32 numTypes)
{
   // Reset GUI Info
   g_extraInfo.Clear();

   // Game Stuff
   g_world_width = width;
   g_world_height = height;
   g_world_total_size = g_world_width * g_world_height;
   if (numTypes > CELL_TYPE_COUNT)
      numTypes = CELL_TYPE_COUNT;

   g_cell_types_used = numTypes;
   if (world_data == nullptr) {
      world_data = new int[g_world_total_size]{0};
   }

   // Clear all tiles
   no_valid_moves_ = false;
   SetWorldCells(RANDOM);

   return true;
}

bool Match3::Step(const IVec2 from_cell, const IVec2 to_cell)
{
   // Check cells being passed in are valid cells
   if (!(IsValidCell(from_cell.x, from_cell.y) || IsValidCell(to_cell.x, to_cell.y)))
      return false;

   // We only want to move 1 square, anything other than 1 should be an invalid move
   if (!FloatsEqual(IVec2::Distance(from_cell, to_cell), 1.0f))
      return false;

   const int xDir = (from_cell.x - to_cell.x);
   const int yDir = (from_cell.y - to_cell.y);

   bool isMatch = false;
   // Horizontal
   if (xDir != 0)
   {
      // Check A and C
      if (!IsValidCell(to_cell.x, to_cell.y - 1) || !IsValidCell(to_cell.x, to_cell.y + 1))
         return false;

      // Cell to left
      int CellA = GetCellIndex(to_cell.x, to_cell.y - 1);
      // Cell to right
      int CellC = GetCellIndex(to_cell.x, to_cell.y + 1);

      isMatch = IsMatch(CellA, GetCellIndex(from_cell.x, from_cell.y), CellC);
   }
   // Vertical
   if (yDir != 0)
   {
      // Check A and C
      if (!IsValidCell(to_cell.x - 1, to_cell.y) || !IsValidCell(to_cell.x + 1, to_cell.y))
         return false;

      // Cell Above
      int CellA = GetCellIndex(to_cell.x - 1, to_cell.y);
      // Cell Below
      int CellC = GetCellIndex(to_cell.x + 1, to_cell.y);

      isMatch = IsMatch(CellA, GetCellIndex(from_cell.x, from_cell.y), CellC);
   }

   if (isMatch)
   {
      SwapCellValues(from_cell, to_cell);
      return true;
   }
   return false;
}

/// <summary>
/// Attempts to Tick the game by one event, basically the game loop.
/// </summary>
void Match3::ProgressGame()
{
   g_extraInfo.ClearMovedCells();

   // We try to move the cells down
   if (!StepCellsDown())
   {
      // If we don't we try clear matches, if we do have a match we wait for the next input to "Step" makes matches easier to follow
      if (!ClearMatches()) {
         IVec2 moves[2];
         if (AnyLegalMatchesExist(moves)) {
            g_extraInfo.moves_since_last_reset++;
            Step(moves[CellMove::FROM], moves[CellMove::TO]);
         }
         else
         {
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
   }
   CreateCellsMissingInRow(0);
}

/// <summary>
/// Moves any cells that have an EMPTY cell below them down.
/// </summary>
/// <returns>Returns true if any changes are made</returns>
bool Match3::StepCellsDown()
{
   bool isChanged = false;
   for (int y = g_world_height - 1; y >= 0; y--)
   {
      for (int x = 0; x < g_world_width; x++)
      {
         if (world_data[GetCellIndex(x, y)] != EMPTY && world_data[GetCellIndex(x, y + 1)] == EMPTY)
         {
            world_data[GetCellIndex(x, y + 1)] = world_data[GetCellIndex(x, y)];
            world_data[GetCellIndex(x, y)] = EMPTY;
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
   for (int x = 0; x < g_world_width; x++)
   {
      if (world_data[GetCellIndex(x, row)] == EMPTY)
      {
         world_data[GetCellIndex(x, row)] = GetNewRandomCell();
         isChanged = true;
      }
   }
   return isChanged;
}

/// <summary> Sets all cells to the type passed in, if RANDOM is passed in, all cells are set to a random type </summary>
void Match3::SetWorldCells(CellTypes type)
{
   for (int y = 0; y < g_world_height; y++)
   {
      for (int x = 0; x < g_world_width; x++)
      {
         world_data[GetCellIndex(x, y)] = (type == RANDOM ? GetNewRandomCell() : type);
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

/// <summary> Returns true if any 'legal' matches exist. </summary>
bool Match3::AnyLegalMatchesExist(IVec2 move[])
{
   // Check for valid moves
   //
   // Vertical Moves
   for (int y = 0; y < g_world_height; y++)
   {
      for (int x = 0; x < g_world_width; x++)
      {
         // Check A and C
         if (!IsValidCell(x - 1, y) || !IsValidCell(x + 1, y))
            continue;

         // Cell to left
         int CellA = GetCellIndex(x - 1, y);
         // Cell to right
         int CellC = GetCellIndex(x + 1, y);

         if (IsValidCell(x, y - 1))
         {
            if (IsMatch(CellA, GetCellIndex(x, y - 1), CellC)) {
               if (move != nullptr)
               {
                  move[CellMove::TO] = IVec2(x, y);
                  move[CellMove::FROM] = IVec2(x, y - 1);
               }
               return true;
            }
         }
         if (IsValidCell(x, y + 1))
         {
            if (IsMatch(CellA, GetCellIndex(x, y + 1), CellC)) {
               if (move != nullptr)
               {
                  move[CellMove::TO] = IVec2(x, y);
                  move[CellMove::FROM] = IVec2(x, y + 1);
               }
               return true;
            }
         }
      }
   }
         // Horizontal
   for (int y = 0; y < g_world_height; y++)
   {
      for (int x = 0; x < g_world_width; x++)
      {
         // Check A and C
         if (!IsValidCell(x, y - 1) || !IsValidCell(x, y + 1))
            continue;

         // Cell Above
         int CellA = GetCellIndex(x, y - 1);
         // Cell Below
         int CellC = GetCellIndex(x, y + 1);

         if (IsValidCell(x - 1, y))
         {
            if (IsMatch(CellA, GetCellIndex(x - 1, y), CellC)) {
               if (move != nullptr)
               {
                  move[CellMove::TO] = IVec2(x, y);
                  move[CellMove::FROM] = IVec2(x - 1, y);
               }
               return true;
            }
         }
         if (IsValidCell(x + 1, y))
         {
            if (IsMatch(CellA, GetCellIndex(x + 1, y), CellC)) {
               if (move != nullptr)
               {
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
   return (world_data[cell_a] == world_data[cell_b] && world_data[cell_a] == world_data[cell_c]);
}

bool Match3::IsValidCell(const int x, const int y) const
{
   return (x >= 0 && x < g_world_width&& y >= 0 && y < g_world_height);
}
/// <summary> 
///  Searches world_data_ for >3 of a kind, and replaces them with Empty cells.
/// </summary>
/// <returns>True if any cells are changed</returns>
bool Match3::ClearMatches()
{
   bool isChanged = false;

   for (int y = 0; y < g_world_height; y++)
   {
      for (int x = 0; x < g_world_width; x++)
      {
         if (CheckMatches(x, y, true))
         {
            world_clear_array_.push_back(GetCellIndex(x, y));
            world_clear_array_.push_back(GetCellIndex(x, y - 1));
            world_clear_array_.push_back(GetCellIndex(x, y + 1));
         }
         else if (CheckMatches(x, y, false))
         {
            world_clear_array_.push_back(GetCellIndex(x, y));
            world_clear_array_.push_back(GetCellIndex(x - 1, y));
            world_clear_array_.push_back(GetCellIndex(x + 1, y));
         }
      }
   }

   if (world_clear_array_.size() > 0) {
      isChanged = true;
      for (auto index : world_clear_array_)
      {
         world_data[index] = EMPTY;

         // Lazy score, we just add all the cells we remove.
         g_extraInfo.AddPoint();
      }
      world_clear_array_.clear();
   }
   return isChanged;
}

/// <summary>
/// Checks if there is a match on the axis passed in
/// </summary>
/// <param name="vertical">If it looks vertical path or horizontal</param>
/// <returns>True if 3 of a kind is found</returns>
bool Match3::CheckMatches(const int x, const int y, const bool vertical)
{
   if (vertical && y == 0 || vertical && y == g_world_height - 1) return false;
   if (!vertical && x == 0 || !vertical && x == g_world_width - 1) return false;

   if (world_data[GetCellIndex(x,y)] == EMPTY) return false;

   bool Matches = false;
   if (vertical)
      Matches = IsMatch(GetCellIndex(x, y - 1), GetCellIndex(x, y), GetCellIndex(x, y + 1));
   else
      Matches = IsMatch(GetCellIndex(x - 1, y), GetCellIndex(x, y), GetCellIndex(x + 1, y));

   if (Matches)
   {
      if (y + 1 == g_world_height - 1) return true;
      else if (world_data[GetCellIndex(x, y + 2)] != EMPTY) return true;
   }
   return false;
}

void Match3::Start()
{
   GeneratePlayField(8, 8, 5);
   PrintWorldAsText();
}

bool Match3::Draw(Camera* camera)
{
   const IVec2 screenSize = game_settings->screen_size;

   const int cell_spacing = 32;
   const int cell_extra_space = 16;

   const IVec2 moved_from = g_extraInfo.last_cell_moved_from;
   const IVec2 moved_to = g_extraInfo.last_cell_moved_to;

   // All use same shader
   glUseProgram(game_settings->default_shader);
   for (int y = 0; y < g_world_height; y++)
   {
      for (int x = 0; x < g_world_width; x++)
      {
         glActiveTexture(GL_TEXTURE0);
         glBindTexture(GL_TEXTURE_2D, coloured_textures_[world_data[GetCellIndex(x, y)]]);

         glm::mat4 model = glm::mat4(1.0f);
         //TODO Fix this
         glm::vec3 modelPosition = glm::vec3(cell_extra_space + (cell_screen_size / 2) + ((cell_screen_size + cell_spacing) * x), screenSize.y - (cell_extra_space + (cell_screen_size / 2) + ((cell_screen_size + cell_spacing) * y)), 1.0f);
         model = glm::translate(model, modelPosition);

         float movedScaleMultiplier = ((x == moved_from.x && y == moved_from.y) || (x == moved_to.x && y == moved_to.y)) ? 1.5f : 1.0f;

         model = glm::scale(model, glm::vec3(cell_screen_size * movedScaleMultiplier, cell_screen_size * movedScaleMultiplier, 1.0f));

         int modelLoc = glGetUniformLocation(game_settings->default_shader, "model");
         glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
         int projLoc = glGetUniformLocation(game_settings->default_shader, "projection");
         glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(camera->GetProjection()));

         glBindVertexArray(vao_);
         glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
      }
   }

   return true;
}

// Swaps the values in world_data at the related CellIndexes
void Match3::SwapCellValues(IVec2 from_cell, IVec2 to_cell)
{
   const int temp = world_data[GetCellIndex(from_cell.x, from_cell.y)];
   world_data[GetCellIndex(from_cell.x, from_cell.y)] = world_data[GetCellIndex(to_cell.x, to_cell.y)];
   world_data[GetCellIndex(to_cell.x, to_cell.y)] = temp;

   // GUI Info
   g_extraInfo.last_cell_moved_from = from_cell;
   g_extraInfo.last_cell_moved_to = to_cell;
}

// Returns a value between 1 and the cell_types_used (inclusive)
int Match3::GetNewRandomCell() const
{
   return InclusiveRandom(1, g_cell_types_used);
}
