cmake_minimum_required(VERSION 3.20)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
project(PhysicsEngine VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Dependencies via FetchContent ──
include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.3.8
)
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        0.9.9.8
)
FetchContent_MakeAvailable(glm)

# ── GLAD (OpenGL loader) ──
add_library(glad STATIC thirdparty/glad/src/glad.c)
target_include_directories(glad PUBLIC thirdparty/glad/include)

# ── Project sources ──
set(SOURCES
    src/main.cpp
    src/stb_image_impl.cpp
    src/core/Timer.cpp
    src/math/Vec3.cpp
    src/math/Mat4.cpp
    src/renderer/Shader.cpp
    src/renderer/Mesh.cpp
    src/renderer/Camera.cpp
    src/renderer/ObjLoader.cpp
    src/renderer/Raycaster.cpp
    src/renderer/DebugRenderer.cpp
    src/renderer/CrosshairRenderer.cpp
    src/renderer/TextureLoader.cpp
    src/renderer/ProceduralTextures.cpp
    src/renderer/ParticleSystem.cpp
    src/renderer/Skybox.cpp
    src/renderer/BitmapFont.cpp
    src/physics/RigidBody.cpp
    src/physics/Collider.cpp
    src/physics/CollisionDetection.cpp
    src/physics/CollisionResponse.cpp
    src/physics/PhysicsWorld.cpp
    src/renderer/HeightFieldWater.cpp
    src/scene/SceneObject.cpp
    src/scene/Scene.cpp
    src/scene/SceneSaver.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/thirdparty/stb
)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw glad glm::glm)

file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/assets  DESTINATION ${CMAKE_BINARY_DIR})
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/scenes)
