#include <linux/notifier.h>
#include <linux/fb.h>
#include <linux/console.h>
#include <linux/vt_kern.h>
#include <linux/printk.h>

static int msm_panic_handler(struct notifier_block *nb, unsigned long event, void *ptr)
{
    struct fb_info *info = NULL;
    struct fb_fillrect rect;
    int i;

    if (num_registered_fb > 0)
        info = registered_fb[0];

    if (info) {
        rect.dx = 0;
        rect.dy = 0;
        rect.width = info->var.xres;
        rect.height = info->var.yres;
        rect.color = 0xFFFFFFFF;
        rect.rop = ROP_COPY;
        if (info->fbops && info->fbops->fb_fillrect)
            info->fbops->fb_fillrect(info, &rect);
        else
            sys_fillrect(info, &rect);
    }

    console_lock();
    for (i = 0; i < MAX_NR_CONSOLES; i++) {
        if (!vc_cons[i].d)
            continue;
        invert_screen(vc_cons[i].d, 0, vc_cons[i].d->vc_screenbuf_size, 0);
    }
    console_unlock();

    pr_emerg("%s", (char *)ptr);
    console_flush_on_panic();
    return NOTIFY_DONE;
}

static struct notifier_block msm_panic_nb = {
    .notifier_call = msm_panic_handler,
};

int msm_register_panic_notifier(void)
{
    return atomic_notifier_chain_register(&panic_notifier_list, &msm_panic_nb);
}

void msm_unregister_panic_notifier(void)
{
    atomic_notifier_chain_unregister(&panic_notifier_list, &msm_panic_nb);
}
