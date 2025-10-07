## REPORT.md

### **Version 1.0.0 – Basic ls Implementation**

#### **Question:**

What are the basic steps involved in implementing a simplified version of the `ls` command?

#### **Answer:**

The basic implementation of `ls` involves three key steps:

1. **Reading directory entries:** Using the `opendir()` and `readdir()` functions to access all files in a directory.
2. **Filtering and displaying filenames:** Printing only visible files (skipping those beginning with `.`).
3. **Sorting output:** Sorting the filenames alphabetically for consistent display.
   This forms the foundation for later enhancements such as long listing, column display, and option parsing.

---

### **Version 1.1.0 – Long Listing Format (-l Option)**

#### **Question:**

What information does the long-listing format (`-l`) show, and how is it retrieved programmatically?

#### **Answer:**

The `-l` option in `ls` displays detailed information about each file, including permissions, number of links, owner, group, file size, and modification time.
This information is retrieved using the `stat()` system call, which provides a `struct stat` containing all file metadata. Functions such as `getpwuid()` and `getgrgid()` convert numeric IDs into readable names. The output is formatted to align each column, similar to the standard `ls -l` display.

---

### **Version 1.2.0 – Column Display (Down Then Across)**

#### **Question 1:**

Explain the general logic for printing items in a “down then across” columnar format. Why is a simple single loop through the list of filenames insufficient for this task?

#### **Answer:**

In a “down then across” format, filenames are arranged vertically first and then horizontally across columns.
To achieve this:

1. All filenames must be stored in memory first.
2. The total number of rows and columns is computed based on the terminal width and the maximum filename length.
3. Printing occurs **row by row**, using the index pattern `index = col * num_rows + row`.

A simple single loop cannot be used because it prints filenames sequentially, which results in a **row-major (horizontal)** order instead of the required **column-major (vertical)** layout.

---

#### **Question 2:**

What is the purpose of the `ioctl` system call in this context? What would be the limitations of your program if you only used a fixed-width fallback (e.g., 80 columns)?

#### **Answer:**

The `ioctl()` system call with `TIOCGWINSZ` retrieves the current terminal’s width and height dynamically. This allows the program to automatically adjust the number of columns to fit the terminal size.

If only a fixed width (like 80 columns) were used:

* The output would **not adapt** to terminal resizing.
* On narrow terminals, filenames could **overflow or wrap incorrectly**.
* On wide terminals, there would be **excessive unused space**.
  Thus, using `ioctl()` provides flexibility and ensures that the layout adjusts correctly to different environments.

---

### **Version 1.3.0 – Horizontal Column Display (-x Option)**

#### **Question 1:**

Compare the implementation complexity of the “down then across” (vertical) printing logic versus the “across” (horizontal) printing logic. Which one requires more pre-calculation and why?

#### **Answer:**

The “down then across” logic is more complex because it requires calculating both the **number of rows and columns**, then using index transformations (`col * num_rows + row`) to print items in the correct vertical order.
In contrast, the “across” (horizontal) format simply prints filenames **left to right** and wraps when the next filename would exceed the terminal width. Therefore, it needs only the column width and running position tracking — fewer pre-calculations overall.

---

#### **Question 2:**

Describe the strategy you used in your code to manage the different display modes (`-l`, `-x`, and default). How did your program decide which function to call for printing?

#### **Answer:**

The program uses a **display mode flag** (an integer or enum variable) that tracks the user’s selected output mode:

* Default mode → column display (“down then across”)
* `-l` flag → long-listing mode
* `-x` flag → horizontal column mode

Command-line arguments are parsed using `getopt()`. Based on the flag set, the main logic selects the appropriate display function:

```c
if (mode == LONG_LIST)
    print_long_listing();
else if (mode == HORIZONTAL)
    print_across();
else
    print_in_columns();
```

This modular approach keeps the code clean, makes future extensions easier, and prevents mode conflicts.

## Feature-6: Colorized Output (v1.5.0)

### Q1. How do ANSI escape codes work to produce color in a standard Linux terminal? Show the specific code sequence for printing text in green.

ANSI escape codes are special sequences of characters that start with `\033[` and tell the terminal to change text appearance such as color or style.  
To print text in green, we can use the escape sequence `\033[0;32m` before the text and `\033[0m` after it to reset the color.

**Example:**
```c
printf("\033[0;32mThis text is green!\033[0m\n");

Explanation:

\033[ → Begins the escape sequence

0;32m → Sets normal intensity (0) and green foreground color (32)

\033[0m → Resets to default terminal color
```
### Q2. To color an executable file, you need to check its permission bits. Explain which bits in the st_mode field you need to check to determine if a file is executable by the owner, group, or others.

The executability of a file is determined using the execute permission bits in the st_mode field of the stat structure.
| Permission     | Macro     | Description                           |
| -------------- | --------- | ------------------------------------- |
| Owner execute  | `S_IXUSR` | File is executable by the owner       |
| Group execute  | `S_IXGRP` | File is executable by group members   |
| Others execute | `S_IXOTH` | File is executable by all other users |

To check if a file is executable by anyone:
```
if (st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) {
    // File is executable
}
```

Here’s the **`REPORT.md`** with **only the required answers** for **Feature-7: Recursive Listing (-R Option)** — no extra text:

---
## Feature-7: Recursive Listing (-R Option)

### Q1. In a recursive function, what is a "base case"? In the context of your recursive ls, what is the base case that stops the recursion from continuing forever?

A base case is the condition in a recursive function that stops further recursive calls.  
In the recursive `ls`, the base case occurs when there are **no more subdirectories to traverse**, or when the current entry is `"."` or `".."`.  
These checks prevent the function from calling itself infinitely on the same directories.

---

### Q2. Explain why it is essential to construct a full path (e.g., "parent_dir/subdir") before making a recursive call. What would happen if you simply called do_ls("subdir") from within the do_ls("parent_dir") function call?

It is essential to construct the **full path** because `lstat()` and `opendir()` require the correct absolute or relative location of the subdirectory.  
If you only call `do_ls("subdir")`, the program will attempt to open that directory relative to the current working directory instead of its actual parent path, causing **incorrect behavior or “directory not found” errors** during recursion.
```

