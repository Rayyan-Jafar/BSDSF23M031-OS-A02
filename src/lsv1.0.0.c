#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <getopt.h>

#define SPACING 2

typedef enum {
    MODE_DEFAULT,
    MODE_LONG,
    MODE_HORIZONTAL
} DisplayMode;

int compare(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

int get_terminal_width() {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1)
        return 80;
    return w.ws_col;
}

/* --- Feature-3: Down Then Across --- */
void print_down_then_across(char **names, int count) {
    int term_width = get_terminal_width();

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
                printf("%-*s", col_width, names[index]);
        }
        printf("\n");
    }
}

/* --- Feature-4: Horizontal (Across) --- */
void print_across(char **names, int count) {
    int term_width = get_terminal_width();

    int max_len = 0;
    for (int i = 0; i < count; i++)
        if ((int)strlen(names[i]) > max_len)
            max_len = strlen(names[i]);

    int col_width = max_len + SPACING;
    int current_width = 0;

    for (int i = 0; i < count; i++) {
        //int len = strlen(names[i]);
        if (current_width + col_width > term_width) {
            printf("\n");
            current_width = 0;
        }
        printf("%-*s", col_width, names[i]);
        current_width += col_width;
    }
    printf("\n");
}

/* --- Directory Listing --- */
void list_directory(const char *path, DisplayMode mode) {
    DIR *dir;
    struct dirent *entry;

    dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return;
    }

    char **filenames = NULL;
    int count = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        filenames = realloc(filenames, (count + 1) * sizeof(char *));
        filenames[count] = strdup(entry->d_name);
        count++;
    }

    closedir(dir);
    qsort(filenames, count, sizeof(char *), compare);

    // choose display mode
    if (mode == MODE_HORIZONTAL)
        print_across(filenames, count);
    else
        print_down_then_across(filenames, count);

    for (int i = 0; i < count; i++) free(filenames[i]);
    free(filenames);
}

int main(int argc, char *argv[]) {
    DisplayMode mode = MODE_DEFAULT;
    int opt;
    const char *path = ".";

    // Parse options
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

    if (optind < argc)
        path = argv[optind];

    list_directory(path, mode);
    return 0;
}
