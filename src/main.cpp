#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <cmath>
#include <string>
#include <vector>

#include "core/Timer.h"
#include "math/Vec3.h"
#include "math/Mat4.h"
#include "renderer/Shader.h"
#include "renderer/Mesh.h"
#include "renderer/Camera.h"
#include "renderer/Raycaster.h"
#include "renderer/DebugRenderer.h"
#include "renderer/CrosshairRenderer.h"
#include "renderer/ProceduralTextures.h"
#include "renderer/ParticleSystem.h"
#include "renderer/Skybox.h"
#include "renderer/BitmapFont.h"
#include "renderer/Material.h"
#include "renderer/HeightFieldWater.h"
#include "physics/RigidBody.h"
#include "physics/Collider.h"
#include "physics/PhysicsWorld.h"
#include "scene/SceneObject.h"
#include "scene/Scene.h"
#include "scene/SceneSaver.h"

static const float kPI  = 3.14159265f;
static const float kDeg = kPI / 180.0f;

// Window
static int g_width  = 1280;
static int g_height = 720;

// Camera
static Camera g_camera({0.0f, 5.0f, 15.0f}, -90.0f, -15.0f);
static float  g_lastX      = 640.0f;
static float  g_lastY      = 360.0f;
static bool   g_firstMouse = true;

// Light
static Vec3 g_lightPos = {5.0f, 10.0f, 5.0f};

// Scene / Physics
static Scene        g_scene;
static PhysicsWorld g_physics;

// Water
static HeightFieldWater g_water(64, 64, 20.0f, 20.0f, 2.0f);

// Modes
enum class BuildShape { CUBE, SPHERE };
static bool       g_buildMode  = false;
static BuildShape g_buildShape = BuildShape::CUBE;
static Vec3       g_previewPos = {};
static bool       g_previewHit = false;

// Physics gun
static int   g_grabIndex = -1;
static float g_grabDist  = 5.0f;

// Shadows
static bool         g_shadowsEnabled = true;
static unsigned int g_shadowFBO      = 0;
static unsigned int g_shadowMap      = 0;
static const int    SHADOW_RES       = 2048;

// Meshes
static Mesh g_cubeMesh;
static Mesh g_sphereMesh;
static Mesh g_planeMesh;

// Textures
static unsigned int g_texChecker = 0;
static unsigned int g_texBrick   = 0;
static unsigned int g_texWood    = 0;
static unsigned int g_texMetal   = 0;

// Key edge triggers
static bool g_prevB   = false;
static bool g_prevH   = false;
static bool g_prevX   = false;
static bool g_prevE   = false;
static bool g_prevR   = false;
static bool g_prevF   = false;
static bool g_prevG   = false;
static bool g_prevLMB = false;
static bool g_prevRMB = false;
static bool g_prevCS  = false;
static bool g_prevCL  = false;

// Callbacks
static void framebufferSizeCB(GLFWwindow*, int w, int h) {
    g_width = w; g_height = h;
    glViewport(0, 0, w, h);
}
static void mouseCB(GLFWwindow*, double xp, double yp) {
    if (g_firstMouse) { g_lastX = (float)xp; g_lastY = (float)yp; g_firstMouse = false; }
    float dx =  (float)xp - g_lastX;
    float dy = g_lastY - (float)yp;
    g_lastX = (float)xp; g_lastY = (float)yp;
    g_camera.processMouse(dx, dy);
}
static void scrollCB(GLFWwindow*, double, double y) {
    if (g_grabIndex >= 0)
        g_grabDist = std::max(1.0f, std::min(20.0f, g_grabDist + (float)y * 0.5f));
}

// Helpers
static void addFloor() {
    SceneObject obj(&g_planeMesh, {0.0f, -0.25f, 0.0f}, {0.55f, 0.55f, 0.55f},
                    Collider::createAABB({10.0f, 0.25f, 10.0f}),
                    0.0f, true, Material::concrete(), g_texChecker, "plane");
    obj.scale = {10.0f, 0.25f, 10.0f};
    g_scene.addObject(std::move(obj));
}

