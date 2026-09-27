#pragma once
#include "scene/SceneObject.h"
#include <vector>

class Scene {
public:
    std::vector<SceneObject> objects;

    void addObject(SceneObject obj);
    void clear();
};
