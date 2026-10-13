#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "user/user.h"

enum filter { ALL, FILES, DIRS };

static int
match(const char *pattern, const char *name)
{
  if (*pattern == '\0')
    return *name == '\0';
  if (*pattern == '*')
    return match(pattern + 1, name) ||
           (*name != '\0' && match(pattern, name + 1));
  return *name != '\0' && *pattern == *name &&
         match(pattern + 1, name + 1);
}

static void
fail(const char *message, const char *path)
{
  fprintf(2, "find: %s %s\n", message, path);
  exit(1);
}

static void
walk(char *path, const char *pattern, enum filter filter)
{
  char buf[512], name[DIRSIZ + 1], *end;
  struct dirent de;
  struct stat st;
  int fd;

  if ((fd = open(path, O_RDONLY)) < 0)
    fail("cannot open", path);
  if (fstat(fd, &st) < 0) {
    close(fd);
    fail("cannot stat", path);
  }
  if (st.type != T_DIR) {
    close(fd);
    fail("cannot open", path);
  }
  if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
    close(fd);
    fail("path too long", path);
  }

  strcpy(buf, path);
  end = buf + strlen(buf);
  if (end == buf || end[-1] != '/')
    *end++ = '/';

  while (read(fd, &de, sizeof(de)) == sizeof(de)) {
    if (de.inum == 0)
      continue;
    memmove(name, de.name, DIRSIZ);
    name[DIRSIZ] = '\0';
    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
      continue;

    strcpy(end, name);
    if (stat(buf, &st) < 0) {
      close(fd);
      fail("cannot stat", buf);
    }

    if (match(pattern, name) &&
        (filter == ALL ||
         (filter == FILES && (st.type == T_FILE || st.type == T_DEVICE)) ||
         (filter == DIRS && st.type == T_DIR)))
      printf("%s\n", buf);

    if (st.type == T_DIR)
      walk(buf, pattern, filter);
  }
  close(fd);
}

static void
usage(void)
{
  fprintf(2, "usage: find <dir> <pattern> [-type f|d]\n");
  exit(1);
}

int
main(int argc, char *argv[])
{
  enum filter filter = ALL;

  if (argc != 3 && argc != 5)
    usage();
  if (argc == 5) {
    if (strcmp(argv[3], "-type") != 0)
      usage();
    if (strcmp(argv[4], "f") == 0)
      filter = FILES;
    else if (strcmp(argv[4], "d") == 0)
      filter = DIRS;
    else
      usage();
  }

  walk(argv[1], argv[2], filter);
  exit(0);
}
