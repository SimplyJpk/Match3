#include "Match3.h"

#include "FloatExtensions.h"


Match3::Match3(GameSettings* settings)
{
   game_settings = settings;

   srand(time(0));

   coloured_textures = new GLuint[CELL_TYPE_COUNT];
   glGenTextures(CELL_TYPE_COUNT, coloured_textures);

   for (int i = 0; i < CELL_TYPE_COUNT; i++) {
      glBindTexture(GL_TEXTURE_2D, coloured_textures[i]);

      TextureUtility::SetTexParams();

      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, cell_texture_size, cell_texture_size, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, &CellTypeColours[i]);
   }

   glUseProgram(game_settings->default_shader);
   glUniform1i(glGetUniformLocation(game_settings->default_shader, "ourTexture"), 0);

   glGenVertexArrays(1, &VAO);
   glGenBuffers(1, &VBO);
   glGenBuffers(1, &EBO);

   glBindVertexArray(VAO);

   glBindBuffer(GL_ARRAY_BUFFER, VBO);
   glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

   // position attribute
   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
   glEnableVertexAttribArray(0);
   // texture coord attribute
   glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
   glEnableVertexAttribArray(1);
}

void Match3::PrintWorldAsText()
{
   if (world_total_size != 0)
   {
      for (int y = 0; y < world_height; y++) 
      {
         for (int x = 0; x < world_width; x++)
         {
            printf("%i", WorldData[CellIndex(x, y)]);
         }
         printf("\n");
      }
   }
}

bool Match3::GeneratePlayField(Uint32 width, Uint32 height, Uint32 numTypes)
{
   world_width = width;
   world_height = height;
   world_total_size = world_width * world_height;
   cellTypesUsed = numTypes;
   if (WorldData == nullptr)
      WorldData = new int[world_total_size];

   for (int y = 0; y < world_height; y++)
   {
      for (int x = 0; x < world_width; x++)
      {
         WorldData[CellIndex(x, y)] = GetNewRandomCell();
      }
   }
   return true;
}

bool Match3::Step(IVec2 fromCell, IVec2 toCell)
{
   // Check cells being passed in are valid cells
   if (!(IsValidCell(fromCell.x, fromCell.y) || IsValidCell(toCell.x, toCell.y)))
      return false;

   // We only want to move 1 square, anything other than 1 should be an invalid move
   if (!FloatsEqual(IVec2::Distance(fromCell, toCell), 1.0f))
      return false;

   int xDir = (fromCell.x - toCell.x);
   int yDir = (fromCell.y - toCell.y);

   // Horizontal
   if (xDir != 0)
   {
      // Check A and C
      if (!IsValidCell(toCell.x, toCell.y - 1) || !IsValidCell(toCell.x, toCell.y + 1))
         return false;

      // Cell to left
      int CellA = CellIndex(toCell.x, toCell.y - 1);
      // Cell to right
      int CellC = CellIndex(toCell.x, toCell.y + 1);

      if (IsMatch(CellA, CellIndex(fromCell.x, fromCell.y), CellC))
         return true;
   }
   // Vertical
   if (yDir != 0)
   {
      // Check A and C
      if (!IsValidCell(toCell.x - 1, toCell.y) || !IsValidCell(toCell.x + 1, toCell.y))
         return false;

      // Cell Above
      int CellA = CellIndex(toCell.x - 1, toCell.y);
      // Cell Below
      int CellC = CellIndex(toCell.x + 1, toCell.y);

      if (IsMatch(CellA, CellIndex(fromCell.x, fromCell.y), CellC))
         return true;
   }
}

void Match3::ProgressGame()
{
   ClearMatches();
   if (AnyLegalMatchesExist(true)) {
   }
   else
   {
      GeneratePlayField(world_width, world_height, cellTypesUsed);
   }
   MoveCellsDown();
   RegenCellsInRowMissing(0);
}


