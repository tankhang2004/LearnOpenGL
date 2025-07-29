#ifndef CAMERA_HPP 
#define CAMERA_HPP
#include "glm/glm.hpp"
class Camera{
    public: 
    
    //Constructor
    Camera();
    glm::mat4 GetViewMatrix() const;
    void MouseLook(int mouseX, int mouseY);
    void MoveForward(float speed);
    void MoveBackward(float speed);
    void MoveLeft(float speed);
    void MoveRight(float speed);
    private: 
        // glm::mat4 mViewMatrix;
        glm::vec3 mEye;
        glm::vec3 mViewDirection;
        glm::vec3 mUpVector;
        glm::vec2 mOldMousePosition;
};
#endif