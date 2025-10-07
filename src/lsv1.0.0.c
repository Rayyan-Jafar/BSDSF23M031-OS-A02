#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define SPACING 2 // spaces between columns

// Function to compare strings (used for sorting)
int compare(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

// Function to get terminal width
int get_terminal_width() {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
        // fallback if ioctl fails
        return 80;
    }
    return w.ws_col;
}

// Function to print files in column format ("down then across")
void print_in_columns(char **names, int count) {
    int term_width = 80;
    struct winsize ws;

    // Get terminal width if possible
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0)
        term_width = ws.ws_col;

    // Find longest name length
    int max_len = 0;
    for (int i = 0; i < count; i++) {
        int len = strlen(names[i]);
        if (len > max_len) max_len = len;
    }

    // Compute number of columns
    int col_width = max_len + 2;
    int num_cols = term_width / col_width;
    if (num_cols < 1) num_cols = 1;

    // Compute number of rows
    int num_rows = (count + num_cols - 1) / num_cols;

    // Print vertically first
    for (int row = 0; row < num_rows; row++) {
        for (int col = 0; col < num_cols; col++) {
            int index = col * num_rows + row;
            if (index < count)
                printf("%-*s", col_width, names[index]);
        }
        printf("\n");
    }
}

// Function to list directory contents
void list_directory(const char *path) {
    DIR *dir;
    struct dirent *entry;

    dir = opendir(path);
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    // Collect all filenames dynamically
    char **filenames = NULL;
    int count = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue; // skip hidden files
        filenames = realloc(filenames, (count + 1) * sizeof(char *));
        filenames[count] = strdup(entry->d_name);
        count++;
    }

    closedir(dir);

    // Sort filenames alphabetically
    qsort(filenames, count, sizeof(char *), compare);

    // Print them in columns
    print_in_columns(filenames, count);

    // Free memory
    for (int i = 0; i < count; i++) free(filenames[i]);
    free(filenames);
}

int main(int argc, char *argv[]) {
    const char *path = ".";
    if (argc > 1) path = argv[1];

    list_directory(path);
    return 0;
}
