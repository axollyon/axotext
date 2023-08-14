#ifndef AXOTEXT_H
#define AXOTEXT_H

#include <ultra64.h>

/**
 * The maximum number of characters that can be displayed at once.
 * Increasing this will use more memory.
 */
#define AXOTEXT_BUFFER_SIZE 200

/**
 * This should be the height of the screen as set in your engine's configuration.
 * For HackerSM64, this is a define value named: SCREEN_HEIGHT
 */
#define AXOTEXT_SCREEN_H SCREEN_HEIGHT

/**
 * This should point to a boolean value that determines whether the game is in "widescreen mode".
 * It will scale the width of all text by 0.75x so that its size is consistent in 16:9.
 * Most engines will not have this, so if you're not using HackerSM64, you can safely
 * set this to FALSE. Make sure that you also remove the include and the extern.
 * For HackerSM64, this is named: gConfig.widescreen
 */
#define AXOTEXT_WIDESCREEN gConfig.widescreen
#include "types.h"
extern struct Config gConfig;

/**
 * This should point to your engine's function to convert a segmented address to a virtual address.
 * For HackerSM64, this is named: segmented_to_virtual
 */
#define AXOTEXT_SEG_TO_VIRT segmented_to_virtual
#include "memory.h"

/**
 * This should point to your engine's global display list head.
 * For HackerSM64, this is named: gDisplayListHead
 */
#define AXOTEXT_GDL_HEAD gDisplayListHead
extern Gfx *AXOTEXT_GDL_HEAD;

/**
 * This should point to your engine's function for allocating a display list.
 * This should be in this format: void *AXOTEXT_ALLOC(size_t bytes)
 * For HackerSM64, this is named: alloc_display_list
 */
#define AXOTEXT_ALLOC alloc_display_list
extern void *AXOTEXT_ALLOC(size_t);

extern s32 roundf(f32);

/**
 * Definitions that help Axotext refer to the different texture filtering modes.
 */
typedef enum AxotextFilter {
    AXOTEXT_FILTER_POINT,
    AXOTEXT_FILTER_BILERP,
    AXOTEXT_FILTER_AVERAGE
} AxotextFilter;

/**
 * This struct should be stored alongside your textures and tables in memory.
 * For SM64, it is recommended to put these either in segment 7 (level data), or segment 2 (always loaded).
 */
typedef struct AxotextFont {
    u8 textureWidth;        // Texture width
    u8 textureHeight;       // Texture height
    f32 textureAspect;      // How much to squash/stretch the texture. 1.0f is square, 0.5f is half as wide as it is tall, 2.0f is half as tall as it is wide
    u8 **textureTable;      // Pointer to the texture table
    u8 *kerningTable;       // Pointer to the kerning table
    AxotextFilter filter;   // One of the three filter definitions
} AxotextFont;

/**
 * Text alignment definitions.
 */
typedef enum AxotextAlign {
    AXOTEXT_ALIGN_LEFT,
    AXOTEXT_ALIGN_CENTER,
    AXOTEXT_ALIGN_RIGHT
} AxotextAlign;

/**
 * This struct should be stored in code next to where you call axotext_print.
 */
typedef struct AxotextParams {
    AxotextFont *font;
    f32 fontSize;   // Height in framebuffer pixels
    f32 lineHeight; // Height in framebuffer pixels
	AxotextAlign align;
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} AxotextParams;

extern void axotext_print(f32 x, f32 y, AxotextParams *params, s32 limit, const char *str);
extern void axotext_render(void);

#endif