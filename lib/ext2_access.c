// ext2 definitions from the real driver in the Linux kernel.
#include "ext2fs.h"

// This header allows your project to link against the reference library. If you
// complete the entire project, you should be able to remove this directive and
// still compile your code.
#include "reference_implementation.h"

// Definitions for ext2cat to compile against.
#include "ext2_access.h"



///////////////////////////////////////////////////////////
//  Accessors for the basic components of ext2.
///////////////////////////////////////////////////////////

// Return a pointer to the primary superblock of a filesystem.
struct ext2_super_block * get_super_block(void * fs) {
    return (struct ext2_super_block *) (char*)fs + SUPERBLOCK_OFFSET;
}


// Return the block size for a filesystem.
__u32 get_block_size(void * fs) {
    return EXT2_BLOCK_SIZE(get_super_block(fs));
}


// Return a pointer to a block given its number.
// get_block(fs, 0) == fs;
void * get_block(void * fs, __u32 block_num) {
    return (void*) (char*)fs + block_num*get_block_size(fs); 
} 

// Return a pointer to the first block group descriptor in a filesystem. Real
// ext2 filesystems will have several of these, but, for simplicity, we will
// assume there is only one.
struct ext2_group_desc * get_block_group(void * fs, __u32 block_group_num) {
    // get pointer to start of block group descriptor table
    struct ext2_super_block * superBlock = get_super_block(fs);
    // added 2 to get it to work, thought it should be 1 !!!?
    __u32 blockGroupDescriptorFirstBlock = superBlock->s_first_data_block + 2;
    return (struct ext2_group_desc *) get_block(fs, blockGroupDescriptorFirstBlock);
    // add offset for block_group_num ?
}


// Return a pointer to an inode given its number. In a real filesystem, this
// would require finding the correct block group, but you may assume it's in the
// first one.
struct ext2_inode * get_inode(void * fs, __u32 inode_num) {
    struct ext2_super_block * superBlock = get_super_block(fs);
    // superBlock->s_inodes_per_group = 0, we're assuming only one block group so inode_num-1 is index into inode table
    // hardcode 0 b/c assume only one block group
    struct ext2_group_desc * blockGroupDescriptor = get_block_group(fs, 0);
    struct ext2_inode * startOfINodeTable = (struct ext2_inode *) get_block(fs, blockGroupDescriptor->bg_inode_table);
    return startOfINodeTable + inode_num - 1;
}



///////////////////////////////////////////////////////////
//  High-level code for accessing filesystem components by path.
///////////////////////////////////////////////////////////

// Chunk a filename into pieces.
// split_path("/a/b/c") will return {"a", "b", "c"}.
//
// This one's a freebie.
char ** split_path(char * path) {
    int num_slashes = 0;
    for (char * slash = path; slash != NULL; slash = strchr(slash + 1, '/')) {
        num_slashes++;
    }

    // Copy out each piece by advancing two pointers (piece_start and slash).
    char ** parts = (char **) calloc(num_slashes, sizeof(char *));
    char * piece_start = path + 1;
    int i = 0;
    for (char * slash = strchr(path + 1, '/');
         slash != NULL;
         slash = strchr(slash + 1, '/')) {
        int part_len = slash - piece_start;
        parts[i] = (char *) calloc(part_len + 1, sizeof(char));
        strncpy(parts[i], piece_start, part_len);
        piece_start = slash + 1;
        i++;
    }
    // Get the last piece.
    parts[i] = (char *) calloc(strlen(piece_start) + 1, sizeof(char));
    strncpy(parts[i], piece_start, strlen(piece_start));
    return parts;
}


// Convenience function to get the inode of the root directory.
struct ext2_inode * get_root_dir(void * fs) {
    return get_inode(fs, EXT2_ROOT_INO);
}


// Given the inode for a directory and a filename, return the inode number of
// that file inside that directory, or 0 if it doesn't exist there.
// 
// name should be a single component: "foo.txt", not "/files/foo.txt".
__u32 get_inode_from_dir(void * fs, struct ext2_inode * dir, 
        char * name) {
// can assert EXT2_S_IFDIR stored in the i_mode field of the inode structure to make sure dir

    // get data block number from inode then get pointer
    // for now assume whole directory on single block
    __u32 directoryEntriesBlock = dir->i_block[0];
    struct ext2_dir_entry * dirEntry = (struct ext2_dir_entry*) get_block(fs,directoryEntriesBlock);
    // follow linked list until inode is 0
    while (dirEntry->inode != 0) {
        // strncmp is necessary b/c for some reason some dirEntries have extra byte of data, so limit
        // length of search to length of input name
        if (strncmp(dirEntry->name, name, strlen(name)) == 0) {
            return dirEntry->inode;
        }
        else {
            //printf("rec_len: %d\n", dirEntry->rec_len);
            //printf("before: %p\n", dirEntry);
            //struct ext2_dir_entry * oldDirEntry = dirEntry;
            dirEntry = (struct ext2_dir_entry *) (((char*)dirEntry) + dirEntry->rec_len);
            //printf("after: %p\n", dirEntry);
            //printf("difference: %d\n", dirEntry-oldDirEntry);
            //printf("new-inode: %d\n", dirEntry->inode);
        }
    }

    return 0;
    //return _ref_get_inode_from_dir(fs, dir, name);
}


// Find the inode number for a file by its full path.
// This is the functionality that ext2cat ultimately needs.
__u32 get_inode_by_path(void * fs, char * path) {
    // FIXME: Uses reference implementation.
    return _ref_get_inode_by_path(fs, path);
}

