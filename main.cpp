include "f-vdi-read.h"
#include "iostream"
using namespace std;
void displayBufferPage(uint8_t *buf, uint32_t count, uint32_t skip, uint64_t offset) {
    printf("Offset: %llu\n", offset);
    for (uint32_t i = 0; i < 256; i++) {
        if (i >= skip && i < skip + count) {
            printf("%02x ", buf[i]);
        } else {
            printf("   ");
        }

        if ((i + 1) % 16 == 0) {
            printf("\n");
        }


    }
    printf("\n");
    printf("   CHARACTERS\n");
    for (uint32_t i = 0; i < 256; i++) {
        if (i >= skip && i < skip + count) {
            if (isprint(buf[i])) {
                printf("%c", buf[i]);
            } else {
                printf(".");
            }
        }


        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
}



void displayBuffer(uint8_t *buf, uint32_t count, uint64_t offset) {
    uint32_t chunkSize = 256;
    uint32_t bytesLeft = count;

    for (uint64_t i = 0; bytesLeft > 0; i++) {
        uint32_t bytesToDisplay = bytesLeft < chunkSize ? bytesLeft : chunkSize;
        displayBufferPage(buf + i * chunkSize, bytesToDisplay, 0, offset + i * chunkSize);
        bytesLeft -= bytesToDisplay;
    }
}
void displayHeaderInfo(struct VDIFile *file) {
    if (file == nullptr || file->header == nullptr) {
        printf("Invalid file or header\n");
        return;
    }

    VDIHeader *header = (VDIHeader *)file->header;  // Access the header
    printf("\nHeader Information:\n");
    printf("=====================\n");




    printf("Version: %u\n", header->version);

    
    printf("Header Size: %u bytes\n", header->headerSize);

    
    printf("Data Start Offset: %u bytes\n", header->sectorSize);

    printf('')



    // Print the translation map offset

    printf("=====================\n");
}

    int main() {

        const char *validFile = "<enter file path here>";  // Change this to your on your system file path
        printf("Testing with valid file: %s\n", validFile);
        struct VDIFile *file = vdiOpen((char *) validFile);
        if (file != nullptr) {
                printf("File opened successfully with file descriptor %d\n", file->fd);
                displayHeaderInfo(file);
                displayBuffer((uint8_t *)file->header, file->header->headerSize, file->header->sectorSize);
                vdiClose(file);
            } else {
                printf("Failed to open the VDI file.\n");
            }
            return 0;
        }
