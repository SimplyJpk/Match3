#pragma once
// ImGUI
#include <GL/glew.h>
#include "imgui.h"
#include <backends/imgui_impl_sdl.h>
#include <backends/imgui_impl_opengl3.h>
#include <SDL.h>

#include "GameSettings.h"
#include "ExtraInfoGUI.h"

class GuiManager
{
public:
   SDL_Window* g_window;
   ExtraInfoGUI* g_extraInfo;

   GuiManager(GameSettings* settings, SDL_Window* window, SDL_GLContext* context, ExtraInfoGUI* guiInfo)
   {
      settings_ = settings;

      ImGui::CreateContext();
      ImGuiIO& io = ImGui::GetIO(); (void)io;

      ImGui::StyleColorsDark();

      ImGui_ImplSDL2_InitForOpenGL(window, context);
      ImGui_ImplOpenGL3_Init();

      g_extraInfo = guiInfo;

      ImGui::SetWindowSize("Debug Window", ImVec2(205, 82));
      ImGui::SetWindowPos("Debug Window", ImVec2(474, 532));

      ImGui::SetWindowSize("Information", ImVec2(417, 200));
      ImGui::SetWindowPos("Information", ImVec2(18, 515));
      g_window = window;
   }

   void NewGuiFrame() const
   {
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplSDL2_NewFrame(g_window);
      ImGui::NewFrame();
   }

   // Finializes rendering so it can be drawn.
   void FinishGuiFrame()
   {
      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
   }

   void DrawFrameData()
   {
      ImGui::Begin("Information");

      // Match3 Relevant Info
      ImGui::Text("Cell Moved From:\n(%i,%i) -> (%i,%i)", g_extraInfo->last_cell_moved_from.x, g_extraInfo->last_cell_moved_from.y, g_extraInfo->last_cell_moved_to.x, g_extraInfo->last_cell_moved_to.y);
      ImGui::Text("HighScore: %i", (g_extraInfo->game_score > g_extraInfo->game_high_score ? g_extraInfo->game_score : g_extraInfo->game_high_score));
      ImGui::Text("Game Score: %i", g_extraInfo->game_score);
      ImGui::Text("Moves Since last reset: %i", g_extraInfo->moves_since_last_reset);
      ImGui::Text("Restarts next Step: %i", g_extraInfo->next_frame_restarts);
      ImGui::Text("Next Game Step: %0.3f", g_extraInfo->world_step_cooldown);

      ImGui::Text("Key 'A': Step Game");
      ImGui::Text("Key 'L': Toggle AI log");
      ImGui::Text("Key 'P': Print world to console");
      ImGui::Text("Key 'Space': Toggle between 50ms and 250ms Game Step Time");

      ImGui::End();
   }

   void DrawGui()
   {
      DrawFrameData();
      ImGui::Begin("Debug Window");
      ImGui::Text("Screen Size: W-%i\tH-%i", settings_->screen_size.x, settings_->screen_size.y);


      ImGui::Text("Target FPS %0.2f", settings_->target_frames_per_second);
      ImGui::Text("Max Frame Delay: %0.2f", settings_->calculated_frame_delay);
      ImGui::End();
   }

   ~GuiManager()
   {
      ImGui_ImplOpenGL3_Shutdown();
      ImGui_ImplSDL2_Shutdown();
      ImGui::DestroyContext();
   }

private:
   GameSettings* settings_;
};
