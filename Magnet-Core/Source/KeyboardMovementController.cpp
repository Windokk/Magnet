#include "KeyboardMovementController.h"

void Magnet::EngineBase::KeyboardMovementController::moveInPlaneXZ(GLFWwindow* window, float dt, Object& object)
{
    glm::vec3 rotate{ 0 };
    rotate.y += static_cast<float>(glfwGetKey(window, keys.lookRight) == GLFW_PRESS);
    rotate.y -= static_cast<float>(glfwGetKey(window, keys.lookLeft) == GLFW_PRESS);
    rotate.x += static_cast<float>(glfwGetKey(window, keys.lookUp) == GLFW_PRESS);
    rotate.x -= static_cast<float>(glfwGetKey(window, keys.lookDown) == GLFW_PRESS);

    if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
        object.transform.rotation += lookSpeed * dt * glm::normalize(rotate);
    }

    // limit pitch values between about +/- 85ish degrees
    object.transform.rotation.x = glm::clamp(object.transform.rotation.x, -1.5f, 1.5f);
    object.transform.rotation.y = glm::mod(object.transform.rotation.y, glm::two_pi<float>());

    float yaw = object.transform.rotation.y;
    const glm::vec3 forwardDir{ sin(yaw), 0.f, cos(yaw) };
    const glm::vec3 rightDir{ forwardDir.z, 0.f, -forwardDir.x };
    const glm::vec3 upDir{ 0.f, -1.f, 0.f };

    glm::vec3 moveDir{ 0.f };
    if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS) moveDir += forwardDir;
    if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) moveDir -= forwardDir;
    if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) moveDir += rightDir;
    if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) moveDir -= rightDir;
    if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) moveDir += upDir;
    if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) moveDir -= upDir;

    if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
        object.transform.location += moveSpeed * dt * glm::normalize(moveDir);
    }
}