void Match3::MoveCellsDown()
{
   for (int y = world_height - 1; y >= 0; y--)
   {
      for (int x = 0; x < world_width; x++)
      {
         if (WorldData[CellIndex(x, y)] != EMPTY && WorldData[CellIndex(x, y + 1)] == EMPTY)
         {
            WorldData[CellIndex(x, y + 1)] = WorldData[CellIndex(x, y)];
            WorldData[CellIndex(x, y)] = EMPTY;
         }
      }
   }
}

void Match3::RegenCellsInRowMissing(int row = 0)
{
   for (int y = 0; y <= row; y++)
   {
      for (int x = 0; x < world_width; x++)
      {
         if (WorldData[CellIndex(x, y)] == EMPTY)
         {
            WorldData[CellIndex(x, y)] = GetNewRandomCell();
         }
      }
   }
}

/// <summary> Returns true if any 'legal' matches exist. </summary>
bool Match3::AnyLegalMatchesExist(bool makeMove)
{
   // This is cheaper than my move checker
   // We check and see if there are any empty cells
   // If any exist, we still return true to avoid clearing the world when a valid move may show up next step.
   for (int index = 0; index < world_total_size; index++)
   {
      if (WorldData[index] == EMPTY) return true;
   }

   // Check for valid moves
   // Vertical Moves
   for (int y = 0; y < world_height; y++)
   {
      for (int x = 0; x < world_width; x++)
      {
         // Check A and C
         if (!IsValidCell(x - 1, y) || !IsValidCell(x + 1, y))
            continue;

         // Cell to left
         int CellA = CellIndex(x - 1, y);
         // Cell to right
         int CellC = CellIndex(x + 1, y);

         if (IsValidCell(x, y - 1))
         {
            if (IsMatch(CellA, CellIndex(x, y - 1), CellC)) {
               if (makeMove)
                  if(Step(IVec2(x, y - 1), IVec2(x, y)))
                  {
                     int temp = WorldData[CellIndex(x, y)];
                     WorldData[CellIndex(x, y)] = WorldData[CellIndex(x, y - 1)];
                     WorldData[CellIndex(x, y - 1)] = temp;
                  }
               return true;
            }
         }
         if (IsValidCell(x, y + 1))
         {
            if (IsMatch(CellA, CellIndex(x, y + 1), CellC)) {
               if (makeMove)
                  if(Step(IVec2(x, y + 1), IVec2(x, y)))
                  {
                     int temp = WorldData[CellIndex(x, y)];
                     WorldData[CellIndex(x, y)] = WorldData[CellIndex(x, y + 1)];
                     WorldData[CellIndex(x, y +1)] = temp;
                  }
               return true;
            }
         }
      }
   }
   // Horizontal
   for (int y = 0; y < world_height; y++)
   {
      for (int x = 0; x < world_width; x++)
      {
         // Check A and C
         if (!IsValidCell(x, y - 1) || !IsValidCell(x, y + 1))
            continue;

         // Cell Above
         int CellA = CellIndex(x, y - 1);
         // Cell Below
         int CellC = CellIndex(x, y + 1);

         if (IsValidCell(x - 1, y))
         {
            if (IsMatch(CellA, CellIndex(x - 1, y), CellC)) {
               if (makeMove)
                  if (Step(IVec2(x - 1, y), IVec2(x, y)))
                  {
                     int temp = WorldData[CellIndex(x, y)];
                     WorldData[CellIndex(x, y)] = WorldData[CellIndex(x - 1, y)];
                     WorldData[CellIndex(x - 1, y)] = temp;
                  }
               return true;
            }
         }
         if (IsValidCell(x + 1, y))
         {
            if (IsMatch(CellA, CellIndex(x + 1, y), CellC)) {
               if (makeMove)
                  if (Step(IVec2(x + 1, y), IVec2(x, y)))
                  {
                     int temp = WorldData[CellIndex(x, y)];
                     WorldData[CellIndex(x, y)] = WorldData[CellIndex(x + 1, y)];
                     WorldData[CellIndex(x + 1, y)] = temp;
                  }
               return true;
            }
         }
      }
   }
   return false;
}

