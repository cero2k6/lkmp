#include <linux/init.h>
#include <linux/module.h>

#include <linux/kernel.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/time.h>

#include <asm/unistd.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lorenzo Colitti <lorenzo@colitti.com>");

unsigned long **sys_call_table;
ssize_t (*read)(int f, const void *buf, size_t n);

/* Stolen from scprint.c
 * http://downloads.securityfocus.com/downloads/scprint.tar.gz
 */
unsigned long **find_sys_call_table(void) {
    unsigned long **sctable;
    unsigned long ptr;
    extern int loops_per_jiffy;

    sctable = NULL;
    for (ptr = (unsigned long)&loops_per_jiffy;
        ptr < (unsigned long)&boot_cpu_data; ptr += sizeof(void *)){
    
        unsigned long *p;
        p = (unsigned long *)ptr;
        if (p[__NR_close] == (unsigned long) sys_close){
            sctable = (unsigned long **)p;
            return &sctable[0];
        }
    }

    return NULL;
}

ssize_t loggingread(int fd, const void *buf, size_t n) {
    struct file *f = NULL;
	char *name = NULL;
    loff_t offset = 0;
	struct timeval tv;
    char tmpname[256];

	do_gettimeofday(&tv);

    if(current)
        f = current->files->fd[fd];

    if(f)
        offset = f->f_pos;

	if(f && f->f_dentry && f->f_vfsmnt) {
		name = d_path(f->f_dentry, f->f_vfsmnt, tmpname, sizeof(tmpname) - 1);
	}

	printk(KERN_DEBUG "READ: %lu.%lu (%s/%d) %Lu %s\n",
		tv.tv_sec, tv.tv_usec,
		current ? current->comm : "?",
		current ? current->pid : 0,
		offset,
		name ? name : "<no dentry>"
		);

    return read(fd, buf, n);
}

static int __init readlog_init(void) {
    sys_call_table = find_sys_call_table();
	printk(KERN_INFO "Loading readlog module, sys call table at %p\n", sys_call_table);
    read = (void *) (sys_call_table[__NR_read]);
    sys_call_table[__NR_read] = (void *) loggingread;

	return 0;
}

static void __exit readlog_exit(void) {
	printk(KERN_INFO "Unloading readlog module\n");

    sys_call_table[__NR_read] = (void *) read;
}

module_init(readlog_init);
module_exit(readlog_exit);
