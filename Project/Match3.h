#pragma once
#include <cstdio>
#include <ctime>
#include <SDL_stdinc.h>
#include <GL/glew.h>


#include "CellData.h"
#include "Constants.h"
#include "GameObject.h"
#include "GameSettings.h"
#include "IVec2.h"
#include "TextureUtility.h"
#include "Camera.h"

class Match3 : public GameObject
{
public:
   Uint32 CellTypeColours[CELL_TYPE_COUNT] = {
      0x00000000,
      0xFF0000FF,
      0x00FF00FF,
      0x0000FFFF,
      0xFFFF00FF,
      0xFFFFFFFF,
   };

   GameSettings* game_settings;

   int world_width;
   int world_height;
   int world_total_size;
   int cellTypesUsed = 0;
   // CellData* WorldData = nullptr;

   int* WorldData = nullptr;
   std::vector<int> worldClearArray;

   Match3(GameSettings* settings);

   void PrintWorldAsText();
   bool GeneratePlayField(Uint32 width, Uint32 height, Uint32 numTypes);
   bool Step(IVec2 fromCell, IVec2 toCell);

   void ProgressGame();
   void ClearMatches();
   bool CheckMatches(int x, int y, bool vertical);
   void MoveCellsDown();
   void RegenCellsInRowMissing(int row);

   bool AnyLegalMatchesExist(bool makeMove);
   bool IsMatch(int cellA, int CellB, int CellC);
   bool IsValidCell(int x, int y) const;

   void Start() override;
   bool Draw(Camera* camera) override;

private:
   int CellIndex(int x, int y) const;
   int GetNewRandomCell() const;

   static int Random(int min, int max)
   {
      return rand() % (max - min + 1) + min;
   }

   GLuint* coloured_textures;
   unsigned int VBO;
   unsigned int VAO;
   unsigned int EBO;
   float vertices[20] = {
      // positions                    // texture coords
       0.5f,  0.5f, 0.0f,   1.0f, 1.0f, // top right
       0.5f, -0.5f, 0.0f,   1.0f, 0.0f, // bottom right
      -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, // bottom left
      -0.5f,  0.5f, 0.0f,   0.0f, 1.0f  // top left 
   };
   unsigned int indices[6] = {
       0, 1, 3, // first triangle
       1, 2, 3  // second triangle
   };
};

inline int Match3::CellIndex(int x, int y) const
{
   return ((world_width * y) + x);
}
