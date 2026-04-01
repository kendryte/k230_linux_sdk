#ifndef _NONAI2D_V4L2_CONTROLS_H_
#define _NONAI2D_V4L2_CONTROLS_H_

/****************************************************************************
 * Includes
 ****************************************************************************/

#include <linux/version.h>
#include <linux/videodev2.h>
#include <linux/v4l2-controls.h>

#define NONAI2D_OSD_REGION_NUM (8)             /* Max region number of osd */
#define NONAI2D_BORDER_NUM (32)                /* Max number of border */

#define V4L2_CID_USER_NONAI2D_BASE              (V4L2_CID_USER_BASE + 0x1200)

#define V4L2_CID_USER_NONAI2D_MODE              (V4L2_CID_USER_NONAI2D_BASE + 0)
#define V4L2_CID_USER_NONAI2D_COLOR_GAMUT       (V4L2_CID_USER_NONAI2D_BASE + 1)
#define V4L2_CID_USER_NONAI2D_COEFF             (V4L2_CID_USER_NONAI2D_BASE + 2)
#define V4L2_CID_USER_NONAI2D_OSD_ATTR          (V4L2_CID_USER_NONAI2D_BASE + 3)
#define V4L2_CID_USER_NONAI2D_BORDER_ATTR       (V4L2_CID_USER_NONAI2D_BASE + 4)

enum nonai2d_mode {
    NONAI2D_MODE_CSC = 0,     /* Color space conversion */
    NONAI2D_MODE_OSD,         /* On Screen Display */
    NONAI2D_MODE_BORDER,      /* Draw border */
    NONAI2D_MODE_OSD_BORDER,  /* OSD first, then draw border */
    NONAI2D_MODE_BUTT
};

typedef enum {
    /* bottom ------> top */
    NONAI2D_ADD_ORDER_VIDEO_OSD = 0,   /* Add order of OSD, from bottom to top: VIDEO_OSD */
    NONAI2D_ADD_ORDER_OSD_VIDEO,       /* Add order of OSD, from bottom to top: OSD_VIDEO */
    NONAI2D_ADD_ORDER_VIDEO_BG,        /* Add order of OSD, from bottom to top: VIDEO_BG */
    NONAI2D_ADD_ORDER_BG_VIDEO,        /* Add order of OSD, from bottom to top: BG_VIDEO */
    NONAI2D_ADD_ORDER_VIDEO_BG_OSD,    /* Add order of OSD, from bottom to top: VIDEO_BG_OSD */
    NONAI2D_ADD_ORDER_VIDEO_OSD_BG,    /* Add order of OSD, from bottom to top: VIDEO_OSD_BG */
    NONAI2D_ADD_ORDER_BG_VIDEO_OSD,    /* Add order of OSD, from bottom to top: BG_VIDEO_OSD */
    NONAI2D_ADD_ORDER_BG_OSD_VIDEO,    /* Add order of OSD, from bottom to top: BG_OSD_VIDEO */
    NONAI2D_ADD_ORDER_OSD_VIDEO_BG,    /* Add order of OSD, from bottom to top: OSD_VIDEO_BG */
    NONAI2D_ADD_ORDER_OSD_BG_VIDEO,    /* Add order of OSD, from bottom to top: OSD_BG_VIDEO */
    NONAI2D_ADD_ORDER_BUTT
} nonai2d_osd_order;

typedef struct {
    uint8_t  index;                     /* OSD region index, should be less than NONAI2D_OSD_REGION_NUM */
    uint8_t  valid;                     /* Enable or disable of OSD region */
    uint16_t width;                     /* Width of OSD image */
    uint16_t height;                    /* Height of OSD image */
    uint16_t startx;                    /* Start coordinate in horizontal of OSD image */
    uint16_t starty;                    /* Start coordinate in vertical of OSD image */
    uint8_t  *data;                     /* Pointer of OSD image data */
    uint8_t bg_alpha;                   /* Alpha of background in OSD region */
    uint8_t osd_alpha;                  /* Alpha of OSD in OSD region */
    uint8_t video_alpha;                /* Alpha of input image in OSD region */
    nonai2d_osd_order add_order;        /* Add order of OSD region */
    uint32_t bg_color;                  /* Background color in OSD region
                                           (R << 16) | (G << 8) | (B << 0);
                                        */
    uint32_t pixfmt;                    /* Pixel format of OSD image, supported format:
                                            V4L2_PIX_FMT_ARGB32
                                            V4L2_PIX_FMT_ARGB444
                                            V4L2_PIX_FMT_ARGB555
                                            V4L2_PIX_FMT_XRGB32
                                            V4L2_PIX_FMT_XRGB444
                                            V4L2_PIX_FMT_XRGB555
                                            V4L2_PIX_FMT_BGRA32
                                            V4L2_PIX_FMT_BGRA444
                                            V4L2_PIX_FMT_BGRA555
                                            V4L2_PIX_FMT_BGRX32
                                            V4L2_PIX_FMT_BGRX444
                                            V4L2_PIX_FMT_BGRX555
                                            V4L2_PIX_FMT_RGB24
                                            V4L2_PIX_FMT_BGR24
                                            V4L2_PIX_FMT_RGB565
                                        */
} nonai2d_osd_attr;

typedef struct
{
    uint8_t  index;           /* Border index, should be less than NONAI2D_BORDER_NUM */
    uint16_t width;           /* Width of border */
    uint16_t height;          /* Height of border */
    uint16_t line_width;      /* Wide of border line */
    uint32_t color;           /* Color of border: (R << 16) | (G << 8) | (B << 0); */
    uint16_t startx;          /* Start coordinate in horizontal of border */
    uint16_t starty;          /* Start coordinate in vertical of border */
} nonai2d_border_attr;

typedef enum
{
    NONAI2D_COLOR_GAMUT_BT601 = 0,
    NONAI2D_COLOR_GAMUT_BT709,
    NONAI2D_COLOR_GAMUT_BT2020,
    NONAI2D_COLOR_GAMUT_BUTT
} nonai2d_color_gamut;

typedef struct
{
    uint16_t coef[12];                /* Pointer of coefficent */
} nonai2d_coef_attr;

#endif /* _NONAI2D_V4L2_CONTROLS_H_ */
