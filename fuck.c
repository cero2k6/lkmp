#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>

#define MODULE_VERSION "1.0"
#define MODULE_NAME "helloworld proc module"

/* this is how long our data->value char array can be */
#define HW_LEN 8

struct helloworld_data_t {
char value[HW_LEN + 1];
};

static struct proc_dir_entry *helloworld_file;

struct helloworld_data_t helloworld_data;

/* proc_read - proc_read_helloworld
* proc_read_helloworld is the callback function that the kernel calls when
* there's a read file operation on the /proc file (for example,
* cat /proc/helloworld). The file's data pointer (&helloworld_data) is
* passed in the data parameter. You first cast it to the helloworld_data_t
* structure. This proc_read function then uses the sprintf function to
* create a string that is pointed to by the page pointer. The function then
* returns the length of page. Because helloworld_data->value is set to
* "Default", the command cat /proc/helloworld should return
* helloworld Default
*/
static int proc_read_helloworld(char *page, char **start, off_t off,
int count, int *eof, void *data)
{
int len;

/* cast the void pointer of data to helloworld_data_t*/
struct helloworld_data_t *helloworld_data=(struct helloworld_data_t
*)data;

/* use sprintf to fill the page array with a string */
len = sprintf(page, "helloworld %s\n", helloworld_data->value);

return len;
}


/* proc_write - proc_write_helloworld
* proc_write_helloworld is the callback function that the kernel calls 
* when there's a write file operation on the /proc file, (for example,
* echo test > /proc/helloworld). The file's data pointer
* (&helloworld_data) is passed in the data parameter. You first cast it to
* the helloworld_data_t structure. The page parameter points to the
* incoming data. You use the copy_from_user function to copy the page 
* contents to the data->value field. Before you do that, though, you check
* the page length, which is stored in count to ensure that you don't
* overrun the length of data->value. This function then returns the length
* of the data copied.
*/
static int proc_write_helloworld(struct file *file, const char *page,
unsigned long count, void *data)
{
int len;

/* cast the void pointer of data to helloworld_data_t*/
struct helloworld_data_t *helloworld_data=(struct helloworld_data_t
*)data;

/* do a range checking, don't overflow buffers in kernel modules */
if(count > HW_LEN)
len = HW_LEN;
else
len = count;

/* use the copy_from_user function to copy page data to
* to our helloworld_data->value */
if(copy_from_user(helloworld_data->value, page, len)) {
return -EFAULT;
}

/* zero terminate helloworld_data->value */
helloworld_data->value[len] = '\0';

return len;
}

/* init - init_helloworld
* init_helloworld creates the /proc/helloworld entry file and obtains its
* pointer called helloworld_file. The helloworld_file fields, data,
* read_proc, write_proc and owner, are filled. init_helloworld completes
* by writing an entry to the system log using printk.
*/
static int __init init_helloworld(void)
{
int rv = 0;

/* Create the proc entry and make it readable and writeable by all - 0666 */
helloworld_file = create_proc_entry("helloworld", 0666, NULL);
if(helloworld_file == NULL) {
return -ENOMEM;
}

/* set the default value of our data to Default. This way a read operation
on
* /proc/helloworld will return something. */
strcpy(helloworld_data.value, "Default");

/* Set helloworld_file fields */
helloworld_file->data = &helloworld_data;
helloworld_file->read_proc = &proc_read_helloworld;
helloworld_file->write_proc = &proc_write_helloworld;
helloworld_file->owner = THIS_MODULE;

/* everything initialized */
printk(KERN_INFO "%s %s initialized\n",MODULE_NAME, MODULE_VERSION);
return 0;
}

/* exit - cleanup_helloworld
* cleanup_helloworld removes the /proc file entry helloworld and
* prints a message to the system log.
*/
static void __exit cleanup_helloworld(void)
{
remove_proc_entry("helloworld", NULL);

printk(KERN_INFO "%s %s removed\n", MODULE_NAME, MODULE_VERSION);
}


/* here are the compiler macros for module operation */
module_init(init_helloworld);
module_exit(cleanup_helloworld);

MODULE_AUTHOR("Craig Hollabaugh");
MODULE_DESCRIPTION("helloworld proc module");
MODULE_LICENSE("GPL");

EXPORT_NO_SYMBOLS;
