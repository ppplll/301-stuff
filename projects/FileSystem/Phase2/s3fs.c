/* This code is based on the fine code written by Joseph Pfeiffer for his
   fuse system tutorial. */

#include "s3fs.h"
#include "libs3_wrapper.h"

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <libgen.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/xattr.h>

#define GET_PRIVATE_DATA ((s3context_t *) fuse_get_context()->private_data)

/*
 * For each function below, if you need to return an error,
 * read the appropriate man page for the call and see what
 * error codes make sense for the type of failure you want
 * to convey.  For example, many of the calls below return
 * -EIO (an I/O error), since there are no S3 calls yet
 * implemented.  (Note that you need to return the negative
 * value for an error code.)
 */




    
//int fs_updatedir(s3dirent_t *dir){
    /*any variable commented out is something that would not change in an add or remove dir call*/
    //dir[0].st_dev = ;     /* ID of device containing file */
    //dir[0].st_ino;     /* inode number */
    //dir[0].st_mode;    /* protection */
    //dir[0].st_nlink;   /* number of hard links */
    //dir[0].st_uid = ;     /* user ID of owner */
    //dir[0].st_gid = ;     /* group ID of owner */
    //dir[0].st_rdev = ;    /* device ID (if special file) */
    //dir[0].st_size = sizeof(dir);    /* total size, in bytes */
    //dir[0].st_blksize = sizeof(dir)/512; /* blocksize for file system I/O */
    //dir[0].st_blocks = sizeof(dir)/512;  /* number of 512B blocks allocated */
    //struct timeval time;
    
    //dir[0].atime = ;  /* time of last access */
    //dir[0].mtime = ;   /* time of last modification */
    //dir[0].ctime = ;  /*time of last status change */ 

/*
* Find what type of file path, return 1 on directory, 2 on file, 0 if
* it does not exist
*/
int fs_findtype( char *path){
    char parent[1024];
    char file[1024];
    char cpy[1024];
    strcpy(cpy,path);
    strcpy(file, basename(cpy));
    strcpy(parent,dirname(cpy));
     
    if(!strcmp(path,"/")){//if looking for root
        strcpy(file, ".");
        return 1;
    } 
    uint8_t * buff;
    int rv = s3fs_get_object(getenv(S3BUCKET), parent, &buff, 0, 0);
    int dirsize = (rv)/(sizeof(s3dirent_t));
    s3dirent_t * parentdir = (s3dirent_t*)buff;
    int i = 0;
    fprintf(stderr, "\n%d\n", rv);
    parentdir[0].atime = time(NULL);// update access time
    s3fs_put_object(getenv(S3BUCKET), parent, (uint8_t*) parentdir, parentdir[0].st_size);
    for(;i<dirsize;i++){
        fprintf(stderr, "FILE\n%s\n%s\n%s\n", file,parent,parentdir[i].name);
        if (!(strcmp(parentdir[i].name,file))){
            
            if (parentdir[i].type == 'd'){
                free(buff);
                return 1;
            }
            free(buff);
            return 2; //assuming that filetype is not corrupted --> may want to add check/ handling later
        }
    }
    free(buff);
    return 0;
}

/*copy the metadata please*/
void cpystat(s3dirent_t *dir, struct stat *statbuf, int i){
    (*statbuf).st_mode = dir[i].st_mode;
    (*statbuf).st_nlink = dir[i].st_nlink;
    (*statbuf).st_uid = dir[i].st_uid;
    (*statbuf).st_gid = dir[i].st_gid;
    (*statbuf).st_size = dir[i].st_size;
    (*statbuf).st_atime = dir[i].atime;
    (*statbuf).st_mtime = dir[i].mtime;
    (*statbuf).st_ctime = dir[i].ctime; 
    return; 
 }
/* 
 * Get file attributes.  Similar to the stat() call
 * (and uses the same structure).  The st_dev, st_blksize,
 * and st_ino fields are ignored in the struct (and 
 * do not need to be filled in).
 */

