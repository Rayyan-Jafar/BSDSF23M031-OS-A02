/*
 * ls-v1.6.0 – Complete LS Utility with Recursive Listing, Horizontal Display,
 * Alphabetical Sort, and Colorized Output
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/stat.h>
#include <errno.h>

// --------------------------- CONFIG --------------------------- //
#define SPACING 2   // space between columns
#define MAX_PATH 1024

// --------------------------- COLOR MACROS --------------------------- //
#define COLOR_RESET   "\033[0m"
#define COLOR_BLUE    "\033[0;34m"  // directories
#define COLOR_GREEN   "\033[0;32m"  // executables
#define COLOR_RED     "\033[0;31m"  // archives
#define COLOR_PINK    "\033[0;35m"  // symlinks
#define COLOR_REVERSE "\033[7m"     // special files

// --------------------------- ENUM FOR DISPLAY MODES --------------------------- //
typedef enum {
    MODE_DEFAULT,
    MODE_HORIZONTAL
} DisplayMode;

// --------------------------- UTILITY FUNCTIONS --------------------------- //

// Compare filenames alphabetically for qsort
int compare_filenames(const void *a, const void *b) {
    const char *fileA = *(const char **)a;
    const char *fileB = *(const char **)b;
    return strcmp(fileA, fileB);
}

// Get terminal width, fallback to 80
int get_terminal_width() {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1)
        return 80;
    return w.ws_col;
}

// Check if file is executable
int is_executable(mode_t mode) {
    return (mode & S_IXUSR) || (mode & S_IXGRP) || (mode & S_IXOTH);
}

// Check if file is archive
int is_archive(const char *name) {
    return strstr(name, ".tar") || strstr(name, ".gz") || strstr(name, ".zip");
}

// Print filename with appropriate color
void print_colored(const char *path, const char *name) {
    struct stat st;
    if (lstat(path, &st) == -1) {
        printf("%s  ", name);
        return;
    }

    if (S_ISDIR(st.st_mode))
        printf(COLOR_BLUE "%s" COLOR_RESET "  ", name);
    else if (S_ISLNK(st.st_mode))
        printf(COLOR_PINK "%s" COLOR_RESET "  ", name);
    else if (is_executable(st.st_mode))
        printf(COLOR_GREEN "%s" COLOR_RESET "  ", name);
    else if (is_archive(name))
        printf(COLOR_RED "%s" COLOR_RESET "  ", name);
    else if (S_ISCHR(st.st_mode) || S_ISBLK(st.st_mode) || S_ISSOCK(st.st_mode) || S_ISFIFO(st.st_mode))
        printf(COLOR_REVERSE "%s" COLOR_RESET "  ", name);
    else
        printf("%s  ", name);
}

// --------------------------- DISPLAY FUNCTIONS --------------------------- //

// Down-then-across display
void print_down_then_across(char **names, int count, const char *base_path) {
    int term_width = get_terminal_width();
    int max_len = 0;
    for (int i = 0; i < count; i++) {
        int len = strlen(names[i]);
        if (len > max_len) max_len = len;
    }
    int col_width = max_len + SPACING;
    int num_cols = term_width / col_width;
    if (num_cols < 1) num_cols = 1;
    int num_rows = (count + num_cols - 1) / num_cols;

    for (int row = 0; row < num_rows; row++) {
        for (int col = 0; col < num_cols; col++) {
            int index = col * num_rows + row;
            if (index < count) {
                char fullpath[MAX_PATH];
                snprintf(fullpath, sizeof(fullpath), "%s/%s", base_path, names[index]);
                print_colored(fullpath, names[index]);
                int pad = col_width - strlen(names[index]);
                for (int p = 0; p < pad; p++) printf(" ");
            }
        }
        printf("\n");
    }
}

// Horizontal display (-x)
void print_across(char **names, int count, const char *base_path) {
    int term_width = get_terminal_width();
    int max_len = 0;
    for (int i = 0; i < count; i++) {
        int len = strlen(names[i]);
        if (len > max_len) max_len = len;
    }
    int col_width = max_len + SPACING;
    int current_width = 0;

    for (int i = 0; i < count; i++) {
        char fullpath[MAX_PATH];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", base_path, names[i]);
        if (current_width + col_width > term_width) {
            printf("\n");
            current_width = 0;
        }
        print_colored(fullpath, names[i]);
        int pad = col_width - strlen(names[i]);
        for (int p = 0; p < pad; p++) printf(" ");
        current_width += col_width;
    }
    printf("\n");
}

// --------------------------- CORE LS FUNCTION --------------------------- //
void do_ls(const char *path, DisplayMode mode, int recursive_flag) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return;
    }

    printf("%s:\n", path);

    struct dirent *entry;
    char **filenames = NULL;
    int count = 0;

    // Read directory entries
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        filenames = realloc(filenames, (count + 1) * sizeof(char *));
        filenames[count] = strdup(entry->d_name);
        count++;
    }
    closedir(dir);

    // Sort alphabetically
    qsort(filenames, count, sizeof(char *), compare_filenames);

    // Display files
    if (mode == MODE_HORIZONTAL)
        print_across(filenames, count, path);
    else
        print_down_then_across(filenames, count, path);

    // Recursive traversal
    if (recursive_flag) {
        for (int i = 0; i < count; i++) {
            char fullpath[MAX_PATH];
            snprintf(fullpath, sizeof(fullpath), "%s/%s", path, filenames[i]);

            struct stat st;
            if (lstat(fullpath, &st) == -1) continue;

            if (S_ISDIR(st.st_mode) && strcmp(filenames[i], ".") != 0 && strcmp(filenames[i], "..") != 0) {
                printf("\n");
                do_ls(fullpath, mode, recursive_flag);
            }
        }
    }

    // Free memory
    for (int i = 0; i < count; i++) free(filenames[i]);
    free(filenames);
}

// --------------------------- MAIN --------------------------- //
int main(int argc, char *argv[]) {
    DisplayMode mode = MODE_DEFAULT;
    int recursive_flag = 0;
    const char *path = ".";
    int opt;

    while ((opt = getopt(argc, argv, "xR")) != -1) {
        switch (opt) {
            case 'x':
                mode = MODE_HORIZONTAL;
                break;
            case 'R':
                recursive_flag = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-x] [-R] [path]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (optind < argc)
        path = argv[optind];

    do_ls(path, mode, recursive_flag);
    return 0;
}
