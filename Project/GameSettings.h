#pragma once
#include "GL/glew.h"

#include "Math.h"

#include "ConfigFile.h"

#include "Constants.h"
struct GameSettings
{
   GLint default_shader;

   /* Game Settings that will be global */
   IVec2 screen_size = IVec2(1280, 720);
   float aspect_ratio = screen_size.x / screen_size.y;

   // Game Fixed TimeStep
   uint8_t target_fixed_updates_per_second = 60;
   double fixed_update_time = 1000.0 / target_fixed_updates_per_second;
   int16_t max_fixed_updates_per_frame = 10;

   float target_frames_per_second = 60.0f;
   float calculated_frame_delay = 1000.0f / target_frames_per_second;

   int cell_types_used = 5;
   IVec2 world_size = IVec2(8,8);

   void LoadSettings(ConfigFile& config)
   {
      screen_size.x = config.screen_x;
      screen_size.y = config.screen_y;
      // Set aspect ratio for future use
      aspect_ratio = screen_size.x / screen_size.y;
      // Target updates from config (FPS)
      target_frames_per_second = config.target_frames_per_second;

      calculated_frame_delay = 1000.0f / target_frames_per_second;

      target_fixed_updates_per_second = config.target_fixed_updates_per_second;
      fixed_update_time = 1000.0 / target_fixed_updates_per_second;

      // Game Related
      cell_types_used = config.cell_types_used;

      world_size = IVec2(config.world_size_x, config.world_size_y);
   };
};
