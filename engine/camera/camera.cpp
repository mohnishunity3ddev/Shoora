#include "camera.h"
#include <float.h>

void
SetupCamera(shoora_camera *Camera, shoora_projection Type, f32 Near, f32 Far, f32 Aspect, f32 Height, f32 halfFOV,
            Shu::vec3f Pos, Shu::vec3f GlobalUp)
{
    Camera->Pos = Pos;
    Camera->GlobalUp = GlobalUp;

    Camera->Type = Type;
    Camera->Near = Near;
    Camera->Far = Far;
    Camera->Aspect = Aspect;
    Camera->Height = Height;
    Camera->halfFOV = halfFOV;

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

Shu::vec2f
shoora_camera::GetBounds()
{
    Shu::vec2f bounds = {this->Aspect*this->Height, this->Height};

    return bounds;
}

Shu::rect2d
shoora_camera::GetRect()
{
    Shu::rect2d Result;

    Result.x = this->Pos.x;
    Result.y = this->Pos.y;
    Result.width = this->Aspect * this->Height;
    Result.height = this->Height;

    return Result;
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
    if(this->Type == PROJECTION_PERSPECTIVE)
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
            if(MoveDirection.SqMagnitude() - FLT_EPSILON > 0.0f)
            {
                this->Pos += MoveDirection*MovementSpeed*CameraInput->DeltaTime;
            }
        }
    }
    else if(this->Type == PROJECTION_ORTHOGRAPHIC)
    {
        Shu::vec3f MoveDirection = Shu::Vec3f(0.0f);
        if(CameraInput->MoveForwards) { MoveDirection.y = 1.0f; }
        if(CameraInput->MoveBackwards) { MoveDirection.y = -1.0f; }
        if(CameraInput->MoveLeft) { MoveDirection.x = -1.0f; }
        if(CameraInput->MoveRight) { MoveDirection.x = 1.0f; }

        b32 Move = ((MoveDirection.SqMagnitude() - FLT_EPSILON) > 0.0f);
        if (Move)
        {
            f32 MovementSpeed = this->MovementSpeed*SHU_PIXELS_PER_METER;
            if (CameraInput->MoveFaster)
            {
                MovementSpeed *= 3.0f;
            }

            this->Pos += MoveDirection*MovementSpeed*CameraInput->DeltaTime;
        }
    }
}

Shu::mat4f shoora_camera::
GetViewMatrix(Shu::mat4f &M)
{
    if(this->Type == PROJECTION_PERSPECTIVE)
    {
        M = Shu::LookAt(this->Pos, this->Pos + this->Front, this->GlobalUp, M);
    }
    else if(this->Type == PROJECTION_ORTHOGRAPHIC)
    {
        M = Shu::Translate(M, Shu::Vec3f(-this->Pos.x, -this->Pos.y, 0.0f));
    }
    return M;
}

Shu::mat4f
shoora_camera::GetProjectionMatrix()
{
    Shu::mat4f projection;

    if(this->Type == PROJECTION_ORTHOGRAPHIC)
    {
        f32 Width = this->Aspect * this->Height;
        projection = Shu::Orthographic(Width, this->Height, this->Near, this->Far);
    }
    else if (this->Type == PROJECTION_PERSPECTIVE)
    {
        projection = Shu::Perspective(this->halfFOV, this->Aspect, this->Near, this->Far);
    }

    return projection;
}

void
shoora_camera::UpdateWindowSize(const Shu::vec2f &windowSize)
{
    this->Height = windowSize.y;
    this->Aspect = windowSize.x / windowSize.y;
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