#if !defined(CAMERA_H)

#include "defines.h"
#include "math/math.h"

#if SHU_USE_GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#endif

#define SHU_DEFAULT_YAW 90.0f
#define SHU_DEFAULT_PITCH 0.0f
#define SHU_DEFAULT_ROLL 0.0f

#define SHU_DEFAULT_SPEED 2.5f
#define SHU_DEFAULT_SENSITIVITY 0.33f
#define SHU_DEFAULT_ZOOM 45.0f

#define SHU_DEFAULT_FRONT Shu::Vec3f(0, 0, 1)
#define SHU_DEFAULT_RIGHT Shu::Vec3f(1, 0, 0)
#define SHU_DEFAULT_UP Shu::Vec3f(0, 1, 0)

struct shoora_camera_input
{
    f32 MouseDeltaX;
    f32 MouseDeltaY;

    b8 MoveForwards;
    b8 MoveBackwards;
    b8 MoveLeft;
    b8 MoveRight;
    b32 MoveFaster;

    f32 DeltaTime;
};

struct shoora_camera
{
    Shu::vec3f Pos;
    Shu::vec3f Front;
    Shu::vec3f Right;
    Shu::vec3f Up;
    Shu::vec3f GlobalUp;

    f32 Yaw;
    f32 Pitch;
    f32 MovementSpeed;
    f32 MouseSensitivity;
    f32 Zoom;

    void UpdateCameraVectors();

    Shu::mat4f GetViewMatrix(Shu::mat4f &M);
#if SHU_USE_GLM
    glm::mat4 GetViewMatrix(glm::mat4 &M);
#endif
    void HandleInput(const shoora_camera_input *CameraInput);
};



SHU_EXPORT void SetupCamera(shoora_camera *Camera, Shu::vec3f Pos = Shu::Vec3f(0.0f),
                            Shu::vec3f GlobalUp = Shu::Vec3f(0.0f, 1.0f, 0.0f));


#define CAMERA_H
#endif // CAMERA_H