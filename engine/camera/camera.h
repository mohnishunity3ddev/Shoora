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

#define SHU_DEFAULT_FRONT shu::Vec3f(0, 0, 1)
#define SHU_DEFAULT_RIGHT shu::Vec3f(1, 0, 0)
#define SHU_DEFAULT_UP shu::Vec3f(0, 1, 0)

enum shoora_projection
{
    PROJECTION_ORTHOGRAPHIC,
    PROJECTION_PERSPECTIVE,
    PROJECTION_MAX_COUNT
};

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
    shu::vec3f Pos;
    shu::vec3f Front;
    shu::vec3f Right;
    shu::vec3f Up;
    shu::vec3f GlobalUp;

    f32 halfFOV;
    shoora_projection Type;
    f32 Near, Far;
    f32 Aspect;
    f32 Height;

    f32 Yaw;
    f32 Pitch;
    f32 MovementSpeed;
    f32 MouseSensitivity;
    f32 Zoom;

    void UpdateCameraVectors();

    shu::mat4f GetViewMatrix(shu::mat4f &M);
    shu::mat4f GetProjectionMatrix();
    shu::vec2f GetBounds();
    void UpdateWindowSize(const shu::vec2f &windowSize);
    shu::rect2d GetRect();

#if SHU_USE_GLM
    glm::mat4 GetViewMatrix(glm::mat4 &M);
#endif
    void HandleInput(const shoora_camera_input *CameraInput);
};

SHU_EXPORT void SetupCamera(shoora_camera *Camera, shoora_projection Type, f32 Near, f32 Far, f32 Aspect,
                            f32 Height, f32 halfFOV, shu::vec3f Pos = shu::Vec3f(0.0f),
                            shu::vec3f GlobalUp = shu::Vec3f(0.0f, 1.0f, 0.0f));

#define CAMERA_H
#endif // CAMERA_H