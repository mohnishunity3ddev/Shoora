#if !defined(RENDERER_FRONTEND_H)

#define SHU_RENDERER_BACKEND_VULKAN

struct shoora_platform_app_info;
struct shoora_platform_frame_packet;
void InitializeRenderer(shoora_platform_app_info *AppInfo);
void DrawFrame(shoora_platform_frame_packet *FramePacket);
void DestroyRenderer();

#define RENDERER_FRONTEND_H
#endif // RENDERER_H