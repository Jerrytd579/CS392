#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>

/*
struct stat {
    mode_t st_mode; // file type & mode (permissions) 
    ino_t st_ino; // i-node number (serial number) 
    dev_t st_dev; // device number (file system) 
    dev_t st_rdev; // device number for special files 
    nlink_t st_nlink; // number of links 
    uid_t st_uid; // user ID of owner 
    gid_t st_gid; // group ID of owner 
    off_t st_size; // size in bytes, for regular files 
    struct timespec st_atim; // time of last access 
    struct timespec st_mtim; // time of last modification 
    struct timespec st_ctim; // time of last file status change 
    blksize_t st_blksize; // best I/O block size 
    blkcnt_t st_blocks; // number of disk blocks allocated 
};
*/

int main(int argc, char *argv[]) {
    struct stat buf;
    char output[128], *ptr;

    for (int i = 1; i < argc; i++) {
        printf("%s: ", argv[i]);
        /* The lstat function is similar to stat, but when the named file is
           a symbolic link, lstat returns information about the symbolic link,
           not the file referenced by the symbolic link. */
        if (lstat(argv[i], &buf) < 0) {
            fprintf(stderr, "Error: Cannot stat '%s'.\n", argv[i]);
            continue;
        }
        if (S_ISREG(buf.st_mode)) {
            sprintf(output, "regular, %zu bytes", buf.st_size);
            ptr = output;
        } else if (S_ISDIR(buf.st_mode)) {
            ptr = "directory";
        } else if (S_ISCHR(buf.st_mode)) {
            ptr = "character special";
        } else if (S_ISBLK(buf.st_mode)) {
            ptr = "block special";
        } else if (S_ISFIFO(buf.st_mode)) {
            ptr = "fifo";
        } else if (S_ISLNK(buf.st_mode)) {
            ptr = "symbolic link";
        } else if (S_ISSOCK(buf.st_mode)) {
            ptr = "socket";
        } else {
            ptr = "** unknown mode **";
        }
        printf("%s\n", ptr);
        printf("Last modified: %s", ctime(&buf.st_mtime));
    }
    return EXIT_SUCCESS;
}