static void spawnCube(Vec3 pos, Vec3 col, Material mat, unsigned int tex) {
    g_scene.addObject(SceneObject(&g_cubeMesh, pos, col,
        Collider::createAABB({0.5f, 0.5f, 0.5f}), 1.0f, false, mat, tex, "cube"));
}

static void spawnSphere(Vec3 pos, Vec3 col, Material mat, unsigned int tex) {
    g_scene.addObject(SceneObject(&g_sphereMesh, pos, col,
        Collider::createSphere(0.5f), 1.0f, false, mat, tex, "sphere"));
}

// Shadow map
static void initShadowMap() {
    glGenFramebuffers(1, &g_shadowFBO);
    glGenTextures(1, &g_shadowMap);
    glBindTexture(GL_TEXTURE_2D, g_shadowMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 SHADOW_RES, SHADOW_RES, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float border[] = {1,1,1,1};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
    glBindFramebuffer(GL_FRAMEBUFFER, g_shadowFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, g_shadowMap, 0);
    glDrawBuffer(GL_NONE); glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static Mat4 lightSpaceMatrix() {
    float sz = 20.0f;
    Mat4 proj = Mat4::ortho(-sz, sz, -sz, sz, 1.0f, 60.0f);
    Vec3 up   = (std::abs(g_lightPos.y) > std::abs(g_lightPos.x)) ? Vec3{1,0,0} : Vec3{0,1,0};
    return proj * Mat4::lookAt(g_lightPos, {0,0,0}, up);
}

// Scene rendering
static void renderScene(Shader& sh, bool shadowPass) {
    for (auto& obj : g_scene.objects) {
        sh.setMat4("model", obj.getModelMatrix());
        if (!shadowPass) {
            sh.setVec3("objectColor", obj.color);
            sh.setMaterial(obj.material);
            sh.setInt("useTexture", obj.textureID ? 1 : 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, obj.textureID ? obj.textureID : 0);
        }
        obj.mesh->draw();
    }
}

// Raycasting helpers
static Ray centerRay() {
    return Raycaster::fromCamera(g_camera.getPosition(), g_camera.getFront());
}

static float hitObject(const Ray& ray, int i) {
    auto& obj = g_scene.objects[i];
    if (obj.collider.type == ColliderType::AABB)
        return Raycaster::intersectAABB(ray, obj.rigidBody.position, obj.collider.aabb.halfExtents);
    return Raycaster::intersectSphere(ray, obj.rigidBody.position, obj.collider.sphere.radius);
}

// Build preview
static void updatePreview() {
    g_previewHit = false;
    if (!g_buildMode) return;
    Ray ray = centerRay();
    if (ray.direction.y < -0.01f) {
        float t = (2.0f - ray.origin.y) / ray.direction.y;
        if (t > 1.0f && t < 30.0f) {
            g_previewPos = {ray.origin.x + t * ray.direction.x, 2.5f,
                            ray.origin.z + t * ray.direction.z};
            g_previewHit = true;
        }
    }
}

// Physics gun
static void applyGrabForce() {
    if (g_grabIndex < 0 || g_grabIndex >= (int)g_scene.objects.size()) return;
    RigidBody& rb = g_scene.objects[g_grabIndex].rigidBody;
    Vec3 target = g_camera.getPosition() + g_camera.getFront() * g_grabDist;
    Vec3 disp   = target - rb.position;
    rb.applyForce({disp.x*150.0f - rb.velocity.x*10.0f,
                   disp.y*150.0f - rb.velocity.y*10.0f,
                   disp.z*150.0f - rb.velocity.z*10.0f});
}

// Input
static void processInput(GLFWwindow* win, float dt) {
    if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(win, true);

    g_camera.processKeyboard(win, dt);

    // Arrow / PgUp / PgDn: move light
    float ls = 5.0f * dt;
    if (glfwGetKey(win, GLFW_KEY_LEFT)      == GLFW_PRESS) g_lightPos.x -= ls;
    if (glfwGetKey(win, GLFW_KEY_RIGHT)     == GLFW_PRESS) g_lightPos.x += ls;
    if (glfwGetKey(win, GLFW_KEY_UP)        == GLFW_PRESS) g_lightPos.z -= ls;
    if (glfwGetKey(win, GLFW_KEY_DOWN)      == GLFW_PRESS) g_lightPos.z += ls;
    if (glfwGetKey(win, GLFW_KEY_PAGE_UP)   == GLFW_PRESS) g_lightPos.y += ls;
    if (glfwGetKey(win, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) g_lightPos.y -= ls;

    bool ctrl    = (glfwGetKey(win, GLFW_KEY_LEFT_CONTROL)  == GLFW_PRESS ||
                    glfwGetKey(win, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS);
    auto pressed = [&](int key) { return glfwGetKey(win, key) == GLFW_PRESS; };

    // B: build mode
    bool b = pressed(GLFW_KEY_B);
    if (b && !g_prevB) {
        g_buildMode = !g_buildMode; g_previewHit = false;
        g_grabIndex = -1; g_physics.grabbedObjectIndex = -1;
    }
    g_prevB = b;

    // H: toggle shadows
    bool h = pressed(GLFW_KEY_H);
    if (h && !g_prevH) g_shadowsEnabled = !g_shadowsEnabled;
    g_prevH = h;

    // E: spawn cube
    bool e = pressed(GLFW_KEY_E);
    if (e && !g_prevE && !g_buildMode) {
        static int cycle = 0;
        Material     mats[] = {Material::wood(), Material::metal(), Material::rubber(),
                                Material::concrete(), Material::plastic()};
        unsigned int texs[] = {g_texWood, g_texMetal, g_texChecker, g_texBrick, g_texChecker};
        Vec3 p = g_camera.getPosition() + g_camera.getFront() * 4.0f;
        spawnCube(p, {0.8f,0.5f,0.2f}, mats[cycle%5], texs[cycle%5]);
        cycle++;
    }
    g_prevE = e;

    // R: spawn sphere
    bool r = pressed(GLFW_KEY_R);
    if (r && !g_prevR && !g_buildMode) {
        Vec3 p = g_camera.getPosition() + g_camera.getFront() * 4.0f;
        spawnSphere(p, {0.3f,0.6f,0.9f}, Material::rubber(), g_texChecker);
    }
    g_prevR = r;

    // F: cycle build shape
    bool f = pressed(GLFW_KEY_F);
    if (f && !g_prevF && g_buildMode)
        g_buildShape = (g_buildShape == BuildShape::CUBE) ? BuildShape::SPHERE : BuildShape::CUBE;
    g_prevF = f;

    // X: delete aimed object (build mode)
    bool x = pressed(GLFW_KEY_X);
    if (x && !g_prevX && g_buildMode) {
        Ray ray = centerRay(); int best = -1; float bestT = 1e9f;
        for (int i = 0; i < (int)g_scene.objects.size(); i++) {
            if (g_scene.objects[i].rigidBody.isStatic) continue;
            float t = hitObject(ray, i);
            if (t > 0 && t < bestT) { bestT = t; best = i; }
        }
        if (best >= 0) {
            g_scene.objects.erase(g_scene.objects.begin() + best);
            if (g_grabIndex == best)     { g_grabIndex = -1; g_physics.grabbedObjectIndex = -1; }
            else if (g_grabIndex > best) { g_grabIndex--; g_physics.grabbedObjectIndex = g_grabIndex; }
        }
    }
    g_prevX = x;

    // G: splash the water surface where you're aiming
    bool g = pressed(GLFW_KEY_G);
    if (g && !g_prevG) {
        Ray   ray = centerRay();
        float waterY = 2.0f;
        if (std::abs(ray.direction.y) > 0.01f) {
            float t = (waterY - ray.origin.y) / ray.direction.y;
            if (t > 0.5f && t < 40.0f) {
                Vec3 hit = ray.origin + ray.direction * t;
                g_water.disturb(hit.x, hit.z, 1.2f, 2.0f);
            }
        }
    }
    g_prevG = g;

    // Ctrl+S: save
    bool cs = ctrl && pressed(GLFW_KEY_S);
    if (cs && !g_prevCS) {
        SceneSaver::save("scenes/quicksave.txt", g_scene, g_lightPos);
        printf("Scene saved.\n");
    }
    g_prevCS = cs;

    // Ctrl+L: load
    bool cl = ctrl && pressed(GLFW_KEY_L);
    if (cl && !g_prevCL) {
        g_grabIndex = -1; g_physics.grabbedObjectIndex = -1;
        Vec3 ll;
        if (SceneSaver::load("scenes/quicksave.txt", g_scene, ll,
                             g_cubeMesh, g_sphereMesh, g_planeMesh)) {
            g_lightPos = ll; printf("Scene loaded.\n");
        } else { printf("Load failed.\n"); }
    }
    g_prevCL = cl;
}

static void processMouseButtons(GLFWwindow* win) {
    bool lmb = (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT)  == GLFW_PRESS);
    bool rmb = (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);

    if (!g_buildMode) {
        if (lmb && !g_prevLMB) {
            Ray ray = centerRay(); int best = -1; float bestT = 1e9f;
            for (int i = 0; i < (int)g_scene.objects.size(); i++) {
                if (g_scene.objects[i].rigidBody.isStatic) continue;
                float t = hitObject(ray, i);
                if (t > 0 && t < bestT) { bestT = t; best = i; }
            }
            if (best >= 0) { g_grabIndex = best; g_grabDist = bestT; g_physics.grabbedObjectIndex = best; }
        }
        if (!lmb) { g_grabIndex = -1; g_physics.grabbedObjectIndex = -1; }
        if (rmb && !g_prevRMB && g_grabIndex >= 0) {
            Vec3 f = g_camera.getFront();
            g_scene.objects[g_grabIndex].rigidBody.velocity = {f.x*15.0f, f.y*15.0f, f.z*15.0f};
            g_grabIndex = -1; g_physics.grabbedObjectIndex = -1;
        }
    } else {
        if (lmb && !g_prevLMB && g_previewHit) {
            if (g_buildShape == BuildShape::CUBE)
                spawnCube(g_previewPos, {0.7f,0.4f,0.3f}, Material::wood(), g_texWood);
            else
                spawnSphere(g_previewPos, {0.3f,0.6f,0.8f}, Material::rubber(), g_texChecker);
        }
    }
    g_prevLMB = lmb;
    g_prevRMB = rmb;
}

// Main
int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* win = glfwCreateWindow(g_width, g_height, "Physics Engine", nullptr, nullptr);
    if (!win) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(win);
    glfwSetFramebufferSizeCallback(win, framebufferSizeCB);
    glfwSetCursorPosCallback(win, mouseCB);
    glfwSetScrollCallback(win, scrollCB);
    glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Shaders
    Shader phong, shadow, flat, crosshairSh, particle, skyboxSh, fontSh, waterSh;
    phong.load      ("shaders/phong.vert",      "shaders/phong.frag");
    shadow.load     ("shaders/shadow.vert",     "shaders/shadow.frag");
    flat.load       ("shaders/flat.vert",       "shaders/flat.frag");
    crosshairSh.load("shaders/crosshair.vert",  "shaders/crosshair.frag");
    particle.load   ("shaders/particle.vert",   "shaders/particle.frag");
    skyboxSh.load   ("shaders/skybox.vert",     "shaders/skybox.frag");
    fontSh.load     ("shaders/font.vert",       "shaders/font.frag");
    waterSh.load    ("shaders/water.vert",      "shaders/water.frag");

    // Meshes
    g_cubeMesh   = Mesh::createCube();
    g_sphereMesh = Mesh::createSphere(16, 16, 0.5f);
    g_planeMesh  = Mesh::createPlane(20.0f);

    // Textures
    g_texChecker = ProceduralTextures::createCheckerboard(256, 8);
    g_texBrick   = ProceduralTextures::createBrick(256);
    g_texWood    = ProceduralTextures::createWood(256);
    g_texMetal   = ProceduralTextures::createMetal(256);

    // Shadow map
    initShadowMap();

    // Subsystems
    ParticleSystem particles; particles.init();
    Skybox         skybox;    skybox.initProcedural();
    BitmapFont     font;      font.init();
    DebugRenderer  debug;     debug.init();
    CrosshairRenderer crosshair; crosshair.init();

    g_water.init();

    // Default scene
    addFloor();
    spawnCube  ({ 0.0f, 5.0f, 5.0f}, {0.8f,0.4f,0.2f}, Material::wood(),   g_texWood);
    spawnCube  ({ 0.5f, 7.0f, 5.0f}, {0.4f,0.4f,0.8f}, Material::metal(),  g_texMetal);
    spawnSphere({-0.5f, 9.0f, 5.0f}, {0.3f,0.8f,0.3f}, Material::rubber(), g_texChecker);

    phong.use();
    phong.setInt("textureSampler", 0);
    phong.setInt("shadowMap",      1);

    Timer timer;
    float fpsAccum = 0.0f; int fpsCnt = 0; float fpsShow = 0.0f;

    while (!glfwWindowShouldClose(win)) {
        timer.update();
        float dt = timer.getDeltaTime();
        fpsAccum += dt; fpsCnt++;
        if (fpsAccum >= 0.5f) {
            fpsShow  = (float)fpsCnt / fpsAccum;
            fpsAccum = 0.0f; fpsCnt = 0;
        }

        glfwPollEvents();
        processInput(win, dt);
        processMouseButtons(win);
        updatePreview();

        // Fixed timestep: physics + water
        while (timer.shouldStepPhysics()) {
            float fdт = timer.getFixedTimeStep();
            applyGrabForce();
            g_water.applyBuoyancy(g_scene.objects);
            g_physics.step(fdт, g_scene.objects);
            g_water.step(fdт);
            g_water.handleObjectSplash(g_scene.objects);
            for (auto& ev : g_physics.collisionEvents) {
                Vec3 blended = (ev.colorA + ev.colorB) * 0.5f;
                particles.emit(ev.position, blended, (int)(ev.intensity * 3.0f + 2.0f));
            }
        }
        particles.update(dt);

        // Shadow pass
        Mat4 lsm = lightSpaceMatrix();
        if (g_shadowsEnabled) {
            glViewport(0, 0, SHADOW_RES, SHADOW_RES);
            glBindFramebuffer(GL_FRAMEBUFFER, g_shadowFBO);
            glClear(GL_DEPTH_BUFFER_BIT);
            shadow.use();
            shadow.setMat4("lightSpaceMatrix", lsm);
            renderScene(shadow, true);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, g_width, g_height);
        }

        // Main render pass
        glClearColor(0.1f, 0.12f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float aspect = (float)g_width / (float)g_height;
        Mat4  view   = g_camera.getViewMatrix();
        Mat4  proj   = Mat4::perspective(60.0f * kDeg, aspect, 0.1f, 200.0f);

        // Skybox
        skybox.draw(view, proj, skyboxSh);

        // Opaque scene geometry
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, g_shadowMap);
        phong.use();
        phong.setMat4("view",             view);
        phong.setMat4("projection",       proj);
        phong.setMat4("lightSpaceMatrix", lsm);
        phong.setVec3("lightPos",         g_lightPos);
        phong.setVec3("lightColor",       {1.0f, 1.0f, 1.0f});
        phong.setVec3("viewPos",          g_camera.getPosition());
        phong.setInt ("lightingMode",     0);
        phong.setInt ("shadowsEnabled",   g_shadowsEnabled ? 1 : 0);
        renderScene(phong, false);

        // Build preview (translucent)
        if (g_buildMode && g_previewHit) {
            flat.use();
            flat.setMat4("view",       view);
            flat.setMat4("projection", proj);
            flat.setVec3("objectColor", g_buildShape == BuildShape::CUBE ?
                          Vec3{0.8f,0.6f,0.2f} : Vec3{0.2f,0.6f,0.8f});
            flat.setFloat("alpha", 0.4f);
            Mat4 pm = Mat4::translate(g_previewPos) * Mat4::scale({0.5f,0.5f,0.5f});
            flat.setMat4("model", pm);
            ((g_buildShape == BuildShape::CUBE) ? g_cubeMesh : g_sphereMesh).draw();
        }

        // Collision spark particles
        particles.draw(particle, view, proj);

        // Water surface (transparent, drawn after all opaque geometry)
        g_water.draw(waterSh, view, proj, g_lightPos, g_camera.getPosition(),
                     (float)glfwGetTime());

        // Crosshair
        Vec3 chCol = g_grabIndex >= 0 ? Vec3{0.2f,1.0f,0.2f} :
                     g_buildMode       ? Vec3{1.0f,0.8f,0.2f} :
                                          Vec3{1.0f,1.0f,1.0f};
        crosshair.draw(crosshairSh, chCol);

        // HUD
        font.setProjection(fontSh, g_width, g_height);
        fontSh.use();
        fontSh.setInt("fontAtlas", 0);

        float row = (float)g_height - 20.0f;
        char  buf[128];

        snprintf(buf, sizeof(buf), "FPS: %.0f", fpsShow);
        font.drawText(fontSh, buf, 10.0f, row, 1.0f, {1,1,1}); row -= 16.0f;

        snprintf(buf, sizeof(buf), "Objects: %d", (int)g_scene.objects.size());
        font.drawText(fontSh, buf, 10.0f, row, 1.0f, {1,1,1}); row -= 16.0f;

        font.drawText(fontSh, g_buildMode ? "BUILD" : "PLAY", 10.0f, row, 1.0f,
                      g_buildMode ? Vec3{1.0f,0.8f,0.2f} : Vec3{0.5f,1.0f,0.5f});
        row -= 16.0f;

        if (g_buildMode) {
            font.drawText(fontSh, g_buildShape == BuildShape::CUBE ? "Shape: CUBE" : "Shape: SPHERE",
                          10.0f, row, 1.0f, {0.8f,0.8f,0.8f}); row -= 16.0f;
        }

        snprintf(buf, sizeof(buf), "Shadows: %s", g_shadowsEnabled ? "ON" : "OFF");
        font.drawText(fontSh, buf, 10.0f, row, 1.0f, {0.8f,0.8f,1.0f}); row -= 16.0f;

        if (g_grabIndex >= 0)
            font.drawText(fontSh, "GRABBED", 10.0f, row, 1.0f, {0.2f,1.0f,0.2f});

        font.drawText(fontSh,
            "WASD:move  E/R:spawn  B:build  G:splash  H:shadows  Ctrl+S/L:save/load",
            10.0f, 10.0f, 1.0f, {0.5f,0.5f,0.5f});

        glfwSwapBuffers(win);
    }

    glDeleteFramebuffers(1, &g_shadowFBO);
    glDeleteTextures(1, &g_shadowMap);
    glDeleteTextures(1, &g_texChecker);
    glDeleteTextures(1, &g_texBrick);
    glDeleteTextures(1, &g_texWood);
    glDeleteTextures(1, &g_texMetal);
    glfwTerminate();
    return 0;
}
