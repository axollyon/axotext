#include <ultra64.h>
#include "axotext.h"

typedef struct AxotextChar {
    unsigned char c;
    struct AxotextChar *next;
    f32 x;
    f32 y;
    AxotextFont *font;
    f32 w;
    f32 h;
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} AxotextChar;

AxotextChar axotextBuffer[AXOTEXT_BUFFER_SIZE];
AxotextChar *axotextBufferHeads[AXOTEXT_BUFFER_SIZE];

u16 axotextBufferIndex = 0;
u16 axotextBufferHeadsIndex = 0;


f32 axotext_width(u8 *kerningTable, f32 charWidth, f32 textureWidth, s32 limit, const char *str) {
    f32 w = 0;
    while (limit != 0 && *str != 0 && *str != '\n') {
        unsigned char c = *str;
        str++;
        limit--;
        w += (f32)kerningTable[c] * (charWidth / textureWidth);
    }
    return w;
}

static const Vtx axotext_vertex[] = {
    {{{0, 0, 0}, 0, {0, 1}, {0xff, 0xff, 0xff, 0xff}}},
    {{{1, 0, 0}, 0, {0, 0}, {0xff, 0xff, 0xff, 0xff}}},
    {{{1, 1, 0}, 0, {1, 0}, {0xff, 0xff, 0xff, 0xff}}},
    {{{0, 1, 0}, 0, {1, 1}, {0xff, 0xff, 0xff, 0xff}}},
};

void axotext_add_char(unsigned char c, f32 x, f32 y, AxotextFont *font, f32 w, f32 h, u8 r, u8 g, u8 b, u8 a) {
    if (axotextBufferIndex < AXOTEXT_BUFFER_SIZE) {
        s32 i = 0;
        s32 charPlaced = FALSE;

        axotextBuffer[axotextBufferIndex].c = c;
        axotextBuffer[axotextBufferIndex].x = x;
        axotextBuffer[axotextBufferIndex].y = y;
        axotextBuffer[axotextBufferIndex].font = font; 
        axotextBuffer[axotextBufferIndex].w = w;
        axotextBuffer[axotextBufferIndex].h = h;
        axotextBuffer[axotextBufferIndex].r = r;
        axotextBuffer[axotextBufferIndex].g = g;
        axotextBuffer[axotextBufferIndex].b = b;
        axotextBuffer[axotextBufferIndex].a = a;
        
        for (i = 0; i < AXOTEXT_BUFFER_SIZE && !charPlaced; i++) {
            if (i == axotextBufferHeadsIndex) {
                // Character with the same texture has not already been added:
                // Add the new character to the end of the head array
                axotextBufferHeads[i] = &axotextBuffer[axotextBufferIndex];
                axotextBufferHeadsIndex++;

                charPlaced = TRUE;
            }
            else if (
                axotextBufferHeads[i]->c == c &&
                axotextBufferHeads[i]->font == font
            ) {
                // Character with the same texture has already been added:
                // Get the old head character, replace it in the head array with our new character,
                // then make our new character point to the old head character
                AxotextChar *oldHead = axotextBufferHeads[i];
                axotextBufferHeads[i] = &axotextBuffer[axotextBufferIndex];
                axotextBuffer[axotextBufferIndex].next = oldHead;

                charPlaced = TRUE;
            }
        }

        axotextBufferIndex++;
    }
}

/**
 * Add some text to the printing buffer. The AxotextParams struct should be declared alongside your code before this is called.
 */
