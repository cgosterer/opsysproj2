#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/time.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Get time using current kernel time");

#define ENTRY_NAME "timed"
#define PERMS 0644
#define PARENT NULL
static struct file_operations fops;
struct timespec time_spec1, time_spec2;

static char * message;
static char * seconds;
static char * difference;
static int read_p;
int readcount;


int xtime_proc_open (struct inode *sp_inode, struct file *sp_file)
{
	printk(KERN_INFO "proc called open\n");
	read_p = 1;
	message = kmalloc(sizeof(char) * 200, __GFP_RECLAIM | __GFP_IO | __GFP_FS);
	if (message == NULL)
	{
		printk(KERN_WARNING "xtime_proc_open");
		return -ENOMEM;
	}

	strcpy(message, "current time: ");
	return 0;
}

ssize_t xtime_proc_read(struct file *sp_file, char __user *buf, size_t size, loff_t *offset)
{
	if(readcount > 0)
	{
		time_spec2 = current_kernel_time();
		unsigned long result =  time_spec2.tv_sec - time_spec1.tv_sec;
		read_p = !read_p;
        	if (read_p)
                	return 0;
       		printk(KERN_INFO "proc called read\n");
       		seconds = kmalloc(sizeof(char) * 20, __GFP_RECLAIM | __GFP_IO | __GFP_FS);
        	sprintf(seconds, "%lu", time_spec2.tv_sec );
        	strcat(seconds,"\n");
       		strcat(message, seconds );

		difference = kmalloc(sizeof(char) * 20, __GFP_RECLAIM | __GFP_IO | __GFP_FS);
		sprintf(difference, "%lu", result );
		strcat(message,"elapsed time: ");
		strcat(message, difference);
		strcat(message, "\n");


        	int len = strlen(message);
        	copy_to_user(buf, message, len);
		time_spec1.tv_sec = time_spec2.tv_sec;
        	readcount++;
        	return len;
	}

	else
	{
		time_spec1 = current_kernel_time();
		read_p = !read_p;
		if (read_p)
			return 0;
		printk(KERN_INFO "proc called read\n");
		seconds = kmalloc(sizeof(char) * 20, __GFP_RECLAIM | __GFP_IO | __GFP_FS);
		sprintf(seconds, "%lu", time_spec1.tv_sec);
		strcat(seconds,"\n");
		strcat(message, seconds );
		int len = strlen(message);
		copy_to_user(buf, message, len);
		readcount++;
		return len;
	}
}

int xtime_proc_release(struct inode *sp_inode, struct file *sp_file)
{
	printk(KERN_INFO "proc called release\n");
	kfree(message);
	kfree(seconds);
	kfree(difference);
	return 0;
}

static int xtime_init(void)
{
	readcount = 0;
	printk(KERN_NOTICE "/proc/%s create\n", ENTRY_NAME);
	fops.open = xtime_proc_open;
	fops.read = xtime_proc_read;
	fops.release = xtime_proc_release;

	if (!proc_create(ENTRY_NAME, PERMS, NULL, &fops))
	 {
		printk("ERROR! proc_create\n");
		remove_proc_entry(ENTRY_NAME, NULL);
		return -ENOMEM;
	 }
	 return 0;
}

module_init(xtime_init);

static void xtime_exit(void)
{
	remove_proc_entry(ENTRY_NAME, NULL);
	printk(KERN_NOTICE "Removing /proc/%s.\n", ENTRY_NAME);
}

module_exit(xtime_exit);
