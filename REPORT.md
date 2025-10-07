# REPORT for Feature 2 (v1.1.0)

## Q1: What is the crucial difference between stat() and lstat()? When prefer lstat()?
- `stat()` follows symbolic links: calling `stat("link")` returns metadata of the target file that the link points to.
- `lstat()` does not follow links: calling `lstat("link")` returns metadata about the link entry itself (so the file type will be shown as a symbolic link).
- For `ls -l`, `lstat()` is more appropriate because `ls` needs to show that an entry is a symlink and display the link target (e.g., `link -> target`), rather than showing the attributes of the link's target.

## Q2: How to extract file type and permission bits from st_mode using bitwise ops and macros?
- The `st_mode` field contains both file type bits and permission bits. Use bitwise AND (`&`) together with predefined macros to examine parts of it.
  - To test file type:
    - Use `S_ISDIR(st_mode)` to test for directory.
    - Use `S_ISLNK(st_mode)` to test for symbolic link.
    - Alternatively you can mask type bits: `(st_mode & S_IFMT) == S_IFDIR`.
  - To test permission bits:
    - `st_mode & S_IRUSR` → owner read bit is set.
    - `st_mode & S_IWUSR` → owner write bit.
    - `st_mode & S_IXUSR` → owner execute bit.
    - Similarly `S_IRGRP`, `S_IWGRP`, `S_IXGRP`, `S_IROTH`, `S_IWOTH`, `S_IXOTH`.
- Example:
  ```c
  if (S_ISDIR(sb.st_mode)) printf("directory\n");
  if (sb.st_mode & S_IRUSR) printf("owner can read\n");
