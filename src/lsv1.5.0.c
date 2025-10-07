/*
 * Feature-6: ls-v1.5.0 – Colorized Output Based on File Type
 * -----------------------------------------------------------
 * This version enhances the ls utility with colorized output based on file type.
 * 
 * Features:
 *   - Blue for directories
 *   - Green for executables
 *   - Red for compressed/archived files (.tar, .gz, .zip)
 *   - Pink for symbolic links
 *   - Reverse video for special files (devices, sockets, etc.)
 * 
 * Concepts: stat(), ANSI escape codes, string comparison, qsort(), command-line parsing
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>

#include "../include/colors.h"  // color macros

#define SPACING 2  // spaces between columns

// --------------------------- ENUM FOR DISPLAY MODES --------------------------- //
typedef enum {
    MODE_DEFAULT,
    MODE_HORIZONTAL
} DisplayMode;

// --------------------------- HELPER FUNCTIONS --------------------------- //

// Compare two filenames alphabetically (for qsort)
int compare_filenames(const void *a, const void *b) {
    const char *fileA = *(const char **)a;
    const char *fileB = *(const char **)b;
    return strcmp(fileA, fileB);
}

// Get terminal width, with fallback if ioctl fails
int get_terminal_width() {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1)
        return 80;  // fallback default
    return w.ws_col;
}

// Determine color code based on file type
const char* get_color(const char *path, const char *filename) {
    static char fullpath[PATH_MAX];
    struct stat fileStat;

    snprintf(fullpath, sizeof(fullpath), "%s/%s", path, filename);

    // Use lstat to correctly identify symbolic links
    if (lstat(fullpath, &fileStat) == -1)
        return COLOR_RESET;

    if (S_ISDIR(fileStat.st_mode))
        return COLOR_BLUE;
    else if (S_ISLNK(fileStat.st_mode))
        return COLOR_PINK;
    else if (S_ISCHR(fileStat.st_mode) || S_ISBLK(fileStat.st_mode) ||
             S_ISSOCK(fileStat.st_mode) || S_ISFIFO(fileStat.st_mode))
        return COLOR_REVERSE;
    else if (fileStat.st_mode & S_IXUSR)
        return COLOR_GREEN;
    else if (strstr(filename, ".tar") || strstr(filename, ".gz") || strstr(filename, ".zip"))
        return COLOR_RED;

    return COLOR_RESET;
}

// Print filename with color
void print_colored(const char *path, const char *filename, int width) {
    const char *color = get_color(path, filename);
    printf("%s%-*s%s", color, width, filename, COLOR_RESET);
}

// --------------------------- DISPLAY FUNCTIONS --------------------------- //

// Default mode: print "down then across"
void print_down_then_across(char **names, int count, const char *path) {
    int term_width = get_terminal_width();

    // Find longest filename
    int max_len = 0;
    for (int i = 0; i < count; i++)
        if ((int)strlen(names[i]) > max_len)
            max_len = strlen(names[i]);

    int col_width = max_len + SPACING;
    int num_cols = term_width / col_width;
    if (num_cols < 1) num_cols = 1;
    int num_rows = (count + num_cols - 1) / num_cols;

    for (int row = 0; row < num_rows; row++) {
        for (int col = 0; col < num_cols; col++) {
            int index = col * num_rows + row;
            if (index < count)
                print_colored(path, names[index], col_width);
        }
        printf("\n");
    }
}

// Horizontal mode (-x): print "across then down"
void print_across(char **names, int count, const char *path) {
    int term_width = get_terminal_width();

    int max_len = 0;
    for (int i = 0; i < count; i++)
        if ((int)strlen(names[i]) > max_len)
            max_len = strlen(names[i]);

    int col_width = max_len + SPACING;
    int current_width = 0;

    for (int i = 0; i < count; i++) {
        if (current_width + col_width > term_width) {
            printf("\n");
            current_width = 0;
        }
        print_colored(path, names[i], col_width);
        current_width += col_width;
    }
    printf("\n");
}

// --------------------------- MAIN DIRECTORY LOGIC --------------------------- //
void list_directory(const char *path, DisplayMode mode) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    char **filenames = NULL;
    int count = 0;

    // Read all filenames (skip hidden)
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        filenames = realloc(filenames, (count + 1) * sizeof(char *));
        filenames[count] = strdup(entry->d_name);
        count++;
    }

    closedir(dir);

    // Sort filenames alphabetically
    qsort(filenames, count, sizeof(char *), compare_filenames);

    // Display based on selected mode
    if (mode == MODE_HORIZONTAL)
        print_across(filenames, count, path);
    else
        print_down_then_across(filenames, count, path);

    // Free memory
    for (int i = 0; i < count; i++)
        free(filenames[i]);
    free(filenames);
}

// --------------------------- MAIN FUNCTION --------------------------- //
int main(int argc, char *argv[]) {
    DisplayMode mode = MODE_DEFAULT;
    const char *path = ".";
    int opt;

    // Parse command-line options
    while ((opt = getopt(argc, argv, "x")) != -1) {
        switch (opt) {
            case 'x':
                mode = MODE_HORIZONTAL;
                break;
            default:
                fprintf(stderr, "Usage: %s [-x] [path]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // Optional directory argument
    if (optind < argc)
        path = argv[optind];

    // List directory contents
    list_directory(path, mode);
    return 0;
}
