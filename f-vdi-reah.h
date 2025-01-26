

#ifndef UNTITLED1_F_VDI_READ_H
#define UNTITLED1_F_VDI_READ_H
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
using namespace std;
struct __attribute__((packed)) VDIHeader{
    unsigned int diskImage[16];
    unsigned int imageSignature;
    unsigned int version;
    unsigned int headerSize;
    unsigned int imageType;
    unsigned int imageFlags;
    unsigned int imageDescription[64];
    unsigned int offsetBlocks;
    unsigned int offsetData;
    unsigned int cylinders;
    unsigned int heads;
    unsigned int sectors;
    unsigned int sectorSize;
    unsigned int unused;
    unsigned int diskSize[2];
    unsigned int blockSize;
    unsigned int blockExtraData;
    unsigned int blocksInHdd;
    unsigned int blocksAllocated;
    unsigned int UuidVdi[4];
    unsigned int UuidSnap[4];
    unsigned int UuidLink[4];
    unsigned int UuidParent[4];
    unsigned int garbage[14];
};


typedef struct VDIFile {
    int fd;
    off_t cursor;
    size_t size;
    VDIHeader *header;
    void *translationMap;


} VDIFile;

struct VDIFile *vdiOpen(char *fn){
    int fd = open(fn, O_RDWR);
    cout << open(fn, O_RDWR) << endl;
    if (fd < 0) {
        perror("Error opening file");
        return nullptr;
    }
    auto *file = (struct VDIFile *)malloc(sizeof(struct VDIFile));
    cout << sizeof(struct VDIFile) << endl;// This prints the size of our VDI file
   /* cout << malloc(sizeof(struct VDIFile)) << endl;// This prints the hexadecimal address of memory allocated */
    if (!file) {
        perror("Memory allocation failed");
        close(fd);
        return nullptr;
    }

    file->fd = fd;
    file->cursor = 0;
    file->size= 0;
    file->header = (VDIHeader *)malloc(sizeof(VDIHeader));
    file->translationMap = malloc(1024);

    if (!file->header) {
        perror("Memory allocation failed for header");
        close(fd);
        free(file);
        return nullptr;
    }
    if(!file->translationMap){
        perror("Memory allocation failed for translation map");
        close(fd);
        free(file);
        return nullptr;
    }
    ssize_t bytesRead = pread(fd, file->header, 512, 0);
    cout << bytesRead  << "  bytesread"<< endl;
    if (bytesRead < 0) {
        perror("Error reading header from file");
        close(fd);
        free(file->header);
        free(file->translationMap);
        free(file);
        return nullptr;
    }

    file->size = bytesRead;

    return file;
}


void vdiClose(struct VDIFile *f){
    if(f == nullptr){
        printf("passed a nullptr in vdiClose");
        return;
    }
    cout << f << endl;
    close(f -> fd);
    free(f->header);
    free(f->translationMap);

    printf("file closed");

}
size_t vdiRead(struct VDIFile *f, void *buf, size_t count){
    if(f == nullptr){
        return -1;
    }
    size_t bytesRead = pread(f->fd, buf, count, f->cursor);

    if (bytesRead < 0) {
        perror("problem in vdiRead");
        return -1;
    }


    f->cursor += bytesRead;


    return bytesRead;
}




#endif //UNTITLED1_F_VDI_READ_H
