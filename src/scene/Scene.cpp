#include "scene/Scene.h"

void Scene::addObject(SceneObject obj) {
    objects.push_back(std::move(obj));
}

void Scene::clear() {
    objects.clear();
}
