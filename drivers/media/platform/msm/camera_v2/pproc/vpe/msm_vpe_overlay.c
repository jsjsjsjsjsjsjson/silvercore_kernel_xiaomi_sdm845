#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/ion.h>
#include "msm_vpe.h"
#include "msm_vpe_overlay.h"
#include "font8x8_basic.h"

static void draw_char(u8 *buf, int width, int height, int x, int y, char c)
{
    const u8 *glyph;
    int row, col;

    if (c < 0 || c >= 128)
        return;
    glyph = font8x8_basic[(int)c];
    for (row = 0; row < 8; row++) {
        if (y + row >= height)
            break;
        for (col = 0; col < 8; col++) {
            if (x + col >= width)
                break;
            if (glyph[row] & (1 << col))
                buf[(y + row) * width + x + col] = 0xff;
        }
    }
}

static void draw_text(u8 *buf, int width, int height, int x, int y,
                      const char *text)
{
    int i;
    for (i = 0; text[i]; i++)
        draw_char(buf, width, height, x + i * 8, y, text[i]);
}

void msm_vpe_overlay_time(struct vpe_device *vpe_dev,
                          struct msm_vpe_frame_info_t *frame)
{
    char buf[32];
    u8 *vaddr;

    snprintf(buf, sizeof(buf), "%u.%06u",
             (u32)frame->timestamp.tv_sec,
             (u32)frame->timestamp.tv_usec);

    if (!frame->dest_ion_handle)
        return;

    vaddr = ion_map_kernel(vpe_dev->client, frame->dest_ion_handle);
    if (IS_ERR_OR_NULL(vaddr))
        return;

    draw_text(vaddr, frame->strip_info.dst_w, frame->strip_info.dst_h,
              4, 4, buf);

    ion_unmap_kernel(vpe_dev->client, frame->dest_ion_handle);
}
