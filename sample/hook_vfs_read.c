#include "include/common_data.h"
#include "hook_framework.h"
#include <linux/path.h>
#include <linux/fs.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/string.h>

// Must pass the origin_function_name to HOOK_FUNC_TEMPLATE()
HOOK_FUNC_TEMPLATE(vfs_read);

// The hook function name must be "hook_ + origin_function_name"
ssize_t hook_vfs_read(struct file *file, char __user *buf, size_t count, loff_t *pos)
{
    char *path_buffer = NULL;
    char *result = NULL;

    // Must pass the origin_function_name to GET_CODESPACE_ADDERSS()
    char *origin_vfs_read = GET_CODESPACE_ADDERSS(vfs_read);

    path_buffer = kmalloc(512, GFP_KERNEL);
    if (!path_buffer) {
        goto out;
    }

    result = d_path(&file->f_path, path_buffer, 512);
    if (!IS_ERR(result)) {
        if (!strnstr(result, "/dev/kmsg", 512 - (result - path_buffer)) && 
        !strnstr(result, "[timerfd]", 512 - (result - path_buffer)) &&
        !strnstr(result, "/proc/kmsg", 512 - (result - path_buffer)) &&
        !strnstr(result, "/run/log", 512 - (result - path_buffer)) &&
        !strnstr(result, "/var/log", 512 - (result - path_buffer))
        )
            printk(KERN_ALERT"reading %s\n", result);
    }
    kfree(path_buffer);
out:
    return ((ssize_t (*)(struct file *file, char __user *buf, size_t count, loff_t *pos))origin_vfs_read)(file, buf, count, pos);
}
