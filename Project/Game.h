#pragma once
#include <SDL.h>
#include "GameSettings.h"
#include "GUIManager.h"
#include "ShaderManager.h"
#include "InputManager.h"

#include <chrono>

#include "Camera.h"
#include "Match3.h"

class Game
{
   SDL_GLContext* g_context;
   SDL_Window* g_window;

   GameSettings* game_settings;
   GuiManager* gui_manager;
   InputManager* input_manager;

   Match3* match3;
   Camera main_cam;

   GLint defaultShader;

public:
   bool Initialize(SDL_GLContext* gl_context, SDL_Window* gl_window, GameSettings* settings);
   void Run();
};
