-- premake5.lua
workspace "Magnet"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "Magnet-Editor"

   -- Workspace-wide build options for MSVC
   filter "system:windows"
      buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus" }

OutputDir = "%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"

group "Core"
	include "Magnet-Core/Build-Core.lua"
group ""

include "Magner-Editor/Build-Editor.lua"