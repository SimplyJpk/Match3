#pragma once
// ImGUI
#include <gl/GLEW.h>
#include "imgui.h"
#include <backends/imgui_impl_sdl.h>
#include <backends/imgui_impl_opengl3.h>
#include <SDL.h>

#include "GameSettings.h"
#include "Constants.h"

class GuiManager
{
public:
   //Temp Colours
   ImVec4 red = ImVec4(1, 0, 0, 1);
   ImVec4 green = ImVec4(0, 1, 0, 1);

   SDL_Window* g_window;

   float plotData[100];

   GuiManager(GameSettings* settings, SDL_Window* window, SDL_GLContext* context)
   {
      settings_ = settings;

      ImGui::CreateContext();
      ImGuiIO& io = ImGui::GetIO(); (void)io;

      ImGui::StyleColorsDark();

      ImGui_ImplSDL2_InitForOpenGL(window, context);
      ImGui_ImplOpenGL3_Init();

      ImGui::SetWindowSize("Debug Window", ImVec2(240, 240));
      ImGui::SetWindowPos("Debug Window", ImVec2(settings_->screen_size.x - 245, 15));

      ImGui::SetWindowSize("Frame Data", ImVec2(240, 240));
      ImGui::SetWindowPos("Frame Data", ImVec2(settings_->screen_size.x - 245, 275));
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
      ImGui::Begin("Frame Data");

      ImGui::Text("Target FPS %0.2f", settings_->target_frames_per_second);
      ImGui::Text("Max Frame Delay: %0.2f", settings_->calculated_frame_delay);

      ImGui::End();
   }

   void DrawGui()
   {
      DrawFrameData();
      ImGui::Begin("Debug Window");
      ImGui::Text("Screen Size: W-%i\tH-%i", settings_->screen_size.x, settings_->screen_size.y);
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
