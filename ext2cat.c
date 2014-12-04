#include <unistd.h>
#include <stdio.h>
// #include <assert.h>
#include "ext2_access.h"
#include "mmapfs.h"

// #ifndef max
//     #define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
// #endif

int main(int argc, char ** argv) {
    // Extract the file given in the second argument from the filesystem image
    // given in the first.
    if (argc != 3) {
        printf("usage: ext2cat <filesystem_image> <path_within_filesystem>\n");
        exit(1);
    }
    char * fs_path = argv[1];
    char * file_path = argv[2];
    void * fs = mmap_fs(fs_path);

    // Get the inode for our target file.
    __u32 target_ino_num = get_inode_by_path(fs, file_path);
    if (target_ino_num == 0) {
        printf("cat: %s does not exist\n", file_path);
        return 1;
    }
    struct ext2_inode * target_ino = get_inode(fs, target_ino_num);

    __u32 block_size = get_block_size(fs);
    __u32 size = target_ino->i_size;
    __u32 bytes_read = 0;
    void * buf = calloc(size, 1);

    // Read the file one block at a time. In the real world, there would be a
    // lot of error-handling code here.
    __u32 bytes_left;

    /* Old block reading code was here, changed for extra credit */

    void* block = NULL;
    __u32 bytes_to_read;
    __u32 inode_blocks = (__u32)(block_size / sizeof(__u32));

    for (int i = 0; i < EXT2_N_BLOCKS-1; i++) {
        //retrieve block
        block = get_block(fs, target_ino->i_block[i]);
        if(i < EXT2_DIND_BLOCK-1){  
            bytes_left = size - bytes_read;
            if (bytes_left == 0) break;
            bytes_to_read = bytes_left > block_size ? block_size : bytes_left;
            void * block = get_block(fs, target_ino->i_block[i]);
            memcpy(buf + bytes_read, block, bytes_to_read);
            bytes_read += bytes_to_read;
            
        }

        else if(i == EXT2_DIND_BLOCK-1){
        void* new_dir = NULL;
        __u32 j;
            for(j=0;j<inode_blocks;j++){
                    bytes_left = size - bytes_read;
                    if (bytes_left != 0) {
                        bytes_to_read = bytes_left > block_size ? block_size : bytes_left;
                        new_dir = get_block(fs, *(__u32*)(block+j*sizeof(__u32)));

                        memcpy(buf + bytes_read, new_dir, bytes_to_read);
                        bytes_read += bytes_to_read;
                    }

                    else break;   
            }
        }

        /*******************/
        else if(i == EXT2_TIND_BLOCK -1){
            void* new_dir__ = NULL;
            __u32 j, k;
            for(j=0;j<inode_blocks;j++){
                new_dir__ = get_block(fs, *(__u32*)(block+(j*sizeof(__u32))));
                for(k=0;k<inode_blocks;k++){
                    bytes_left = size - bytes_read;
                    if (bytes_left != 0) {
                        bytes_to_read = bytes_left > block_size ? block_size : bytes_left;
                        memcpy(buf + bytes_read, new_dir__, bytes_to_read);
                        bytes_read += bytes_to_read;
                    }
                    else break;
                    
                }
            }
        }
    }

    write(1, buf, bytes_read);
    return 0;
}

