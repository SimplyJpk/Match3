#pragma once

inline constexpr short cell_texture_size = 1;
inline constexpr short cell_screen_size = 32;

// Square
inline constexpr float vertices[20] = {
   // positions         // texture coords
   0.5f,  0.5f, 0.0f,   1.0f, 1.0f, // top right
   0.5f, -0.5f, 0.0f,   1.0f, 0.0f, // bottom right
  -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, // bottom left
  -0.5f,  0.5f, 0.0f,   0.0f, 1.0f  // top left 
};

inline constexpr unsigned int indices[6] = {
    0, 1, 3,
    1, 2, 3
};