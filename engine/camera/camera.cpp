#include "camera.h"

void
SetupCamera(shoora_camera *Camera, Shu::vec3f Pos, Shu::vec3f GlobalUp)
{
    Camera->Pos = Pos;
    Camera->GlobalUp = GlobalUp;

    Camera->Front = SHU_DEFAULT_FRONT;
    Camera->Right = SHU_DEFAULT_RIGHT;
    Camera->Up = SHU_DEFAULT_UP;
    Camera->Yaw = SHU_DEFAULT_YAW;
    Camera->Pitch = SHU_DEFAULT_PITCH;
    Camera->MovementSpeed = SHU_DEFAULT_SPEED;
    Camera->MouseSensitivity = SHU_DEFAULT_SENSITIVITY;
    Camera->Zoom = SHU_DEFAULT_ZOOM;

    Camera->UpdateCameraVectors();
}

void shoora_camera::
UpdateCameraVectors()
{
    Shu::vec3f Front;
    Front.x = Shu::Cos(Yaw) * Shu::Cos(Pitch);
    Front.y = Shu::Sin(Pitch);
    Front.z = Shu::Sin(Yaw) * Shu::Cos(Pitch);

    this->Front = Shu::Normalize(Front);

    this->Right = Shu::Normalize(Shu::Cross(this->GlobalUp, this->Front));
    this->Up = Shu::Cross(this->Front, this->Right);
}

void shoora_camera::
HandleInput(const shoora_camera_input *CameraInput)
{
    f32 Yaw = this->Yaw - this->MouseSensitivity*CameraInput->MouseDeltaX;
    if(Yaw >= 360.0f)
    {
        Yaw = 0.0f;
    }
    else if(Yaw <= 0.0f)
    {
        Yaw = 360.0f;
    }
    this->Yaw = Yaw;

    f32 Pitch = this->Pitch - this->MouseSensitivity*CameraInput->MouseDeltaY;
    if(Pitch >= 89.0f)
    {
        Pitch = 89.0f;
    }
    if(Pitch <= -89.0f)
    {
        Pitch = -89.0f;
    }
    this->Pitch = Pitch;
    this->UpdateCameraVectors();

    Shu::vec3f MoveDirection = Shu::Vec3f(0.0f);
    b32 Move = false;
    if(CameraInput->MoveForwards)
    {
        Move = true;
        MoveDirection += this->Front;
    }
    if(CameraInput->MoveBackwards)
    {
        Move = true;
        MoveDirection -= this->Front;
    }
    if(CameraInput->MoveLeft)
    {
        Move = true;
        MoveDirection -= this->Right;
    }
    if(CameraInput->MoveRight)
    {
        Move = true;
        MoveDirection += this->Right;
    }

    if(Move)
    {
        f32 MovementSpeed = this->MovementSpeed;
        if(CameraInput->MoveFaster)
        {
            MovementSpeed *= 3.0f;
        }

        MoveDirection = Shu::Normalize(MoveDirection);
        this->Pos += MoveDirection * MovementSpeed * CameraInput->DeltaTime;
    }
}

Shu::mat4f shoora_camera::
GetViewMatrix(Shu::mat4f &M)
{
    M = Shu::LookAt(this->Pos, this->Pos + this->Front, this->GlobalUp, M);
    return M;
}

#if SHU_USE_GLM
glm::mat4 shoora_camera::
GetViewMatrix(glm::mat4 &M)
{
    glm::vec3 gPos = glm::vec3(Pos.x, Pos.y, Pos.z);
    glm::vec3 gFront = glm::vec3(Front.x, Front.y, Front.z);
    glm::vec3 gGlobalUp = glm::vec3(GlobalUp.x, GlobalUp.y, GlobalUp.z);

    M = glm::lookAt(gPos, gPos + gFront, gGlobalUp);
    return M;
}
#endif