#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/module.h> // For module macros

static void read_user_file(const char *filename, char *buffer, size_t len) {
    struct file *file;
    ssize_t bytes_read;

    // Open the file
    file = filp_open(filename, O_RDONLY, 0);
    if (IS_ERR(file)) {
        // Handle error
        kfree(buffer);
        return;
    }

    // Read from the file
    bytes_read = kernel_read(file, buffer, len, &file->f_pos);
    if (bytes_read >= 0) {
        buffer[bytes_read] = '\0'; // Null-terminate the string
    }

    // Clean up
    filp_close(file, NULL);
}