bool Match3::IsMatch(int cellA, int CellB, int CellC)
{
   return (WorldData[cellA] == WorldData[CellB] && WorldData[cellA] == WorldData[CellC]);
}

bool Match3::IsValidCell(const int x, const int y) const
{
   return (x >= 0 && x < world_width&& y >= 0 && y < world_height);
}

void Match3::ClearMatches()
{
   for (int y = 0; y < world_height; y++)
   {
      for (int x = 0; x < world_width; x++)
      {
         //TODO Move this somewhere else if have time later.
         if (CheckMatches(x, y, true))
         {
            worldClearArray.push_back(CellIndex(x, y));
            worldClearArray.push_back(CellIndex(x, y - 1));
            worldClearArray.push_back(CellIndex(x, y + 1));
         }
         else if (CheckMatches(x, y, false))
         {
            worldClearArray.push_back(CellIndex(x, y));
            worldClearArray.push_back(CellIndex(x - 1, y));
            worldClearArray.push_back(CellIndex(x + 1, y));
         }
      }
   }

   for (auto index : worldClearArray)
   {
      WorldData[index] = EMPTY;
   }
   worldClearArray.clear();
}

bool Match3::CheckMatches(int x, int y, bool vertical)
{
   if (WorldData[CellIndex(x,y)] == EMPTY) return false;

   if (x == 0 && vertical == false) return false;
   if (x == world_width-1 && vertical == false) return false;
   if (y == 0 && vertical == true) return false;
   if (y == world_height - 1 && vertical == false) return false;

   const int middleCell = CellIndex(x, y);

   bool Matches = false;
   if (vertical)
      Matches = IsMatch(CellIndex(x, y - 1), middleCell, CellIndex(x, y + 1));
   else
      Matches = IsMatch(CellIndex(x - 1, y), middleCell, CellIndex(x + 1, y));

   if (Matches)
   {
      if (y + 1 == world_height - 1) return true;
      else if (WorldData[CellIndex(x, y + 2)] != EMPTY) return true;
   }
   return false;
}

void Match3::Start()
{
   GeneratePlayField(8, 8, CELL_TYPE_COUNT - 1);
   PrintWorldAsText();
}

bool Match3::Draw(Camera* camera)
{
   const IVec2 screenSize = game_settings->screen_size;

   const int cell_spacing = 32;
   const int cell_extra_space = 16;

   // All use same shader
   glUseProgram(game_settings->default_shader);
   for (int y = 0; y < world_height; y++)
   {
      for (int x = 0; x < world_width; x++)
      {
         glActiveTexture(GL_TEXTURE0);
         glBindTexture(GL_TEXTURE_2D, coloured_textures[WorldData[CellIndex(x, y)]]);

         glm::mat4 model = glm::mat4(1.0f);
         //TODO Fix this
         glm::vec3 modelPosition = glm::vec3(cell_extra_space + (cell_screen_size / 2) + ((cell_screen_size + cell_spacing) * x), screenSize.y - (cell_extra_space + (cell_screen_size / 2) + ((cell_screen_size + cell_spacing) * y)), 1.0f);
         model = glm::translate(model, modelPosition);

         model = glm::scale(model, glm::vec3(cell_screen_size, cell_screen_size, 1.0f));

         int modelLoc = glGetUniformLocation(game_settings->default_shader, "model");
         glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
         int projLoc = glGetUniformLocation(game_settings->default_shader, "projection");
         glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(camera->GetProjection()));


         glBindVertexArray(VAO);
         glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
      }
   }

   return true;
}

int Match3::GetNewRandomCell() const
{
   return Random(1, cellTypesUsed);
}
