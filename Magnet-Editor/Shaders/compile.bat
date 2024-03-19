@echo off

..\Thirdparty\bin\vulkan\glslc.exe shader.vert -o shader.vert.spv
..\Thirdparty\bin\vulkan\glslc.exe shader.frag -o shader.frag.spv

pause