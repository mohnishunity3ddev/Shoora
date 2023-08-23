#if !defined(PNG_LOADER_H)

#include "defines.h"
#include "platform/platform.h"
#include "image_loader.h"

shoora_image_data LoadPNG(const char *Filename);
void FreePng(shoora_image_data *ImageData);

#define PNG_LOADER_H
#endif // PNG_LOADER_H