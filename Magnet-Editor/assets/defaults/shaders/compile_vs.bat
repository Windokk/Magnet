@echo off

..\..\..\..\Magnet-Core\Source\Third-Party\bin\vulkan\glslc.exe shader.vert -o shader.vert.spv
..\..\..\..\Magnet-Core\Source\Third-Party\bin\vulkan\glslc.exe shader.frag -o shader.frag.spv

pause