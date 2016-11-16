#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>


#define osErrorFatal(userMsg) osErrorFatalImpl((userMsg), __FILE__, __func__, __LINE__)
#define osAssert(expr, userMsg) \
    do{ \
        if(!(expr)) \
            osErrorFatal(userMsg); \
    }while(0);

#define MAX_CMD_LEN (6)
#define MAX_NAME_LEN (30)
#define MAX_WORD_LEN (10)

/* DECLARATION */   
/*================================== */
// "os" is for name of subject - Operating systems, and then name of function
void osErrorFatalImpl(const char *userMsg, const char *fileName, const char *funcName, const int lineNum);
void osExit();
int mkfile(const char *fileName, int *fd);
bool osCatFile(const char *fileName);
bool osCp(const char *fileSrc, const char *fileDst);
bool osInsert(const char *word, const int position, const char *filePath);
bool osInfo(const char *fileName);
/*=================================== */

int main(){

    printf("%% ");

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, stdin)) != -1) {
        int bytesRead = 0;
        int totalBytesRead = 0;
        char Buf[MAX_CMD_LEN + 1];
        sscanf(line, "%s%n", Buf, &bytesRead);
        totalBytesRead += bytesRead;

        if(!strcmp(Buf, "exit"))
            osExit();
        else if(!strcmp(Buf, "mkfile")){
            char fileName[MAX_NAME_LEN + 1];
            sscanf(line + totalBytesRead, "%s%n", fileName, &bytesRead);
            totalBytesRead += bytesRead;
            int fd;
            if(mkfile(fileName, &fd) == 0)
                printf("File exist or mkfile failed\n");
            close(fd);
        }
        else if(!strcmp(Buf, "mkdir")){
            char dirName[MAX_NAME_LEN + 1];
            sscanf(line + totalBytesRead, "%s%n", dirName, &bytesRead);
            totalBytesRead += bytesRead;
            int oldMask = 0;
            oldMask = umask(0);
            mode_t defaultMode = 0755;
            if(mkdir(dirName, defaultMode) == -1)
                printf("mkdir error occuredd\n");
            umask(oldMask);
        }
        else if(!strcmp(Buf, "rm")){
            char filePath[MAX_NAME_LEN + 1];
            sscanf(line + totalBytesRead, "%s%n", filePath, &bytesRead);
            totalBytesRead += bytesRead;
            if(unlink(filePath) == -1)
                printf("rm failed\n");
        }
        else if(!strcmp(Buf, "rmdir")){
            char dirName[MAX_NAME_LEN + 1];
            sscanf(line + totalBytesRead, "%s%n", dirName, &bytesRead);
            totalBytesRead += bytesRead;

            if(rmdir(dirName) == -1)
                printf("rmdir failed\n");
        }
        else if(!strcmp(Buf, "cat")){
            char fileName[MAX_NAME_LEN + 1];
            sscanf(line + totalBytesRead, "%s%n", fileName, &bytesRead);
            totalBytesRead += bytesRead;

            if(osCatFile(fileName) == false)
                printf("Cat file failed\n");
        }
        else if(!strcmp(Buf, "cp")){
            char fileSrc[MAX_NAME_LEN + 1];
            char fileDst[MAX_NAME_LEN + 1];
            sscanf(line + totalBytesRead, "%s%n", fileSrc, &bytesRead);
            totalBytesRead += bytesRead;
            sscanf(line + totalBytesRead, "%s%n", fileDst, &bytesRead);
            totalBytesRead += bytesRead;

            if(osCp(fileSrc, fileDst) == false)
                printf("cp failed\n");
        }
        else if(!strcmp(Buf, "insert")){
            char word[MAX_WORD_LEN + 1];
            int position;
            char fileName[MAX_NAME_LEN + 1];

            sscanf(line + totalBytesRead, "%s%n", word, &bytesRead);
            totalBytesRead += bytesRead;
            sscanf(line + totalBytesRead, "%d%n", &position, &bytesRead);
            totalBytesRead += bytesRead;
            sscanf(line + totalBytesRead, "%s%n", fileName, &bytesRead);
            totalBytesRead += bytesRead;

            printf("%d\n", position);
            if(osInsert(word, position, fileName) == false)
                printf("insert failed\n");
        }
        else if(!strcmp(Buf, "info")){
            char fileName[MAX_NAME_LEN + 1];
            sscanf(line + totalBytesRead, "%s%n", fileName, &bytesRead);
            totalBytesRead += bytesRead;

            if(osInfo(fileName) == false)
                printf("info failed\n");
        }
        else{
            printf("Not an option\n");
        }
        printf("%% ");
    }

    free(line);
    return 0;
}

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

/* Function for processing errors */
void osErrorFatalImpl(const char *userMsg, const char *fileName, const char *funcName, const int lineNum){
    perror(userMsg);
    fprintf(stderr, "File: %s\nFunction: %s\n, Line: %d\n", fileName, funcName, lineNum);
    exit(EXIT_FAILURE);
}
/*============================================================*/

/* Function for exit */
void osExit(){
    exit(EXIT_SUCCESS);
}
/*============================================================*/

