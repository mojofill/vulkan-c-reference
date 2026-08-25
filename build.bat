@echo off

mkdir src/spvs 2>nul

echo Compiling shaders ...
glslc src\shaders\shader.vert -o src\spvs\vert.spv
glslc src\shaders\shader.frag -o src\spvs\frag.spv
glslc src\shaders\shader.comp -o src\spvs\comp.spv

echo Configuring CMake...
cmake -S . -B build ^
  -G "MinGW Makefiles" ^
  -DCMAKE_C_COMPILER=C:/msys64/ucrt64/bin/gcc.exe ^
  -DCMAKE_MAKE_PROGRAM=C:/msys64/ucrt64/bin/mingw32-make.exe ^
  -DCMAKE_BUILD_TYPE=Release ^
  -Dglfw3_DIR=C:/glfw-mingw/lib/cmake/glfw3

echo Building...
cmake --build build

echo Running...
build\vk_app.exe