void cpymeta(s3dirent_t *new, s3dirent_t *old, int i){
    new[i].st_mode = old[i].st_mode;
    new[i].st_nlink = old[i].st_nlink;
    new[i].st_uid = old[i].st_uid;
    new[i].st_gid = old[i].st_gid;
    new[i].st_size = old[i].st_size;
    new[i].atime = time(NULL);
    new[i].mtime = old[i].mtime;
    new[i].ctime = old[i].ctime;
    return;
}

int fs_getattr(const char *path, struct stat *statbuf) {
    fprintf(stderr, "fs_getattr(path=\"%s\")\n", path);
    int type = fs_findtype(path);
    fprintf(stderr, "%d\n", type);
    char parent[1024];
    char file[1024];
    char cpy[1024];
    strcpy(cpy,path);
    strcpy(file, basename(cpy));
    strcpy(parent,dirname(cpy));
    if (type == 1){//get attributes from a directory type
        fprintf(stderr,"\nIM a DIR\n");       
        uint8_t * buff;
        int rv = s3fs_get_object(getenv(S3BUCKET), path, &buff, 0, 0);
        int dirsize = (rv)/(sizeof(s3dirent_t));
        s3dirent_t *dir = (s3dirent_t*)buff;
        cpystat(dir, statbuf, 0);
        free(buff);
        //fprintf(stderr, "hey\n");
        return 0;
    } 
    if (type == 2){
        fprintf(stderr,"\nIM a FILE\n");
        uint8_t * buff;
        int rv = s3fs_get_object(getenv(S3BUCKET), parent, &buff, 0, 0);
        int dirsize = (rv)/(sizeof(s3dirent_t));
        s3dirent_t * dir = (s3dirent_t*)buff;
        int i = 0;
        for (;i<dirsize;i++){
            fprintf(stderr, "\n%s\n%s\n", dir[i].name, file);
            if(!(strcmp(dir[i].name,file))){
                fprintf(stderr, "INSIDE");
                cpystat(dir,statbuf, i);
            }
        }
        free(buff);
        return 0;
    }        
    //s3context_t *ctx = GET_PRIVATE_DATA;
    return -ENOENT;
}


/* 
 * Create a file "node".  When a new file is created, this
 * function will get called.  
 * This is called for creation of all non-directory, non-symlink
 * nodes.  You *only* need to handle creation of regular
 * files here.  (See the man page for mknod (2).)
 */
int fs_mknod(const char *path, mode_t mode, dev_t dev) {
    fprintf(stderr, "fs_mknod(path=\"%s\", mode=0%3o)\n", path, mode);
    s3context_t *ctx = GET_PRIVATE_DATA;
    char parent[1024];
    char file[1024];
    char cpy[1024];
    strcpy(cpy,path);
    strcpy(file, basename(cpy));
    strcpy(parent,dirname(cpy));
    
    
    if(fs_findtype(parent) !=1 ){//check if path is real
        return -1;
    }
    if(fs_findtype(path) !=0 ){//check if name already exists
        return -EEXIST;
    }
    s3fs_put_object(getenv(S3BUCKET),path, 0, 0);//empty object up
    uint8_t * buff;
    int rv = s3fs_get_object(getenv(S3BUCKET), parent, &buff, 0, 0);
    int dirsize = (rv)/(sizeof(s3dirent_t));
    s3dirent_t *dir = (s3dirent_t*)buff;
    int i = 0;
    s3dirent_t newdir[(dirsize+1)];
    for(;i<dirsize;i++)
    {
    	newdir[i]=dir[i];
    }
    strcpy((newdir[dirsize]).name,file);
    fprintf(stderr,"here");
    fprintf(stderr, "%s\n\n\n", file);
    newdir[dirsize].type='f';
    newdir[dirsize].st_mode = mode;
    newdir[dirsize].st_size = 0;
    newdir[dirsize].st_nlink = 1;
    newdir[dirsize].st_uid = getuid();
    newdir[dirsize].st_gid = getgid();
    newdir[dirsize].atime = time(NULL);
    newdir[dirsize].mtime = time(NULL);
    newdir[dirsize].ctime = time(NULL);

    newdir[0].st_size=sizeof(newdir);
    cpymeta(newdir,dir,0);
    newdir[0].st_mode = dir[0].st_mode;
    newdir[0].st_size = sizeof(newdir);
    newdir[0].mtime = newdir[0].atime;
    s3fs_remove_object(getenv(S3BUCKET),parent);
    s3fs_put_object(getenv(S3BUCKET),parent, (uint8_t*) newdir, newdir[0].st_size);      

    free(buff);
    return 0;
}

