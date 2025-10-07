/*
 * Feature-5: ls-v1.4.0 – Alphabetical Sort
 * -----------------------------------------
 * This version adds alphabetical sorting to the ls utility using qsort().
 * It works for all display modes: default (down then across) and horizontal (-x).
 * 
 * Concepts: qsort(), dynamic memory allocation, command-line parsing, terminal width formatting
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <getopt.h>

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

// --------------------------- DISPLAY FUNCTIONS --------------------------- //

// Default mode: print "down then across"
void print_down_then_across(char **names, int count) {
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

    // Print vertically first (down then across)
    for (int row = 0; row < num_rows; row++) {
        for (int col = 0; col < num_cols; col++) {
            int index = col * num_rows + row;
            if (index < count)
                printf("%-*s", col_width, names[index]);
        }
        printf("\n");
    }
}

// Horizontal mode (-x): print "across then down"
void print_across(char **names, int count) {
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
        printf("%-*s", col_width, names[i]);
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
        print_across(filenames, count);
    else
        print_down_then_across(filenames, count);

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