void axotext_print(f32 x, f32 y, AxotextParams *params, s32 limit, const char *str) {
    f32 currentX, currentY, charWidth, charHeight;
    AxotextFont *font = AXOTEXT_SEG_TO_VIRT(params->font);
    u8 **textureTable;
    u8 *kerningTable;

    if (font->textureWidth % 2 != 0) {
        return;
    }

    textureTable = AXOTEXT_SEG_TO_VIRT(font->textureTable);
    kerningTable = AXOTEXT_SEG_TO_VIRT(font->kerningTable);
    currentY = y;
    charWidth = AXOTEXT_WIDESCREEN ? params->fontSize * font->textureAspect * 0.75f : params->fontSize * font->textureAspect;
    charHeight = params->fontSize;

newLine:
    switch (params->align) {
        default:
        case AXOTEXT_ALIGN_LEFT:
            currentX = x;
            break;
        case AXOTEXT_ALIGN_CENTER:
            currentX = x - (axotext_width(kerningTable, charWidth, (f32)font->textureWidth, limit, str) / 2);
            break;
        case AXOTEXT_ALIGN_RIGHT:
            currentX = x - axotext_width(kerningTable, charWidth, (f32)font->textureWidth, limit, str);
            break;
    }
    while (limit != 0 && *str != 0) {
        unsigned char c = *str;
        str++;
        limit--;
        switch (c) {
            case '\n': {
                currentY -= params->lineHeight;
                goto newLine;
                break;
            }
            default: {
                const u8 *texture = textureTable[c];
                if (texture != NULL) {
                    axotext_add_char(c, currentX, currentY, font, charWidth, charHeight, params->r, params->g, params->b, params->a);
                }
                currentX += (f32)kerningTable[c] * (charWidth / (f32)font->textureWidth);
                break;
            }
        }
    }
}

void axotext_setup(void) {
    gDPPipeSync(AXOTEXT_GDL_HEAD++);
    gDPSetCombineLERP(
        AXOTEXT_GDL_HEAD++, 
        0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0, 
        0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0
    );
    gSPClearGeometryMode(AXOTEXT_GDL_HEAD++, G_ZBUFFER);
    gDPSetRenderMode(AXOTEXT_GDL_HEAD++, G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2);
}

void axotext_revert(void) {
    gDPPipeSync(AXOTEXT_GDL_HEAD++);
    gSPSetGeometryMode(AXOTEXT_GDL_HEAD++, G_ZBUFFER);
    gDPSetTextureFilter(AXOTEXT_GDL_HEAD++, G_TF_BILERP);
    gDPPipeSync(AXOTEXT_GDL_HEAD++);
    gSPSetGeometryMode(AXOTEXT_GDL_HEAD++, G_LIGHTING);
    gSPClearGeometryMode(AXOTEXT_GDL_HEAD++, G_TEXTURE_GEN);
    gDPSetCombineLERP(
        AXOTEXT_GDL_HEAD++, 
        0, 0, 0, SHADE, 0, 0, 0, ENVIRONMENT, 
        0, 0, 0, SHADE, 0, 0, 0, ENVIRONMENT
    );
    gSPTexture(AXOTEXT_GDL_HEAD++, 65535, 65535, 0, 0, 0);
}

/**
 * Render all text that has been added to the printing buffer, then reset it.
 */