/* 
 * Create a new directory.
 *
 * Note that the mode argument may not have the type specification
 * bits set, i.e. S_ISDIR(mode) can be false.  To obtain the
 * correct directory type bits (for setting in the metadata)
 * use mode|S_IFDIR.
 */
int fs_mkdir(const char *path, mode_t mode) {
    fprintf(stderr, "fs_mkdir(path=\"%s\", mode=0%3o)\n", path, mode);
    s3context_t *ctx = GET_PRIVATE_DATA;
    mode |= S_IFDIR;
	
	char key[1024];
    char parent[1024];
    char cpy[1024];
    strcpy(cpy,path);
    strcpy(key, basename(cpy));
    strcpy(parent,dirname(cpy));
    
	
	
	//No parent directory
	if(fs_findtype(parent) !=1){
    	return -EIO;
    }
	
	//The directory you want to make already exits (we do not allow for dirs and files to share names)
	if(fs_findtype(path) !=0 ){
        return -EEXIST;
    }
    

    //create and upload the new directroy
    s3dirent_t newdir[1];
    strcpy(newdir[0].name,".");
    newdir[0].type = 'd';
    newdir[0].st_size = sizeof(s3dirent_t);
    newdir[0].st_mode = mode;
    newdir[0].st_nlink = 1;
    newdir[0].st_uid = getuid();
    newdir[0].st_gid = getgid();
    newdir[0].atime = time(NULL);
    newdir[0].mtime = time(NULL);
    newdir[0].ctime = time(NULL);
    
    s3fs_put_object(getenv(S3BUCKET),path, (uint8_t*) newdir, newdir[0].st_size);
    
    
    
    
    
    
    //create new updated parent directory to replace the old outdated one 
    uint8_t * buff;
    int rv = s3fs_get_object(getenv(S3BUCKET), parent, &buff, 0, 0);
    int plength = (rv)/(sizeof(s3dirent_t));
    s3dirent_t * parentdir = (s3dirent_t*)buff;
    
    s3dirent_t newparent[plength+1];
    
    int item =0;
    for(;item<plength;item++)
    {
    	newparent[item]=parentdir[item];
    }
    
    strcpy(newparent[plength].name,key);
    newparent[plength].type='d';
    newparent[plength].st_mode = mode;
    cpymeta(newparent,parentdir,0);
    newparent[0].st_size=sizeof(newparent);
    newparent[0].mtime = newparent[0].atime; //keep it consistent
    
    s3fs_remove_object(getenv(S3BUCKET),parent);//ineffecient :)
    s3fs_put_object(getenv(S3BUCKET),parent, (uint8_t*) newparent, newparent[0].st_size); 
    

         

    free(parentdir);
    return 0;

	
	
    
}

/*
 * Remove a file.
 */
int fs_unlink(const char *path) {
    fprintf(stderr, "fs_unlink(path=\"%s\")\n", path);
    char key[1024];
    char parent[1024];
    char cpy[1024];
    strcpy(cpy,path);
    strcpy(key, basename(cpy));
    strcpy(parent,dirname(cpy));
    if (fs_findtype(path) != 2){//if not a file, return 0
        return -1;
    }
    s3fs_remove_object(getenv(S3BUCKET),path);//gone!
    //now update meta data!
     s3dirent_t *pdir;
     s3fs_get_object(getenv(S3BUCKET),parent, (uint8_t **) &pdir, 0, 0);
    int psize = (pdir[0].st_size)/(sizeof(s3dirent_t));
    s3dirent_t newparent[(psize-1)];
    int i =0;
    int j = 0;
    //resize parentdir
    for (;i<(psize-1); i++){
        if(!(strcmp(pdir[j].name,key))){
            j++;
        }
        else{
            newparent[i] = pdir[j];
        }
        j++;
    }
    newparent[0].st_size = sizeof(newparent);//update the size
    s3fs_put_object(getenv(S3BUCKET), parent,(uint8_t*) newparent, newparent[0].st_size);
    free(pdir); 
    return 0;
    
    
    
}

