#pragma once
#include <cstdio>
#include <SDL_stdinc.h>
#include <GL/glew.h>


#include "CellTypes.h"
#include "GameObject.h"
#include "GameSettings.h"
#include "IVec2.h"
#include "Camera.h"
#include "ExtraInfoGUI.h"

class Match3 : public GameObject
{
public:
   // Used for some additional on-screen information
   ExtraInfoGUI g_extraInfo;

   enum CellMove { FROM = 0, TO = 1 };

   GameSettings* game_settings;

   int g_world_width;
   int g_world_height;
   int g_world_total_size;
   int g_cell_types_used = 0;

   Match3(GameSettings* settings);

   // Required by Technical Sheet
   void PrintWorldAsText() const;
   bool GeneratePlayField(Uint32 width, Uint32 height, Uint32 numTypes);

   bool AnyLegalMatchesExist(IVec2 move[]);
   bool Step(IVec2 from_cell, IVec2 to_cell);

   // General Purpose
   void ProgressGame();

   // Helpers
   short CheckMatches(int x, int y);
   int GetCellIndex(int x, int y) const;
   bool IsMatch(int cell_a, int cell_b, int cell_c);
   bool IsValidCell(int x, int y) const;

   // Inherited
   void Start() override;
   bool Draw(Camera* camera) override;

private:
   int* world_data_ = nullptr;

   void SwapCellValues(IVec2 from_cell, IVec2 to_cell);
   int GetNewRandomCell() const;
   bool CreateCellsMissingInRow(int row);
   void SetWorldCells(CellTypes type);
   void ResetWorld();

   bool ClearMatches();
   bool CheckForMatches();
   bool StepCellsDown();

   bool no_valid_moves_ = false;

   // Filled during ClearMatches to match all cells before clearing them
   std::vector<int> world_clear_array_;
   // Returns random number >= min <= max
   static int InclusiveRandom(const int min, const int max)
   {
      return rand() % (max - min + 1) + min;
   }
   // OpenGL texture indexes
   GLuint* coloured_textures_;
   unsigned int vbo_;
   unsigned int vao_;
   unsigned int ebo_;
};

/// <summary> Returns the 1D Array cell index based on the X and Y passed in </summary>
/// <returns>( (world_width * y) + x )</returns>
inline int Match3::GetCellIndex(const int x, const int y) const
{
   return ((g_world_width * y) + x);
}