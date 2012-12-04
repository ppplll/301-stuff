#ifndef __USERSPACEFS_H__
#define __USERSPACEFS_H__

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>   // for uint32_t, etc.
#include <sys/time.h> // for struct timeval

/* This code is based on the fine code written by Joseph Pfeiffer for his
   fuse system tutorial. */

/* Declare to the FUSE API which version we're willing to speak */
#define FUSE_USE_VERSION 26

#define S3ACCESSKEY "S3_ACCESS_KEY_ID"
#define S3SECRETKEY "S3_SECRET_ACCESS_KEY"
#define S3BUCKET "S3_BUCKET"

#define BUFFERSIZE 1024

// store filesystem state information in this struct

typedef struct {
    char name[1024];
    char type;
    int size;
    dev_t st_dev;     /* ID of device containing file */
    ino_t st_ino;     /* inode number */
    mode_t st_mode;    /* protection */
    nlink_t st_nlink;   /* number of hard links */
    uid_t st_uid;     /* user ID of owner */
    gid_t st_gid;     /* group ID of owner */
    dev_t st_rdev;    /* device ID (if special file) */
    off_t st_size;    /* total size, in bytes */
    blksize_t st_blksize; /* blocksize for file system I/O */
    blkcnt_t  st_blocks;  /* number of 512B blocks allocated */
    time_t atime;  /* time of last access */
    time_t mtime;   /* time of last modification */
    time_t ctime;   /* time of last status change */

    } s3dirent_t;






typedef struct {
    char s3bucket[BUFFERSIZE];
} s3context_t;

/*
 * Other data type definitions (e.g., a directory entry
 * type) should go here.
 */




#endif // __USERSPACEFS_H__