void axotext_render(void) {
    s32 i = 0;
    s32 s = 0;
    s32 t = 0;
    s32 imageW = 0;
    s32 tileW = 0;
    s32 tileH = 0;
    s32 tileSizeW = 0;
    s32 tileSizeH = 0;
    AxotextFont *font = NULL;
    u8 **textureTable = NULL;
    axotext_setup();
    for (i = 0; i < axotextBufferHeadsIndex; i++) {
        AxotextChar *curChar = axotextBufferHeads[i];
        u8 *texture;

        if (font != curChar->font) {
            font = curChar->font;
            textureTable = AXOTEXT_SEG_TO_VIRT(font->textureTable);

            s = font->textureWidth * 32;
            t = font->textureHeight * 32;
            imageW = font->textureWidth / 2;
            tileW = (font->textureWidth * 2) - 2;
            tileH = (font->textureHeight * 4) - 4;
            tileSizeW = tileW * 2;
            tileSizeH = tileH;

            switch (font->filter) {
                case AXOTEXT_FILTER_POINT:
                    gDPSetTextureFilter(AXOTEXT_GDL_HEAD++, G_TF_POINT);
                    break;
                case AXOTEXT_FILTER_BILERP:
                    gDPSetTextureFilter(AXOTEXT_GDL_HEAD++, G_TF_BILERP);
                    break;
                case AXOTEXT_FILTER_AVERAGE:
                    gDPSetTextureFilter(AXOTEXT_GDL_HEAD++, G_TF_AVERAGE);
                    break;
            }
            gSPVertex(AXOTEXT_GDL_HEAD++, axotext_vertex, 4, 0);
            gSPModifyVertex(AXOTEXT_GDL_HEAD++, 0, G_MWO_POINT_ST, t);
            gSPModifyVertex(AXOTEXT_GDL_HEAD++, 1, G_MWO_POINT_ST, ((s << 16) + t));
            gSPModifyVertex(AXOTEXT_GDL_HEAD++, 2, G_MWO_POINT_ST, (s << 16));
            gSPModifyVertex(AXOTEXT_GDL_HEAD++, 3, G_MWO_POINT_ST, 0);
        }

        if (font != NULL) {
            texture = textureTable[curChar->c];
            gDPPipeSync(AXOTEXT_GDL_HEAD++);
            gSPTexture(AXOTEXT_GDL_HEAD++, 65535, 65535, 0, 0, 1);
            gDPSetTextureImage(AXOTEXT_GDL_HEAD++, G_IM_FMT_I, G_IM_SIZ_8b, imageW, texture);
            gDPSetTile(AXOTEXT_GDL_HEAD++, G_IM_FMT_I, G_IM_SIZ_8b, 4, 0, 7, 0, G_TX_WRAP | G_TX_NOMIRROR, 0, 0, G_TX_WRAP | G_TX_NOMIRROR, 0, 0);
            gDPLoadTile(AXOTEXT_GDL_HEAD++, 7, 0, 0, tileW, tileH);
            gDPSetTile(AXOTEXT_GDL_HEAD++, G_IM_FMT_I, G_IM_SIZ_4b, 4, 0, 0, 0, G_TX_CLAMP | G_TX_NOMIRROR, 7, 0, G_TX_CLAMP | G_TX_NOMIRROR, 6, 0);
            gDPSetTileSize(AXOTEXT_GDL_HEAD++, 0, 0, 0, tileSizeW, tileSizeH);
    
            while (curChar != NULL) {
                s32 modVtxLeft   = roundf(curChar->x * 4.0f);
                s32 modVtxRight  = roundf((curChar->x + curChar->w) * 4.0f);
                // Subtract top and bottom from screen height because gSPModifyVertex is stupid and the origin is in the top left
                s32 modVtxTop    = roundf((AXOTEXT_SCREEN_H - (curChar->y + curChar->h)) * 4.0f);
                s32 modVtxBottom = roundf((AXOTEXT_SCREEN_H - curChar->y) * 4.0f);
    
                gDPSetPrimColor(AXOTEXT_GDL_HEAD++, 0, 0, curChar->r, curChar->g, curChar->b, curChar->a);
                gSPModifyVertex(AXOTEXT_GDL_HEAD++, 0, G_MWO_POINT_XYSCREEN, ((modVtxLeft  << 16) + modVtxBottom));
                gSPModifyVertex(AXOTEXT_GDL_HEAD++, 1, G_MWO_POINT_XYSCREEN, ((modVtxRight << 16) + modVtxBottom));
                gSPModifyVertex(AXOTEXT_GDL_HEAD++, 2, G_MWO_POINT_XYSCREEN, ((modVtxRight << 16) + modVtxTop));
                gSPModifyVertex(AXOTEXT_GDL_HEAD++, 3, G_MWO_POINT_XYSCREEN, ((modVtxLeft  << 16) + modVtxTop));
                gSP2Triangles(AXOTEXT_GDL_HEAD++, 0, 1, 2, 0x0, 0, 2, 3, 0x0);
    
                curChar = curChar->next;
            }
        }
    }
    axotextBufferIndex = 0;
    axotextBufferHeadsIndex = 0;
}