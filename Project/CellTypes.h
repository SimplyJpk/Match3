#pragma once


enum CellTypes
{
   EMPTY = 0,
   RED = 1,
   GREEN = 2,
   BLUE = 3,
   YELLOW = 4,
   WHITE = 5,
   ORANGE,
   CYAN,
   BROWN,
   PURPLE,
   CELL_TYPE_COUNT,
   RANDOM
};

inline Uint32 g_cell_colours[CELL_TYPE_COUNT] = {
   0x00000000, // EMPTY
   0xFF0000FF, // RED
   0x00FF00FF, // GREEN
   0x0000FFFF, // BLUE
   0xFFFF00FF, // YELLOW
   0xFFFFFFFF, // WHITE
   0xFF8000FF, // ORANGE
   0x33CCCCFF, // CYAN
   0x996633FF, // BROWN
   0x9900CCFF, // PURPLE
};

enum MatchType
{
   NONE = 0,
   TOP = 1,
   CENTER = 2,
   BOTTOM = 3
};