/*
 * Remove a directory. 
 */
int fs_rmdir(const char *path) {
    char key[1024];
    char parent[1024];
    char cpy[1024];
    strcpy(cpy,path);
    strcpy(key, basename(cpy));
    strcpy(parent,dirname(cpy));
    fprintf(stderr, "fs_rmdir(path=\"%s\")\n", path);
    s3context_t *ctx = GET_PRIVATE_DATA;
    //check that we are infact removing a dir
    if (fs_findtype(path) != 1){//if not a dir, return 0
        return -1;
    }
    //remove the dir
    s3dirent_t *dir;
    s3fs_get_object(getenv(S3BUCKET),path, (uint8_t **) &dir, 0, 0);
    int dsize = (dir[0].st_size)/(sizeof(s3dirent_t));
    if (dsize!=1){//make sure the dir is of size one
        return -1;
    }
    s3fs_remove_object(getenv(S3BUCKET),path);//remove this object from the cloud
    //update parent dir
    s3dirent_t *pdir;
    s3fs_get_object(getenv(S3BUCKET),parent, (uint8_t **) &pdir, 0, 0);
    int psize = (pdir[0].st_size)/(sizeof(s3dirent_t));
    s3dirent_t newparent[(psize-1)];
    int i =0;
    int j = 0;
    //resize parentdir
    for (;i<(psize-1); i++){
        if(!(strcmp(pdir[j].name,key))){
            j++;
        }
        else{
            newparent[i] = pdir[j];
        }
        j++;
    }
    newparent[0].st_size = sizeof(newparent);//update the size
    s3fs_put_object(getenv(S3BUCKET), parent,(uint8_t*) newparent, newparent[0].st_size);
    free(pdir);
    free(dir); 
    return 0;
}   
    

/*
 * Rename a file.
 */
int fs_rename(const char *path, const char *newpath) {
    fprintf(stderr, "fs_rename(fpath=\"%s\", newpath=\"%s\")\n", path, newpath);
    char file[1024];
    char parent[1024];
    char cpy[1024];
    strcpy(cpy,path);
    strcpy(file, basename(cpy));
    strcpy(parent,dirname(cpy));
    if (fs_findtype(path) != 2){//if not a file, return -1
        return -1;
    }
    uint8_t * pbuff;//get directory
    int prv = s3fs_get_object(getenv(S3BUCKET), parent, &pbuff, 0, 0);
    int psize = (prv)/(sizeof(s3dirent_t));
    s3dirent_t * pdir = (s3dirent_t*)pbuff;
    //find file for later to update metadata if needed
    int j = 0;
    int fildex;
    for(;j<psize;j++){
        if (!strcmp(pdir[j].name,file)){
            fildex = j;
        }
    }
    fs_mknod(newpath, pdir[fildex].st_mode, 0);
    fs_unlink(path);
    s3context_t *ctx = GET_PRIVATE_DATA;
    free(pdir);
    return 0;
}


/*
 * Change the permission bits of a file.
 */
int fs_chmod(const char *path, mode_t mode) {
    fprintf(stderr, "fs_chmod(fpath=\"%s\", mode=0%03o)\n", path, mode);
    s3context_t *ctx = GET_PRIVATE_DATA;
    return -EIO;
}

/*
 * Change the owner and group of a file.
 */
