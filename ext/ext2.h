#pragma once
#include <stdint.h>
#define pstruct struct __attribute__((packed))

#define EXT2_SIGNATURE (0xef53)

typedef struct superblock {
  uint32_t total_inodes;
  uint32_t total_blocks;
  uint32_t n_resv_blocks;
  uint32_t n_unalloc_blocks;
  uint32_t n_unalloc_inodes;
  uint32_t lba_superblock;
  uint32_t block_size;
  uint32_t frag_size;
  uint32_t n_blocks_per_group;
  uint32_t n_frags_per_group;
  uint32_t n_inodes_per_group;
  uint32_t time_lastmount;
  uint32_t time_lastwrite;
  uint16_t n_mounts_since_fsck;
  uint16_t n_mounts_allowed_fsck;
  uint16_t ext2_signature;
  uint16_t fs_state;
  uint16_t err_handling;
  uint16_t ver_minor;
  uint32_t time_last_fsck;
  uint32_t time_btw_fscks;
  uint32_t osid;
  uint32_t ver_major;
  uint16_t resv_uid;
  uint16_t resv_gid;
  uint32_t first_nonresv_inode;
  uint16_t sizeof_inode;
  uint16_t lba_superblock2;
  uint32_t opt_features;
  uint32_t req_features_readwrite;
  uint32_t req_features_write;
  uint8_t fsid[16];
  uint8_t volid[16];

} superblock_t;

typedef pstruct blockgroup_desctable {
  uint32_t lba_block_usage_bitmap;
  uint32_t lba_incode_usage_bitmap;
  uint32_t lba_inode_table;
  uint16_t n_unalloc_blocks;
  uint16_t n_unalloc_inodes;
  /* pad it out to 32 bytes */
  uint8_t padding[32-18];
} blockgroup_desctable_t;

 typedef pstruct inode {
  uint16_t perms;
  uint16_t uid;
  uint32_t sizelo;
  uint32_t lastaccess;
  uint32_t creation;
  uint32_t lastmod;
  uint32_t deletion;
  uint16_t gid;
  uint16_t n_hardlinks;
  uint32_t disk_size;
  uint32_t flag;
  uint32_t osv1;
  uint32_t dbptr[12];
  uint32_t sind_dbptr;
  uint32_t dind_dbptr;
  uint32_t tind_dbptr;
  uint32_t gen_num;
  uint32_t acl;
  uint32_t sizehi;
  uint32_t frag_addr;
  uint8_t osv2[12];
} inode_t;

typedef struct dirent {
  uint32_t inode; 
  uint16_t sz;
  uint8_t namelen;
  uint8_t resv;
  char name[];
} dirent_t;
