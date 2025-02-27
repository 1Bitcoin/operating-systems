#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>

#define COOKIE_BUF_SIZE PAGE_SIZE

MODULE_LICENSE("Dual BSD/GPL");

int fortune_init(void);
ssize_t fortune_read(struct file *file, char *buf, size_t count, loff_t *f_pos);
ssize_t fortune_write(struct file *file, const char *buf, size_t count, loff_t *f_pos);
void fortune_exit(void);
int my_open(struct inode *inode, struct file *file);
int my_release(struct inode *inode, struct file *file);

struct proc_ops fops = {
    .proc_open = my_open,
    .proc_release = my_release,
    .proc_read = fortune_read,
    .proc_write = fortune_write,
};

char *cookie_buf;
struct proc_dir_entry *proc_file;
unsigned int read_index;
unsigned int write_index;

int my_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "call open\n");
    return 0;
}

int my_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "call release\n");
    return 0;
}

int fortune_init(void)
{
    printk(KERN_INFO "call init\n");
    cookie_buf = vmalloc(COOKIE_BUF_SIZE);

    if (!cookie_buf)
    {
        printk(KERN_INFO "Error: can't malloc cookie buffer\n");
        return -ENOMEM;
    }

    memset(cookie_buf, 0, COOKIE_BUF_SIZE);
    proc_file = proc_create("fortune", 0666, NULL, &fops);

    if (!proc_file)
    {
        vfree(cookie_buf);
        printk(KERN_INFO "Error: can't create fortune file\n");
        return -ENOMEM;
    }

    read_index = 0;
    write_index = 0;

    printk(KERN_INFO "Fortune module loaded successfully\n");
    return 0;
}




ssize_t fortune_read(struct file *file, char *buf, size_t count, loff_t *f_pos)
{
    printk(KERN_INFO "call read\n");
    int len;

    if (write_index == 0 || *f_pos > 0)
        return 0;

    if (read_index >= write_index)
        read_index = 0;

    len = sprintf(buf, "%s\n", &cookie_buf[read_index]);
    read_index += len;
    *f_pos += len;

    return len;
}


ssize_t fortune_write(struct file *file, const char *buf, size_t count, loff_t *f_pos)
{
    printk(KERN_INFO "call write\n");
    int free_space = (COOKIE_BUF_SIZE - write_index) + 1;

    if (count > free_space)
    {
        printk(KERN_INFO "Error: cookie pot is full\n");
        return -ENOSPC;
    }

    if (copy_from_user(&cookie_buf[write_index], buf, count))
        return -EFAULT;

    write_index += count;
    cookie_buf[write_index-1] = 0;

    return count;
}


void fortune_exit(void)
{
    printk(KERN_INFO "call exit\n");
    remove_proc_entry("fortune", NULL);

    if (cookie_buf)
        vfree(cookie_buf);

    printk(KERN_INFO "Fortune module unloaded\n");
}

module_init(fortune_init);
module_exit(fortune_exit);


