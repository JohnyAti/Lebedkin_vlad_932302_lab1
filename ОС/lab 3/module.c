#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/time64.h>
#include <linux/ktime.h>
#include <linux/timekeeping.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
#define HAVE_PROC_OPS
#endif

#define PROC_FILENAME "tsulab"

// Максимальное сближение с Землей: 19 декабря 2025 года, 00:00 UTC
static struct timespec64 closest_approach_date = 
{
    .tv_sec = 1766102400, // 19 декабря 2025, 00:00 UTC
};

static struct proc_dir_entry *our_proc_file;

static long days_between(struct timespec64 *start, struct timespec64 *end)
{
    time64_t diff_seconds = end->tv_sec - start->tv_sec;
    return diff_seconds / (60 * 60 * 24); 
}

static ssize_t profile_read(struct file *file, char __user *buffer, size_t buffer_length, loff_t *offset) 
{
    struct timespec64 now;
    long days_difference;
    char output_msg[512];
    int msg_len;
    ssize_t ret;
    
    ktime_get_real_ts64(&now);
    days_difference = days_between(&now, &closest_approach_date);

    if (days_difference > 0) 
    {
        msg_len = snprintf(output_msg, sizeof(output_msg),
            "====================================================\n"
            "        МОДУЛЬ ТГУ: МЕЖЗВЕЗДНЫЙ ОБЪЕКТ 3I/ATLAS\n"
            "====================================================\n\n"
            "НАУЧНЫЕ ДАННЫЕ:\n"
            "• Объект:          Межзвездная комета 3I/ATLAS\n"
            "• Дата обнаружения: Июль 2025 года\n"
            "• Макс. сближение: 19 декабря 2025 года\n\n"
            "СТАТУС:\n"
            "• До сближения осталось: %ld дня(ей)\n"
            "====================================================\n",
            days_difference);
    } 
    else if (days_difference == 0) 
    {
        msg_len = snprintf(output_msg, sizeof(output_msg),
            "====================================================\n"
            "        МОДУЛЬ ТГУ: МЕЖЗВЕЗДНЫЙ ОБЪЕКТ 3I/ATLAS\n"
            "====================================================\n\n"
            "ВНИМАНИЕ! СЕГОДНЯ ДЕНЬ МАКСИМАЛЬНОГО СБЛИЖЕНИЯ!\n\n"
            "НАУЧНЫЕ ДАННЫЕ:\n"
            "• Объект:          Межзвездная комета 3I/ATLAS\n"
            "• Дата обнаружения: Июль 2025 года\n"
            "• Макс. сближение: СЕГОДНЯ (19 декабря 2025)\n\n"
            "СТАТУС:\n"
            "• Происходит прямо сейчас!\n"
            "====================================================\n");
    }
    else 
    {
        long days_passed = -days_difference; 
        msg_len = snprintf(output_msg, sizeof(output_msg),
            "====================================================\n"
            "        МОДУЛЬ ТГУ: МЕЖЗВЕЗДНЫЙ ОБЪЕКТ 3I/ATLAS\n"
            "====================================================\n\n"
            "ИСТОРИЧЕСКАЯ СПРАВКА:\n"
            "• Объект:          Межзвездная комета 3I/ATLAS\n"
            "• Дата обнаружения: Июль 2025 года\n"
            "• Макс. сближение: 19 декабря 2025 года\n\n"
            "СТАТУС:\n"
            "• Со дня сближения прошло: %ld дня(ей)\n"
            "====================================================\n",
            days_passed);
    }

    if (*offset >= msg_len) 
    {
        ret = 0;  
    } 
    else 
    {
        if (copy_to_user(buffer, output_msg + *offset, msg_len - *offset)) {
            pr_info("Ошибка copy_to_user\n");
            ret = -EFAULT;
        } 
        else 
        {
            ret = msg_len - *offset;
            *offset += ret;
        }
    }
    
    pr_info("/proc/%s прочитан. Разница в днях: %ld\n", PROC_FILENAME, days_difference);
    return ret;
}

static ssize_t profile_write(struct file *file, const char __user *buff, size_t len, loff_t *off)
{
    
    *off += len;
    pr_info("Запись в /proc/%s (игнорируется)\n", PROC_FILENAME);
    return len;
}

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_file_fops = {
    .proc_read = profile_read,
    .proc_write = profile_write,
};
#else
static const struct file_operations proc_file_fops = {
    .owner = THIS_MODULE,  
    .read = profile_read,
    .write = profile_write,
};
#endif

static int __init tsulab_init(void)
{
    pr_info("Добро пожаловать в Томский государственный университет\n");
    
    our_proc_file = proc_create(PROC_FILENAME, 0644, NULL, &proc_file_fops);
    if (NULL == our_proc_file)
    {
        pr_alert("Ошибка: Не удалось создать /proc/%s\n", PROC_FILENAME);
        return -ENOMEM;
    }
    
    pr_info("Создан /proc/%s\n", PROC_FILENAME);
    return 0;
}

static void __exit tsulab_exit(void)
{
    proc_remove(our_proc_file);
    pr_info("/proc/%s удален\n", PROC_FILENAME);
    pr_info("Томский государственный университет навсегда!\n");
}

module_init(tsulab_init);
module_exit(tsulab_exit);
MODULE_LICENSE("GPL");
