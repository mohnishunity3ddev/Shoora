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
HandleInput(f32 XMovedSinceLastFrame, f32 YMovedSinceLastFrame)
{
    f32 Yaw = this->Yaw - this->MouseSensitivity*XMovedSinceLastFrame;
    if(Yaw < 0.0f || Yaw >= 360.0f)
    {
        Yaw = 0.0f;
    }
    this->Yaw = Yaw;

    f32 Pitch = this->Pitch - this->MouseSensitivity*YMovedSinceLastFrame;
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

Shu::mat4f shoora_camera::
GetViewMatrix(Shu::mat4f &M)
{
    M = Shu::LookAt(this->Pos, this->Pos + this->Front, this->GlobalUp, M);
    return M;
}
