#ifndef _VDI_WRITE_H
#define _VDI_WRITE_H

#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <cstring>
#include <cerrno>

ssize_t vdiWrite(struct VDIFile *f, void *buf, size_t count) {
    if (f == nullptr || buf == nullptr) {
      errno = EINVAL;
      perror("vdiWrite: invalid argument");
      return -1;
    }

    size_t totalWritten = 0;
    char* data static_cast<char *>(buf);

    while (totalWritten < count) {
        ssize_t bytesWritten = pwrite(f->fd, data + totalWritten, count - totalWritten, f->cursor);
        if (bytesWritten < 0) {
            perror("vdiWrite error");
            return -1;
        }
        if (bytesWritten == 0) {
            break;
        }
        totalWritten += bytesWritten;
        f->cursor += bytesWritten;
    }

    return totalWritten
}

#endif
