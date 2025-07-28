#include "Camera.hpp"
#include "glm/gtc/matrix_transform.hpp"

    Camera::Camera(){
        //Assume placed at origin
        mEye = glm::vec3(0.0f, 0.0f, 0.0f);
        mViewDirection = glm::vec3(0.0f, 0.0f, -1.0f);
        mUpVector = glm::vec3(0.0f, 1.0f, 0.0f);
    }
    glm::mat4 Camera::GetViewMatrix() const{
        return glm::lookAt(mEye, mEye + mViewDirection, mUpVector);
    }

    void Camera::MoveForward(float speed){
        mEye += mViewDirection * speed;
    }
    void Camera::MoveBackward(float speed){
        mEye -= mViewDirection * speed;
    }
    void Camera::MoveLeft(float speed){
        glm::vec3 rightVector = glm::cross(mViewDirection, mUpVector);
        mEye -= rightVector * speed;
    }
    void Camera::MoveRight(float speed){
        glm::vec3 rightVector = glm::cross(mViewDirection, mUpVector);
        mEye += rightVector * speed;
    }