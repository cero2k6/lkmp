/*
 *  procfs1.c -  create a "file" in /proc
 *
 */

#include <linux/module.h>	/* Specifically, a module */
#include <linux/kernel.h>	/* We're doing kernel work */
#include <linux/proc_fs.h>	/* Necessary because we use the proc fs */
#include <asm/uaccess.h>

#define procfs_name "helloworld"
static char MSG_BUFFER[256];
static char *msg_ptr;

/**
 * This structure hold information about the /proc file
 *
 */
struct proc_dir_entry *Our_Proc_File;

/* Put data into the proc fs file.
 * 
 * Arguments
 * =========
 * 1. The buffer where the data is to be inserted, if
 *    you decide to use it.
 * 2. A pointer to a pointer to characters. This is
 *    useful if you don't want to use the buffer
 *    allocated by the kernel.
 * 3. The current position in the file
 * 4. The size of the buffer in the first argument.
 * 5. Write a "1" here to indicate EOF.
 * 6. A pointer to data (useful in case one common 
 *    read for multiple /proc/... entries)
 *
 * Usage and Return Value
 * ======================
 * A return value of zero means you have no further
 * information at this time (end of file). A negative
 * return value is an error condition.
 *
 * For More Information
 * ====================
 * The way I discovered what to do with this function
 * wasn't by reading documentation, but by reading the
 * code which used it. I just looked to see what uses
 * the get_info field of proc_dir_entry struct (I used a
 * combination of find and grep, if you're interested),
 * and I saw that  it is used in <kernel source
 * directory>/fs/proc/array.c.
 *
 * If something is unknown about the kernel, this is
 * usually the way to go. In Linux we have the great
 * advantage of having the kernel source code for
 * free - use it.
 */
int
procfile_read(char *buffer,
	      char **buffer_location,
	      off_t offset, int buffer_length, int *eof, void *data)
{
	int ret;
	
	printk(KERN_INFO "procfile_read (/proc/%s) called\n", procfs_name);
	
	/* 
	 * We give all of our information in one go, so if the
	 * user asks us if we have more information the
	 * answer should always be no.
	 *
	 * This is important because the standard read
	 * function from the library would continue to issue
	 * the read system call until the kernel replies
	 * that it has no more information, or until its
	 * buffer is filled.
	 */
	if (offset > 0) {
		/* we have finished to read, return 0 */
		ret  = 0;
	} else {
		/* fill the buffer, return the buffer size */
		ret = sprintf(buffer, "HelloWorld!\n");
	}

	return ret;
}



static int helloworld_proc_open(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "/proc/%s open called\n", procfs_name);
	sprintf(MSG_BUFFER, "holyshit\n");
	msg_ptr = MSG_BUFFER;
	return 0;
}

static ssize_t helloworld_proc_read(struct file *file, char *buffer, size_t length, loff_t *loff) 
{
	int read_bytes = 0;
	if (*msg_ptr == 0) 
		return 0;
	while (length && *msg_ptr) {
		put_user(*(msg_ptr++), buffer++);
		length--;
		read_bytes++;
	}
	printk(KERN_INFO "/proc/%s read called length=%ld\n", procfs_name, length);
	return read_bytes;
}


static const struct file_operations helloworld_proc_fops = {
	.owner = THIS_MODULE,
	.open = helloworld_proc_open,
	.read = helloworld_proc_read,
};

int init_module()
{
	//Our_Proc_File = create_proc_entry(procfs_name, 0644, NULL, NULL);
	//Our_Proc_File = proc_create(procfs_name, 0644, NULL, &helloworld_proc_fops);
        /*
	如果指定proc_create方法，第四个参数file_operations，那么Our_Proc_File entry的read_proc将不会生效
        */
	Our_Proc_File = proc_create(procfs_name, 0644, NULL, NULL);
	
	if (Our_Proc_File == NULL) {
		remove_proc_entry(procfs_name, NULL);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
		       procfs_name);
		return -ENOMEM;
	}

	//Our_Proc_File->proc_fops = &helloworld_proc_fops;
	Our_Proc_File->read_proc = procfile_read;
	Our_Proc_File->mode 	 = S_IFREG | S_IRUGO;
	Our_Proc_File->uid 	 = 0;
	Our_Proc_File->gid 	 = 0;
	Our_Proc_File->size 	 = 37;

	printk(KERN_INFO "/proc/%s created\n", procfs_name);	
	return 0;	/* everything is ok */
}

void cleanup_module()
{
	remove_proc_entry(procfs_name, NULL);
	printk(KERN_INFO "/proc/%s removed\n", procfs_name);
}

MODULE_AUTHOR("Yong Li");
MODULE_DESCRIPTION("helloworld proc module");
MODULE_LICENSE("GPL");