int fs_chown(const char *path, uid_t uid, gid_t gid) {
    fprintf(stderr, "fs_chown(path=\"%s\", uid=%d, gid=%d)\n", path, uid, gid);
    s3context_t *ctx = GET_PRIVATE_DATA;
    return -EIO;
}

/*
 * Change the size of a file.
 */
int fs_truncate(const char *path, off_t newsize) {
    fprintf(stderr, "fs_truncate(path=\"%s\", newsize=%d)\n", path, (int)newsize);
    char file[1024];
    char parent[1024];
    char cpy[1024];
    strcpy(cpy,path);
    strcpy(file, basename(cpy));
    strcpy(parent,dirname(cpy));
    if (fs_findtype(path) != 2){//if not a file, return -1
        return -1;
    }
    uint8_t * pbuff;//get directory
    int prv = s3fs_get_object(getenv(S3BUCKET), parent, &pbuff, 0, 0);
    int psize = (prv)/(sizeof(s3dirent_t));
    s3dirent_t * pdir = (s3dirent_t*)pbuff;
    //find file for later to update metadata if needed
    int j = 0;
    int fildex;
    for(;j<psize;j++){
        if (!strcmp(pdir[j].name,file)){
            fildex = j;
        }
    }
    mode_t mode = pdir[fildex].st_mode = mode;//get necessary metadata
    unlink(path);//remove
    mknod(path, mode, 0);//and put back
    return 0;
}

/*
 * Change the access and/or modification times of a file. 
 */
int fs_utime(const char *path, struct utimbuf *ubuf) {
    fprintf(stderr, "fs_utime(path=\"%s\")\n", path);
    s3context_t *ctx = GET_PRIVATE_DATA;
    return -EIO;
}


/* 
 * File open operation
 * No creation, or truncation flags (O_CREAT, O_EXCL, O_TRUNC)
 * will be passed to open().  Open should check if the operation
 * is permitted for the given flags.  
 * 
 * Optionally open may also return an arbitrary filehandle in the 
 * fuse_file_info structure (fi->fh).
 * which will be passed to all file operations.
 * (In stages 1 and 2, you are advised to keep this function very,
 * very simple.)
 */
int fs_open(const char *path, struct fuse_file_info *fi) {
    fprintf(stderr, "fs_opendir(path=\"%s\")\n\n", path);
    s3context_t *ctx = GET_PRIVATE_DATA;
    
	if( fs_findtype(path) !=2 ){
        return -EIO;
    }
    return 0;
}


/* 
 * Read data from an open file
 *
 * Read should return exactly the number of bytes requested except
 * on EOF or error, otherwise the rest of the data will be
 * substituted with zeroes.  
 */
int fs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    fprintf(stderr, "fs_read(path=\"%s\", buf=%p, size=%d, offset=%d)\n",
	        path, buf, (int)size, (int)offset);
    s3context_t *ctx = GET_PRIVATE_DATA;
    if( fs_findtype(path) !=2 ){
        return -EIO;
    }
    char file[1024];
    char parent[1024];
    char cpy[1024];
    strcpy(cpy,path);
    strcpy(file, basename(cpy));
    strcpy(parent,dirname(cpy));
    uint8_t * buff;
    int rv = s3fs_get_object(getenv(S3BUCKET), path, &buff, 0, 0);
    int i = 0;
    int bytes = 0;
    for (;i<size;i++){
        if ((i+offset) >= size){
            buf[i] = 0;
        }
        else{
            buf[i] = buff[i+offset]; 
            bytes++;
        }
    }
    free(buff);  
    return bytes;
}

/*
 * Write data to an open file
 *
 * Write should return exactly the number of bytes requested
 * except on error.
 */
