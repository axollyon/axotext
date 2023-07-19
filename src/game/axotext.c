#include <ultra64.h>
#include "axotext.h"

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
 * Print some text. The AxotextParams struct should be declared alongside your code before this is called.
 */
void axotext_print(f32 x, f32 y, AxotextParams *params, s32 limit, const char *str) {
    f32 currentX, currentY, charWidth, charHeight;
    s32 s, t, imageW, tileW, tileH, tileSizeW, tileSizeH;
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
    s = font->textureWidth * 32;
    t = font->textureHeight * 32;
    imageW = font->textureWidth / 2;
    tileW = (font->textureWidth * 2) - 2;
    tileH = (font->textureHeight * 4) - 4;
    tileSizeW = tileW * 2;
    tileSizeH = tileH;

    gDPSetEnvColor(AXOTEXT_GDL_HEAD++, params->r, params->g, params->b, params->a);

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
                currentY += params->lineHeight;
                goto newLine;
                break;
            }
            default: {
                const u8 *texture = textureTable[c];
                if (texture != NULL) {
                    s32 modVtxLeft   = roundf(currentX * 4.0f);
                    s32 modVtxRight  = roundf((currentX + charWidth) * 4.0f);
                    // Subtract top and bottom from screen height because gSPModifyVertex is stupid and the origin is in the top left
                    s32 modVtxTop    = roundf((AXOTEXT_SCREEN_H - (currentY + charHeight)) * 4.0f);
                    s32 modVtxBottom = roundf((AXOTEXT_SCREEN_H - currentY) * 4.0f);

                    gDPPipeSync(AXOTEXT_GDL_HEAD++);
                    gDPSetCombineLERP(
                        AXOTEXT_GDL_HEAD++, 
                        0, 0, 0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0, 
                        0, 0, 0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0
                    );
                    gSPClearGeometryMode(AXOTEXT_GDL_HEAD++, G_ZBUFFER);
                    gDPSetRenderMode(AXOTEXT_GDL_HEAD++, G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2);
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
                    gSPTexture(AXOTEXT_GDL_HEAD++, 65535, 65535, 0, 0, 1);
                    gDPSetTextureImage(AXOTEXT_GDL_HEAD++, G_IM_FMT_I, G_IM_SIZ_8b, imageW, texture);
                    gDPSetTile(AXOTEXT_GDL_HEAD++, G_IM_FMT_I, G_IM_SIZ_8b, 4, 0, 7, 0, G_TX_WRAP | G_TX_NOMIRROR, 0, 0, G_TX_WRAP | G_TX_NOMIRROR, 0, 0);
                    gDPLoadTile(AXOTEXT_GDL_HEAD++, 7, 0, 0, tileW, tileH);
                    gDPSetTile(AXOTEXT_GDL_HEAD++, G_IM_FMT_I, G_IM_SIZ_4b, 4, 0, 0, 0, G_TX_CLAMP | G_TX_NOMIRROR, 7, 0, G_TX_CLAMP | G_TX_NOMIRROR, 6, 0);
                    gDPSetTileSize(AXOTEXT_GDL_HEAD++, 0, 0, 0, tileSizeW, tileSizeH);
                    gSPVertex(AXOTEXT_GDL_HEAD++, axotext_vertex, 4, 0);
                    gSPModifyVertex(AXOTEXT_GDL_HEAD++, 0, G_MWO_POINT_XYSCREEN, ((modVtxLeft  << 16) + modVtxBottom));
                    gSPModifyVertex(AXOTEXT_GDL_HEAD++, 1, G_MWO_POINT_XYSCREEN, ((modVtxRight << 16) + modVtxBottom));
                    gSPModifyVertex(AXOTEXT_GDL_HEAD++, 2, G_MWO_POINT_XYSCREEN, ((modVtxRight << 16) + modVtxTop));
                    gSPModifyVertex(AXOTEXT_GDL_HEAD++, 3, G_MWO_POINT_XYSCREEN, ((modVtxLeft  << 16) + modVtxTop));
                    gSPModifyVertex(AXOTEXT_GDL_HEAD++, 0, G_MWO_POINT_ST, t); // 0
                    gSPModifyVertex(AXOTEXT_GDL_HEAD++, 1, G_MWO_POINT_ST, ((s << 16) + t)); // t
                    gSPModifyVertex(AXOTEXT_GDL_HEAD++, 2, G_MWO_POINT_ST, (s << 16)); // ((s << 16) + t)
                    gSPModifyVertex(AXOTEXT_GDL_HEAD++, 3, G_MWO_POINT_ST, 0); // (s << 16)
                    gSP2Triangles(AXOTEXT_GDL_HEAD++, 0, 1, 2, 0x0, 0, 2, 3, 0x0);
                }
                currentX += (f32)kerningTable[c] * (charWidth / (f32)font->textureWidth);
                break;
            }
        }
    }
    axotext_revert();
}