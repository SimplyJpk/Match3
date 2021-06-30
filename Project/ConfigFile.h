#pragma once
#include "ISerializable.h"

struct ConfigFile final : public ISerializable
{
   short screen_x = 720;
   short screen_y = 720;

   float target_frames_per_second = 60.0;
   float target_fixed_updates_per_second = 50.0;

   SaveTypes SaveType() override
   {
      return SaveTypes::Json;
   }

   std::string FilePath() override { return std::string("config"); }

   // Inherited via ISerializable
   virtual void Save(cereal::JSONOutputArchive out_archive) override
   {
      out_archive(CEREAL_NVP(screen_x));
      out_archive(CEREAL_NVP(screen_y));
      out_archive(CEREAL_NVP(target_frames_per_second));
      out_archive(CEREAL_NVP(target_fixed_updates_per_second));
   }

   virtual void Load(cereal::JSONInputArchive in_archive) override
   {
      in_archive(screen_x);
      in_archive(screen_y);
      in_archive(target_frames_per_second);
      in_archive(target_fixed_updates_per_second);
   }
};
