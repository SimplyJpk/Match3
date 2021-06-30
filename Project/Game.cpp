#include "Game.h"

typedef ShaderManager::ShaderTypes ShaderType;

bool Game::Initialize(SDL_GLContext* gl_context, SDL_Window* gl_window, GameSettings* settings)
{
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   game_settings = settings;
   g_context = gl_context;
   g_window = gl_window;

   glViewport(0, 0, settings->screen_size.x, settings->screen_size.y);

   if (!ShaderManager::Instance()->CompileShader("orthoWorld", ShaderType::Vertex, "shaders/orthoWorld.vert"))
      printf("Failed to generate Vertex Shader");
   if (!ShaderManager::Instance()->CompileShader("orthoWorld", ShaderType::Fragment, "shaders/orthoWorld.frag"))
      printf("Failed to generate Frag Shader");

   defaultShader = ShaderManager::Instance()->CreateShaderProgram("orthoWorld", false);
   game_settings->default_shader = defaultShader;

   match3 = new Match3(settings);

   // Initialize ImGUI
   gui_manager = new GuiManager(game_settings, g_window, g_context, &match3->g_extraInfo);

   // Input
   input_manager = InputManager::Instance();
   // Camera
   main_cam.SetOrtho(0, game_settings->screen_size.x, 0, game_settings->screen_size.y);

   return true;
}

void Game::Run()
{
   //? Debug Info?
   typedef std::chrono::steady_clock clock;
   typedef std::chrono::duration<float, std::milli> duration;

   auto deltaClock = clock::now();
   double deltaTime = 0.0;
   double fixedRemainingTime = 0.0;

   //? Debug Info
   auto frameStart = clock::now();
   auto secondCounter = clock::now();
   int frameCounter = 0;

   match3->Start();
   bool isLockInputOn = false;

   while (!input_manager->IsShuttingDown())
   {
      deltaTime = static_cast<duration>(clock::now() - deltaClock).count();
      deltaClock = clock::now();
      frameStart = clock::now();

      fixedRemainingTime += deltaTime;

      uint8_t frameFixedStepCounter = 0;
      while (fixedRemainingTime > game_settings->fixed_update_time) {
         fixedRemainingTime -= game_settings->fixed_update_time;

         // Prevents lockup of game during intensive updates, hopefully allow recovery.
         frameFixedStepCounter++;
         if (frameFixedStepCounter >= game_settings->max_fixed_updates_per_frame)
            break;
      }

      //? ======
      //! Update
      input_manager->Update();
      if (input_manager->IsShuttingDown()) break;

      match3->Update(deltaTime);

      if (isLockInputOn || InputManager::Instance()->GetKeyDown(KeyCode::A))
         match3->ProgressGame();

      if (InputManager::Instance()->GetKeyDown(KeyCode::P))
         match3->PrintWorldAsText();

      if (InputManager::Instance()->GetKeyDown(KeyCode::Space))
         isLockInputOn = !isLockInputOn;

      //? ======
      //! Render
      // Clear Screen
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      match3->Draw(&main_cam);

      gui_manager->NewGuiFrame();

      gui_manager->DrawGui();


      gui_manager->FinishGuiFrame();


      SDL_GL_SwapWindow(g_window);

   }

   //TODO Should do some cleanup for destruction?
}