int fs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    fprintf(stderr, "fs_write(path=\"%s\", buf=%p, size=%d, offset=%d)\n",path, buf, (int)size, (int)offset);
    if( fs_findtype(path) !=2 ){
        return -EIO;
    }
    char file[1024];
    char parent[1024];
    char cpy[1024];
    strcpy(cpy,path);
    strcpy(file, basename(cpy));
    strcpy(parent,dirname(cpy));
    uint8_t * buff;
    int rv = s3fs_get_object(getenv(S3BUCKET), path, &buff, 0, 0);
    uint8_t * pbuff;//get directory
    int prv = s3fs_get_object(getenv(S3BUCKET), parent, &pbuff, 0, 0);
    int psize = (prv)/(sizeof(s3dirent_t));
    s3dirent_t * pdir = (s3dirent_t*)pbuff;
    //find file for later to update metadata if needed
    int j = 0;
    int fildex;
    for(;j<psize;j++){
        if (!strcmp(pdir[j].name,file)){
            fildex = j;
        }
    }
    fprintf(stderr,"\n%s\n",pdir[fildex].name);
    int i = 0;
    int bytes = 0;
    if ((size+offset)>rv){
        fprintf(stderr,"\ningrower\n");
        uint8_t * newbuff = malloc( sizeof(uint8_t)*(size+offset));
        fprintf(stderr,"\nmalloced\n");
        for (;i<size+offset;i++){
            if (i<offset){
                newbuff[i] = buff[i];
            } else{
                newbuff[i] = buf[i]; 
            }bytes++;
        }
        fprintf(stderr,"\nloop done\n");
        pdir[fildex].st_size = (size+offset);
        s3fs_put_object(getenv(S3BUCKET),path,(uint8_t*) newbuff, pdir[fildex].st_size);
        s3fs_put_object(getenv(S3BUCKET),parent,(uint8_t*) pdir, pdir[0].st_size);
        free(buff);
        free(pbuff);
        free(newbuff);
        fprintf(stderr,"\nall done\n");
        return bytes;
            
    }else{
        for (;i<size;i++){
            buff[i+offset] = buf[i];
            bytes++;
         }
    fprintf(stderr, "im thoght the loop\n%d\n%d\n",bytes, sizeof(buff));
    s3fs_put_object(getenv(S3BUCKET),path,(uint8_t*) buff, rv);
    fprintf(stderr, "file up\n");
    //pdir[fildex].st_size = sizeof(newbuff);
    //fprintf(stderr, "size adjusted\n");
    s3fs_put_object(getenv(S3BUCKET),path,(uint8_t*) pdir, sizeof(pdir));
    fprintf(stderr, "dir up\n");
    free(buff);
    free(pbuff);
    //free(newbuff);         
    return bytes;
    }
}


/* 
 * Possibly flush cached data for one file.
 *
 * Flush is called on each close() of a file descriptor.  So if a
 * filesystem wants to return write errors in close() and the file
 * has cached dirty data, this is a good place to write back data
 * and return any errors.  Since many applications ignore close()
 * errors this is not always useful.
 */
int fs_flush(const char *path, struct fuse_file_info *fi) {
    fprintf(stderr, "fs_flush(path=\"%s\", fi=%p)\n", path, fi);
    s3context_t *ctx = GET_PRIVATE_DATA;
    return 0; //returning 0 so it doesnt yell at me for calls like touch
}

/*
 * Release an open file
 *
 * Release is called when there are no more references to an open
 * file: all file descriptors are closed and all memory mappings
 * are unmapped.  
 *
 * For every open() call there will be exactly one release() call
 * with the same flags and file descriptor.  It is possible to
 * have a file opened more than once, in which case only the last
 * release will mean, that no more reads/writes will happen on the
 * file.  The return value of release is ignored.
 */
int fs_release(const char *path, struct fuse_file_info *fi) {//do nothing!
    fprintf(stderr, "fs_release(path=\"%s\")\n", path);
    return 0;
}

/*
 * Synchronize file contents; any cached data should be written back to 
 * stable storage.
 */
int fs_fsync(const char *path, int datasync, struct fuse_file_info *fi) {
    fprintf(stderr, "fs_fsync(path=\"%s\")\n", path);
    return 0;
}

/*
 * Open directory
 *
 * This method should check if the open operation is permitted for
 * this directory
 */
