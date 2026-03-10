#include <freak/scene/Scene.h>
#include <freak/scene/Components.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace freak {

void RebuildWorldMatrix(TransformComponent& t) {
    // TRS: Translation * Rotation * Scale
    glm::mat4 T = glm::translate(glm::mat4(1.0f), t.position);
    glm::mat4 R = glm::mat4_cast(t.rotation);
    glm::mat4 S = glm::scale(glm::mat4(1.0f), t.scale);
    t.worldMatrix = T * R * S;
}

} // namespace freak
