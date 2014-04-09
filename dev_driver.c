#include <linux/kernel.h>
#include <linux/module.h> 
#include <linux/init.h> 
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/interrupt.h>
#include <linux/hrtimer.h>
#include <linux/sched.h>

MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "justlex i kir9k i microwolnovka" );
MODULE_DESCRIPTION( "Test module" );
MODULE_SUPPORTED_DEVICE( "timer_out" );

#define SUCCESS 0
MODULE_SUPPORTED_DEVICE( "test" ); 

#define SUCCESS 0
#define DEVICE_NAME "test"

static int device_open( struct inode *, struct file * );
static int device_release( struct inode *, struct file * );
static ssize_t device_read( struct file *, char *, size_t, loff_t * );
static ssize_t device_write( struct file *filp, char *buf,
                      size_t count, loff_t *f_pos);
static struct timer_list my_timer;

static int major_number;
static int is_device_open = 0;
static char text[ 5 ] = "hello\n";
static char* text_ptr = text; /* Указатель на текущую позицию в тексте */
static int tick = 0;
static struct hrtimer htimer;
static ktime_t kt_periode;

static enum hrtimer_restart timer_function (struct hrtimer * unused)
{

    if (tick != 0) {
        printk( "%s\n", text);
        kt_periode = ktime_set(tick, 0); 
    }
    hrtimer_forward_now(& htimer, kt_periode);
    return HRTIMER_RESTART;
}

static void timer_init (void)
{
    kt_periode = ktime_set(1, 0); 
    hrtimer_init (& htimer, CLOCK_REALTIME, HRTIMER_MODE_REL);
    htimer.function = timer_function;
    hrtimer_start(& htimer, kt_periode, HRTIMER_MODE_REL);
}

static void timer_cleanup (void)
{
    hrtimer_cancel(& htimer);
}

static struct file_operations fops =
{
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release
};

static int __init test_init (void)
{
    printk( KERN_ALERT "TEST driver loaded!\n" );
    timer_init();
    major_number = register_chrdev( 0, DEVICE_NAME, &fops );
    if ( major_number < 0 ) {
        printk( "Registering the character device failed with %d\n", major_number );
        return major_number;
    }
    return SUCCESS;
}

static void __exit test_exit (void)
{
    unregister_chrdev( major_number, DEVICE_NAME );
    timer_cleanup();
}


static int device_open (struct inode *inode, struct file *file )
{
    text_ptr = text;

    if ( is_device_open ) {
        return -EBUSY;
    }

    is_device_open++;
    return SUCCESS;
}

static int device_release (struct inode *inode, struct file *file)
{
    is_device_open--;
    return SUCCESS;
}

static ssize_t device_write (struct file *filp, char *buf, size_t count, loff_t *f_pos) 
{
    sscanf(buf, "%d", &tick);
    return 1;
}

static ssize_t device_read (struct file *filp, char *buffer,size_t length, loff_t * offset )
{
    int byte_read = 0;

    if ( *text_ptr == 0 ) {
        return 0;
    }
    while ( length && *text_ptr ) {
        put_user( *( text_ptr++ ), buffer++ );
        length--;
        byte_read++;
    }
    return byte_read;
}

module_init( test_init );
module_exit( test_exit );