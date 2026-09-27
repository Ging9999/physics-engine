#include "scene/SceneObject.h"

SceneObject::SceneObject(Mesh* mesh, Vec3 position, Vec3 color,
                          Collider collider, float mass, bool isStatic,
                          Material mat, unsigned int tex, const std::string& tag)
    : mesh(mesh), color(color), collider(collider),
      material(mat), textureID(tex), meshTag(tag) {
    rigidBody.position   = position;
    rigidBody.mass       = mass;
    rigidBody.isStatic   = isStatic;
    rigidBody.restitution = mat.restitution;
}

Mat4 SceneObject::getModelMatrix() const {
    return Mat4::translate(rigidBody.position)
         * Mat4::rotateY(rigidBody.rotation.y)
         * Mat4::rotateX(rigidBody.rotation.x)
         * Mat4::rotateZ(rigidBody.rotation.z)
         * Mat4::scale(scale);
}