int fs_opendir(const char *path, struct fuse_file_info *fi) {
    fprintf(stderr, "fs_opendir(path=\"%s\")\n\n", path);
    s3context_t *ctx = GET_PRIVATE_DATA;
    
	if( fs_findtype(path) !=1 ){
        return -EIO;
    }
    return 0;
    
}

/*
 * Read directory.  See the project description for how to use the filler
 * function for filling in directory items.
 */
int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
	       struct fuse_file_info *fi)
{
    fprintf(stderr, "fs_readdir(path=\"%s\", buf=%p, offset=%d)\n",
	        path, buf, (int)offset);
    s3context_t *ctx = GET_PRIVATE_DATA;
    if( fs_findtype(path) !=1 ){
        return -EIO;
    }
    uint8_t * buff;
    int rv = s3fs_get_object(getenv(S3BUCKET), path, &buff, 0, 0);
    int dirsize = (rv)/(sizeof(s3dirent_t));
    s3dirent_t * dir = (s3dirent_t*)buff;
    int i = 0;
    for (; i < dirsize; i++) {
        // call filler function to fill in directory name
        // to the supplied buffer
        if (filler(buf, dir[i].name, NULL, 0) != 0) {
            free(dir);            
            return -ENOMEM;
        }
    }
    free(dir);
    return 0;
}

/*
 * Release directory.
 */
int fs_releasedir(const char *path, struct fuse_file_info *fi) {
    fprintf(stderr, "fs_releasedir(path=\"%s\")\n", path);
    s3context_t *ctx = GET_PRIVATE_DATA;
    return 0;
}

/*
 * Synchronize directory contents; cached data should be saved to 
 * stable storage.
 */
int fs_fsyncdir(const char *path, int datasync, struct fuse_file_info *fi) {
    fprintf(stderr, "fs_fsyncdir(path=\"%s\")\n", path);
    s3context_t *ctx = GET_PRIVATE_DATA;
    return -EIO;
}

/*
 * Initialize the file system.  This is called once upon
 * file system startup.
 */
void *fs_init(struct fuse_conn_info *conn)
{
    fprintf(stderr, "fs_init --- initializing file system.\n");
    s3context_t *ctx = GET_PRIVATE_DATA;
    //erase old directory//
    s3fs_clear_bucket(getenv(S3BUCKET));
    //create new root dir
    s3dirent_t root[1];  
    strcpy(root[0].name,".");
    root[0].type = 'd';
    root[0].st_size = sizeof(s3dirent_t);
    root[0].st_mode = (S_IFDIR | S_IRUSR | S_IWUSR | S_IXUSR);
    root[0].st_nlink = 1;
    root[0].st_uid = getuid();
    root[0].st_gid = getgid();
    root[0].atime = time(NULL);
    root[0].mtime = time(NULL);
    root[0].ctime = time(NULL);
    //fs_updatedir(root);
    //to the cloud!
    ssize_t rv = s3fs_put_object(getenv(S3BUCKET),"/", (uint8_t*) root, sizeof(root)); 
    fprintf(stderr, "\n%d\n%d\n%d\n",sizeof(root),(root[0].st_size),rv);
    return 0;

}

/*
 * Clean up filesystem -- free any allocated data.
 * Called once on filesystem exit.
 */
void fs_destroy(void *userdata) {
    fprintf(stderr, "fs_destroy --- shutting down file system.\n");
    free(userdata);
    int val = s3fs_clear_bucket(getenv(S3BUCKET));
    
    
}

/*
 * Check file access permissions.  For now, just return 0 (success!)
 * Later, actually check permissions (don't bother initially).
 */
int fs_access(const char *path, int mask) {
    fprintf(stderr, "fs_access(path=\"%s\", mask=0%o)\n", path, mask);
    s3context_t *ctx = GET_PRIVATE_DATA;
    return 0;
}

/*
 * Change the size of an open file.  Very similar to fs_truncate (and,
 * depending on your implementation), you could possibly treat it the
 * same as fs_truncate.
 */
