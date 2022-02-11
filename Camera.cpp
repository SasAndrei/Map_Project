#include "Camera.hpp"

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        //TODO
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;
        this->cameraFrontDirection = glm::normalize(cameraPosition - cameraTarget);
        this->cameraRightDirection = glm::normalize(glm::cross(cameraUp, this->cameraFrontDirection));
        this->yaw = -90.0f;
        this->pitch = 1.0f;
        this->fov = 23.0f;

    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        //TODO

        return glm::lookAt(this->cameraPosition, this->cameraTarget, this->cameraUpDirection);
    }

    void Camera::setCamera(glm::vec3 pos, glm::vec3 trg) {
        this->cameraPosition = pos;
        this->cameraTarget = trg;
        this->yaw = -90.0f;
        this->pitch = 50.0f;
        this->fov = 23.0f;
    }

    glm::vec3 Camera::getPosition() {
        return (this->cameraPosition);
    }

    glm::vec3 Camera::getTarget() {
        return (this->cameraTarget);
    }

    glm::vec3 Camera::getFront() {
        return (this->cameraFrontDirection);
    }

    float Camera::getFov() {
        return this->fov;
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        //TODO
        if (direction == MOVE_FORWARD) {
            this->cameraPosition -= speed * this->cameraFrontDirection;
            this->cameraTarget -= speed * this->cameraFrontDirection;
        }
        if (direction == MOVE_BACKWARD) {
            this->cameraPosition += speed * this->cameraFrontDirection;
            this->cameraTarget += speed * this->cameraFrontDirection;
        }
        if (direction == MOVE_LEFT) {
            this->cameraPosition -= speed * this->cameraRightDirection;
            this->cameraTarget -= speed * this->cameraRightDirection;
        }
        if (direction == MOVE_RIGHT) {
            this->cameraPosition += speed * this->cameraRightDirection;
            this->cameraTarget += speed * this->cameraRightDirection;
        }
    }


    void Camera::zoom(MOVE_DIRECTION direction, float speed) {
        //TODO
        if (direction == MOVE_FORWARD) {
            fov -= speed;
            if (fov < 1.0f)
                fov = 1.0f;
        }
        if (direction == MOVE_BACKWARD) {
            this->fov += speed;
            if (fov > 45.0f)
                fov = 45.0f;
        }
    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        glm::vec3 direction;

        this->yaw += yaw;
        this->pitch += pitch;

       if (this->pitch > 89.0f)
            this->pitch = 89.0f;
        if (this->pitch < -89.0f)
            this->pitch = -89.0f;

        direction.x = cos(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
        direction.y = sin(glm::radians(this->pitch));
        direction.z = sin(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));

        this->cameraTarget = this->cameraPosition + direction;

        this->cameraFrontDirection = glm::normalize(cameraPosition - cameraTarget);

        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraUpDirection, this->cameraFrontDirection));

    }
}