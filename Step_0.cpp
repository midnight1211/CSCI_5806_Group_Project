#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <sys/types.h>
#include <unistd.h>

// Macros
#define MAX_FILES 100
#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR 2

typedef struct {
    FILE *file;    // File pointer
    int is_open;   // File open status
    int mode;      // Mode (O_RDONLY, O_WRONLY, O_RDWR)
    long offset;   // Current offset
} FileDescriptor;

FileDescriptor fd_table[MAX_FILES];

// Initialize the file descriptor table
void init_fd_table() {
    for (int i = 0; i < MAX_FILES; ++i) {
        fd_table[i].is_open = 0;
        fd_table[i].file = NULL;
    }
}

// Open function
int my_open(const char *fn, int mode) {
    if (!fn) {
        errno = EINVAL; // Invalid argument
        return -1;
    }

    const char *file_mode;
    if (mode == O_RDONLY) {
        file_mode = "r";
    } else if (mode == O_WRONLY) {
        file_mode = "w";
    } else if (mode == O_RDWR) {
        file_mode = "r+";
    } else {
        errno = EINVAL; // Invalid mode
        return -1;
    }

    for (int i = 0; i < MAX_FILES; ++i) {
        if (!fd_table[i].is_open) {
            FILE *file = fopen(fn, file_mode);
            if (!file) {
                return -1; // Error opening file
            }

            fd_table[i].file = file;
            fd_table[i].is_open = 1;
            fd_table[i].mode = mode;
            fd_table[i].offset = 0; // Start at the beginning of the file
            return i; // Return custom file descriptor
        }
    }

    errno = EMFILE; // Too many files open
    return -1;
}

// Close function
int my_close(int fd) {
    if (fd < 0 || fd >= MAX_FILES || !fd_table[fd].is_open) {
        errno = EBADF;
        return -1;
    }

    fclose(fd_table[fd].file);
    fd_table[fd].file = NULL;
    fd_table[fd].is_open = 0;
    return 0;
}

// Read function
ssize_t my_read(int fd, void *buf, size_t count) {
    if (fd < 0 || fd >= MAX_FILES || !fd_table[fd].is_open || !buf) {
        errno = EBADF;
        return -1;
    }
    size_t bytes_read = fread(buf, 1, count, fd_table[fd].file);
    if (bytes_read < count && ferror(fd_table[fd].file)) {
        return -1;
    }

    fd_table[fd].offset += bytes_read;
    return bytes_read;
}

// Write function
ssize_t my_write(int fd, const void *buf, size_t count) {
    if (fd < 0 || fd >= MAX_FILES || !fd_table[fd].is_open || !buf) {
        errno = EBADF;
        return -1;
    }

    size_t bytes_written = fwrite(buf, 1, count, fd_table[fd].file);
    if (bytes_written < count && ferror(fd_table[fd].file)) {
        return -1;
    }

    fd_table[fd].offset += bytes_written;
    return bytes_written;
}

// Lseek function
off_t my_lseek(int fd, off_t offset, int anchor) {
    if (fd < 0 || fd >= MAX_FILES || !fd_table[fd].is_open) {
        errno = EBADF;
        return -1;
    }

    int origin;
    if (anchor == 0) { // SEEK_SET
        origin = SEEK_SET;
    } else if (anchor == 1) { // SEEK_CUR
        origin = SEEK_CUR;
    } else if (anchor == 2) { // SEEK_END
        origin = SEEK_END;
    } else {
        errno = EINVAL;
        return -1;
    }

    if (fseek(fd_table[fd].file, offset, origin) != 0) {
        return -1;
    }

    fd_table[fd].offset = ftell(fd_table[fd].file);
    return fd_table[fd].offset;
}

void displayBufferPage(uint8_t *buf, uint32_t count, uint32_t skip, uint64_t offset) {
    if (count > 256) {
        count = 256; // Cap the count to 256 as per the specification
    }

    buf += skip; // Skip the specified number of bytes

    // Print header with the offset.
    printf("Offset: 0x%016llx\n", offset);

    for (uint32_t i = 0; i < count; i++) {
        if (i % 16 == 0) {
            // Print the offset for each new line of bytes.
            printf("0x%08llx: ", offset + i);
        }

        // Print each byte in hexadecimal format
        printf("%02x ", buf[i]);

        if (i % 16 == 15 || i == count - 1) {
            // End the line every 16 bytes or at the end of the count.
            printf("\n");
        }
    }
}

void displayBuffer(uint8_t *buf, uint32_t count, uint64_t offset) {
    uint32_t remaining = count;
    uint32_t chunk_size = 256;

    while (remaining > 0) {
        uint32_t to_display = (remaining < chunk_size) ? remaining : chunk_size;

        displayBufferPage(buf, to_display, 0, offset);

        buf += to_display;
        offset += to_display;
        remaining -= to_display;
    }
}

int main() {
    // Example usage
    uint8_t buffer[512];
    for (int i = 0; i < 512; i++) {
        buffer[i] = i % 256; // Fill buffer with sample data.
    }

    // Display the buffer in pages.
    displayBuffer(buffer, 512, 0);

    return 0;
}

int main() {
    init_fd_table();

    // Open a file for read-write
    int fd = open("example.txt", 2); //  read = 0, write = 1, read-write = 2
    if (fd == -1) {
         perror("Failed to open file");
        return 1;
    }

    // Write to the file
    const char *data = "Hello, world!";
    if (write(fd, data, strlen(data)) == -1) {
        perror("Failed to write");
        close(fd);
        return 1;
    }

    // Seek to the beginning of the file
    if (lseek(fd, 0, 0) == -1) {
        perror("Failed to seek");
        close(fd);
        return 1;
    }

    // Read the data back
    char buffer[50];
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    if (bytes_read == -1) {
        perror("Failed to read");
        close(fd);
        return 1;
    }

    buffer[bytes_read] = '\0';  // Null-terminate the string
    printf("Read: %s\n", buffer);

    // Close the file
    if (close(fd) == -1) {
        perror("Failed to close File");
        return 1;
    }

    return 0;
}
