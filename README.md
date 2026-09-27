# Physics Engine - Computer Graphics Coursework

A real-time 3D physics sandbox built with C++ and OpenGL 3.3.

## Prerequisites

- **C++17 compiler** (GCC 9+, Clang 10+, or MSVC 2019+)
- **CMake 3.20+**
- **Git** (for FetchContent to download GLFW and GLM)
- **OpenGL 3.3** capable GPU and drivers

## GLAD Setup (One-Time)

Before building, you need to generate the GLAD loader:

1. Go to https://glad.davemorris.com/
2. Set: Language = **C/C++**, API gl = **3.3**, Profile = **Core**, Generate a Loader = **yes**
3. Click **Generate** and download the zip
4. Extract and copy files into this project:
   - `glad.c` → `thirdparty/glad/src/glad.c`
   - `glad/glad.h` → `thirdparty/glad/include/glad/glad.h`
   - `KHR/khrplatform.h` → `thirdparty/glad/include/KHR/khrplatform.h`

## Building

### CLion (Recommended)
1. Open the project folder in CLion (File → Open → select the PhysicsEngine folder)
2. CLion will detect `CMakeLists.txt` automatically
3. Wait for CMake to configure (it will download GLFW and GLM, first time takes a minute)
4. Click the green Run button (or Shift+F10)

### Command Line
```bash
mkdir build && cd build
cmake ..
cmake --build .
./PhysicsEngine
```

## Controls

- **WASD** - Move camera
- **Mouse** - Look around
- **Escape** - Close window

(More controls will be added as physics features are implemented)

## Project Structure

```
src/
  main.cpp           - Entry point
  core/              - Application loop, timing
  math/              - Custom Vec3, Mat4 (own math library)
  renderer/          - Shader, Mesh, Camera, OBJ loading
  physics/           - RigidBody, Colliders, Collision detection/response
  scene/             - SceneObject, Scene management
shaders/             - GLSL vertex and fragment shaders
assets/models/       - .obj model files
```
