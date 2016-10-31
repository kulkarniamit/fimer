#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
int main(int argc, char **argv){
    if(argc != 2){
        /* Filepath missing */
        fprintf(stderr, "Missing file argument\n");
        exit(1);
    }
    else{
        printf("Permissions will be changed for %s\n",argv[1]);
    }
    
    char filepath[256];
    strcpy(filepath, argv[1]);
    char mode[] = "0777";
    char buf[100] = "testfile";
    //    int i;
    //    i = strtol(mode, 0, 8);
    //    if (chmod (buf,i) < 0)
    struct stat *statpointer = malloc(sizeof(struct stat));
    if (chmod (filepath,S_IRWXU) < 0){
        /* Unable to change permissions */
        fprintf(stderr, "%s: error in chmod(%s, %s) - %d (%s)\n",
        argv[0], buf, mode, errno, strerror(errno));
        exit(1);
    }
    else{
        if(stat(filepath, statpointer) == 0){
            int size = statpointer->st_size;
            printf("Size of \"%s\" is %d bytes.\n", filepath, size);
            printf("File Permissions: \t");
            printf( (S_ISDIR(statpointer->st_mode)) ? "d" : "-");
            printf( (statpointer->st_mode & S_IRUSR) ? "r" : "-");
            printf( (statpointer->st_mode & S_IWUSR) ? "w" : "-");
            printf( (statpointer->st_mode & S_IXUSR) ? "x" : "-");
            printf( (statpointer->st_mode & S_IRGRP) ? "r" : "-");
            printf( (statpointer->st_mode & S_IWGRP) ? "w" : "-");
            printf( (statpointer->st_mode & S_IXGRP) ? "x" : "-");
            printf( (statpointer->st_mode & S_IROTH) ? "r" : "-");
            printf( (statpointer->st_mode & S_IWOTH) ? "w" : "-");
            printf( (statpointer->st_mode & S_IXOTH) ? "x" : "-");
            printf("\n");
        }
    }
    return(0);
}