int fs_ftruncate(const char *path, off_t offset, struct fuse_file_info *fi) {
    fprintf(stderr, "fs_ftruncate(path=\"%s\", offset=%d)\n", path, (int)offset);
    return truncate(path, offset);
}

/*
 * The struct that contains pointers to all our callback
 * functions.  Those that are currently NULL aren't 
 * intended to be implemented in this project.
 */
struct fuse_operations s3fs_ops = {
  .getattr     = fs_getattr,    // get file attributes
  .readlink    = NULL,          // read a symbolic link
  .getdir      = NULL,          // deprecated function
  .mknod       = fs_mknod,      // create a file
  .mkdir       = fs_mkdir,      // create a directory
  .unlink      = fs_unlink,     // remove/unlink a file
  .rmdir       = fs_rmdir,      // remove a directory
  .symlink     = NULL,          // create a symbolic link
  .rename      = fs_rename,     // rename a file
  .link        = NULL,          // we don't support hard links
  .chmod       = fs_chmod,      // change mode bits
  .chown       = fs_chown,      // change ownership
  .truncate    = fs_truncate,   // truncate a file's size
  .utime       = fs_utime,      // update stat times for a file
  .open        = fs_open,       // open a file
  .read        = fs_read,       // read contents from an open file
  .write       = fs_write,      // write contents to an open file
  .statfs      = NULL,          // file sys stat: not implemented
  .flush       = fs_flush,      // flush file to stable storage
  .release     = fs_release,    // release/close file
  .fsync       = fs_fsync,      // sync file to disk
  .setxattr    = NULL,          // not implemented
  .getxattr    = NULL,          // not implemented
  .listxattr   = NULL,          // not implemented
  .removexattr = NULL,          // not implemented
  .opendir     = fs_opendir,    // open directory entry
  .readdir     = fs_readdir,    // read directory entry
  .releasedir  = fs_releasedir, // release/close directory
  .fsyncdir    = fs_fsyncdir,   // sync dirent to disk
  .init        = fs_init,       // initialize filesystem
  .destroy     = fs_destroy,    // cleanup/destroy filesystem
  .access      = fs_access,     // check access permissions for a file
  .create      = NULL,          // not implemented
  .ftruncate   = fs_ftruncate,  // truncate the file
  .fgetattr    = NULL           // not implemented
};



/* 
 * You shouldn't need to change anything here.  If you need to
 * add more items to the filesystem context object (which currently
 * only has the S3 bucket name), you might want to initialize that
 * here (but you could also reasonably do that in fs_init).
 */
int main(int argc, char *argv[]) {
    // don't allow anything to continue if we're running as root.  bad stuff.
    if ((getuid() == 0) || (geteuid() == 0)) {
    	fprintf(stderr, "Don't run this as root.\n");
    	return -1;
    }
    s3context_t *stateinfo = malloc(sizeof(s3context_t));
    memset(stateinfo, 0, sizeof(s3context_t));

    char *s3key = getenv(S3ACCESSKEY);
    if (!s3key) {
        fprintf(stderr, "%s environment variable must be defined\n", S3ACCESSKEY);
    }
    char *s3secret = getenv(S3SECRETKEY);
    if (!s3secret) {
        fprintf(stderr, "%s environment variable must be defined\n", S3SECRETKEY);
    }
    char *s3bucket = getenv(S3BUCKET);
    if (!s3bucket) {
        fprintf(stderr, "%s environment variable must be defined\n", S3BUCKET);
    }
    strncpy((*stateinfo).s3bucket, s3bucket, BUFFERSIZE);

    fprintf(stderr, "Initializing s3 credentials\n");
    s3fs_init_credentials(s3key, s3secret);

    fprintf(stderr, "Totally clearing s3 bucket\n");
    s3fs_clear_bucket(s3bucket);

    fprintf(stderr, "Starting up FUSE file system.\n");
    int fuse_stat = fuse_main(argc, argv, &s3fs_ops, stateinfo);
    fprintf(stderr, "Startup function (fuse_main) returned %d\n", fuse_stat);
    
    return fuse_stat;
}

