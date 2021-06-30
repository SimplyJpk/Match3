#include "GL/glew.h"
#include <SDL.h>
#include <SDL_opengl.h>

#include "IO.h"
#include "GameSettings.h"
#include "ConfigFile.h"

#include "Game.h"
#include <iostream>

bool CreateWindowAndContext(GameSettings* settings);

SDL_GLContext g_context;
SDL_Window* g_window;
Game* game;

int main(int argc, char** argv)
{
   bool success = true;

   //TODO Implement a check for all required directories?
   IO::create_file_if_not_exist("/data/");

   // Load our Config class which we can then use to construct out GameSettings
   printf("Loading config file..\n");
   ConfigFile config;
   config.StartLoad(config.FilePath().c_str(), true);

   // Generate our game settings with the Serialized config file
   GameSettings* settings = new GameSettings();
   settings->LoadSettings(config);

   if (!CreateWindowAndContext(settings))
      success = false;

   game = new Game();
   if (!success || !game->Initialize(&g_context, g_window, settings))
   {
      printf("Failed to Initialize");
      success = false;
   }
   else
   {
      game->Run();
   }

   SDL_DestroyWindow(g_window);
   SDL_Quit();
   return 0;
}

bool CreateWindowAndContext(GameSettings* settings)
{
   bool success = true;

   SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
   g_window = SDL_CreateWindow(
      "Gameloft Technical Assessment", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
      settings->screen_size.x, settings->screen_size.y,
      SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
   );

   if (g_window == nullptr) {
      printf("Window could not be created!\nSDL Err: %s\n", SDL_GetError());
      success = false;
   }
   g_context = SDL_GL_CreateContext(g_window);
   if (g_context == nullptr)
   {
      printf("OpenGL context could not be created! SDL Err: %s\n", SDL_GetError());
      success = false;
   }
   else
   {
      // Vsync cause we're bad
      if (SDL_GL_SetSwapInterval(1) < 0)
      {
         printf("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
      }
      glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
   }
   // Init glew, has to be done after context
   GLenum err = glewInit();
   if (GLEW_OK != err)
   {
      /* Problem: glewInit failed, something is seriously wrong. */
      std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
      success = false;
   }
   std::cerr << "Status: Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;
   return success;
}