/* Function for making new file */
int mkfile(const char *fileName, int *fd){
    int oldMask = 0;
    oldMask = umask(0);
    mode_t defaultMode = 0644;
    if((*fd = open(fileName, O_CREAT | O_EXCL, defaultMode)) == -1)
        return 0;
    umask(oldMask);
    return *fd;
}
/*============================================================*/

/* Function for writing file to terminal */
bool osCatFile(const char *fileName){
    int oldMask = 0;
    oldMask = umask(0);
    mode_t defaultMode = 0644;
    int fd;
    if((fd = open(fileName, O_RDONLY | O_EXCL, defaultMode)) == -1)
        return false;
    umask(oldMask);

    uint32_t bufSize = 1 << 13U;
    char *buff = malloc(bufSize);
    int bytesReadFromRead = 0;

    if(buff == NULL){
        printf("Allocation failed\n");
        return false;
    }
    while((bytesReadFromRead = read(fd, buff, bufSize)) > 0){
        if(write(STDOUT_FILENO, buff, bytesReadFromRead) != bytesReadFromRead){
            free(buff);
            return false;
        }
    }
    close(fd);
    free(buff);
    return 0 == bytesReadFromRead;
}
/*============================================================*/

/* Function for copy file to file */
bool osCp(const char *fileSrc, const char *fileDst){
    int oldMask = 0;
    oldMask = umask(0);
    mode_t defaultMode = 0644;
    int fdSrc, fdDst;
    if((fdSrc = open(fileSrc, O_RDONLY | O_EXCL, defaultMode)) == -1)
        return false;
    if((fdDst = open(fileDst, O_WRONLY | O_CREAT | O_TRUNC, defaultMode)) == -1)
        return false;
    umask(oldMask);

    uint32_t bufSize = 1 << 13U;
    char *buff = malloc(bufSize);
    int bytesReadFromRead = 0;

    if(buff == NULL){
        printf("Allocation failed\n");
        return false;
    }
    while((bytesReadFromRead = read(fdSrc, buff, bufSize)) > 0){
        if(write(fdDst, buff, bytesReadFromRead) != bytesReadFromRead){
            free(buff);
            return false;
        }
    }
    free(buff);
    close(fdSrc);
    close(fdDst);
    return 0 == bytesReadFromRead;
}
/*============================================================*/

/* Function for insert word to file from a given position */
bool osInsert(const char *word, const int position, const char *filePath){
    int oldMask = 0;
    oldMask = umask(0);
    mode_t defaultMode = 0644;
    int fd;
    if((fd = open(filePath, O_RDWR | O_EXCL, defaultMode)) == -1)
        return false;
    umask(oldMask);

    printf("%d\n", position);

    if(lseek(fd, (off_t)position, SEEK_SET) == -1)
    {
        printf("Lseek failed\n");
        return false;
    }

    int l = strlen(word);
    if(write(fd, word, l) != l)
        return false;

    close(fd);
    return true;
}
/*============================================================*/

/* Function something like stat, that gives primary info about file (EVERYTHING IS FILE ON LINUX :D) */
bool osInfo(const char *fileName){
    struct stat fInfo;

    if(stat(fileName, &fInfo) == -1)
        return false;

    switch(fInfo.st_mode & S_IFMT){
        case(S_IFSOCK):
            printf("Type: socket \n");
            break;
        case(S_IFLNK):
            printf("Type: symbolic link \n");
            break;
        case(S_IFREG):
            printf("Type: regular file \n");
            break;
        case(S_IFBLK):
            printf("Type: block device \n");
            break;
        case(S_IFDIR):
            printf("Type: directory \n");
            break;
        case(S_IFCHR):
            printf("Type: character device \n");
            break;
        case(S_IFIFO):
            printf("Type: pipe \n");
            break;
        default:
            return false;
    }

    struct passwd *user = getpwuid(fInfo.st_uid);
    if(user== NULL)
        return false;
    printf("Owner: %s\n", user->pw_name);

    struct group *group = getgrgid(fInfo.st_gid);
    if(group == NULL)
        return false;
    printf("Group: %s\n", group->gr_name);
    printf("Size: %ju\n", (uintmax_t)fInfo.st_size);

    int users = 0;
    int groups = 0;
    int others = 0;

    if(fInfo.st_mode & S_IRUSR)
        users += 4;
    if(fInfo.st_mode & S_IWUSR)
        users += 2;
    if(fInfo.st_mode & S_IXUSR)
        users += 1;
    if(fInfo.st_mode & S_IRGRP)
        groups += 4;
    if(fInfo.st_mode & S_IWGRP)
        groups += 2;
    if(fInfo.st_mode & S_IXGRP)
        groups += 1;

    if(fInfo.st_mode & S_IROTH)
        others += 4;
    if(fInfo.st_mode & S_IWOTH)
        others += 2;
    if(fInfo.st_mode & S_IXOTH)
        others += 1;

    printf("Access: 0%d%d%d\n", users, groups, others);
    return true;
}
/*============================================================*/