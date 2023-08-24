#if !defined(PNG_LOADER_H)

#include "defines.h"
#include "platform/platform.h"
#include "image_loader.h"

#if !SHU_USE_STB
shoora_image_data LoadPNG(const char *Filename);
void FreePng(shoora_image_data *ImageData);
#endif

#define PNG_LOADER_H
#endif // PNG_LOADER_H