/*
* Programming Assignment 02: lsv1.0.0
* This is the source file of version 1.0.0
* Read the write-up of the assignment to add the features to this base version
* Usage:
*       $ lsv1.0.0 
*       % lsv1.0.0  /home
*       $ lsv1.0.0  /home/kali/   /etc/
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

extern int errno;

void do_ls(const char *dir);
void list_long_format(const char *path);
void list_simple(const char *path);

void mode_to_string(mode_t mode, char *str) {
    // File type
    str[0] = S_ISDIR(mode) ? 'd' :
             S_ISLNK(mode) ? 'l' :
             S_ISCHR(mode) ? 'c' :
             S_ISBLK(mode) ? 'b' :
             S_ISFIFO(mode)? 'p' :
             S_ISSOCK(mode)? 's' : '-';

    // Owner permissions
    str[1] = (mode & S_IRUSR) ? 'r' : '-';
    str[2] = (mode & S_IWUSR) ? 'w' : '-';
    str[3] = (mode & S_IXUSR) ? 'x' : '-';

    // Group permissions
    str[4] = (mode & S_IRGRP) ? 'r' : '-';
    str[5] = (mode & S_IWGRP) ? 'w' : '-';
    str[6] = (mode & S_IXGRP) ? 'x' : '-';

    // Others permissions
    str[7] = (mode & S_IROTH) ? 'r' : '-';
    str[8] = (mode & S_IWOTH) ? 'w' : '-';
    str[9] = (mode & S_IXOTH) ? 'x' : '-';
    str[10] = '\0';
}

int main(int argc, char *argv[]) {
    int opt;
    int long_format = 0;

    while ((opt = getopt(argc, argv, "l")) != -1) {
        switch (opt) {
            case 'l':
                long_format = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-l] [directory]\n", argv[0]);
                return 1;
        }
    }

    // Optional: directory path argument (default = ".")
    const char *path = (optind < argc) ? argv[optind] : ".";

    if (long_format)
        list_long_format(path);
    else
        list_simple(path);

    return 0;
}

void list_long_format(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip hidden files (those starting with '.')
        if (entry->d_name[0] == '.')
            continue;

        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

        struct stat sb;
        if (lstat(fullpath, &sb) == -1) {
            perror("lstat");
            continue;
        }

        // 1️⃣ File type and permissions
        char perms[11];
        mode_to_string(sb.st_mode, perms);

        // 2️⃣ Link count
        nlink_t links = sb.st_nlink;

        // 3️⃣ Owner and group names
        struct passwd *pw = getpwuid(sb.st_uid);
        struct group  *gr = getgrgid(sb.st_gid);

        // 4️⃣ File size
        off_t size = sb.st_size;

        // 5️⃣ Modification time
        char timebuf[64];
        struct tm *tm = localtime(&sb.st_mtime);
        strftime(timebuf, sizeof(timebuf), "%b %e %H:%M", tm);

        // 6️⃣ Print output aligned like ls -l
        printf("%s %2lu %-8s %-8s %8ld %s %s\n",
               perms,
               (unsigned long)links,
               pw ? pw->pw_name : "unknown",
               gr ? gr->gr_name : "unknown",
               (long)size,
               timebuf,
               entry->d_name);
    }

    closedir(dir);
}

void list_simple(const char *path) {
    do_ls(path);
}

void do_ls(const char *dir)
{
    struct dirent *entry;
    DIR *dp = opendir(dir);
    if (dp == NULL)
    {
        fprintf(stderr, "Cannot open directory : %s\n", dir);
        return;
    }
    errno = 0;
    while ((entry = readdir(dp)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;
        printf("%s\n", entry->d_name);
    }

    if (errno != 0)
    {
        perror("readdir failed");
    }

    closedir(dp);
}
