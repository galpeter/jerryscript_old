/* Copyright 2014-2015 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * Jerry libc platform-specific functions linux implementation
 */

#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syscall.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef __TARGET_HOST_x64
# include "asm_x64.h"
#elif defined (__TARGET_HOST_x86)
# include "asm_x86.h"
#elif defined (__TARGET_HOST_ARMv7)
# include "asm_arm.h"
#else /* !__TARGET_HOST_x64 && !__TARGET_HOST_x86 && !__TARGET_HOST_ARMv7 */
# error "!__TARGET_HOST_x64 && !__TARGET_HOST_x86 && !__TARGET_HOST_ARMv7 "
#endif /* !__TARGET_HOST_x64 && !__TARGET_HOST_x86 && !__TARGET_HOST_ARMv7 */
#include "jerry-libc-defs.h"

LIBC_UNREACHABLE_STUB_FOR (int raise (int sig_no __attr_unused___))

/**
 * Exit program with ERR_SYSCALL if syscall_ret_val is negative
 */
#define LIBC_EXIT_ON_ERROR(syscall_ret_val) \
  if ((syscall_ret_val) < 0) \
{ \
  libc_fatal ("Syscall", __FILE__, __func__, __LINE__); \
}

static long int syscall_0 (long int syscall_no);
static long int syscall_1 (long int syscall_no, long int arg1);
static long int syscall_2 (long int syscall_no, long int arg1, long int arg2);
static long int syscall_3 (long int syscall_no, long int arg1, long int arg2, long int arg3);

extern long int syscall_0_asm (long int syscall_no);
extern long int syscall_1_asm (long int syscall_no, long int arg1);
extern long int syscall_2_asm (long int syscall_no, long int arg1, long int arg2);
extern long int syscall_3_asm (long int syscall_no, long int arg1, long int arg2, long int arg3);

/**
 * System call with no argument.
 *
 * @return syscall's return value
 */
static __attr_noinline___ long int
syscall_0 (long int syscall_no) /**< syscall number */
{
  long int ret = syscall_0_asm (syscall_no);

  LIBC_EXIT_ON_ERROR (ret);

  return ret;
} /* syscall_0 */

/**
 * System call with one argument.
 *
 * @return syscall's return value
 */
static __attr_noinline___ long int
syscall_1 (long int syscall_no, /**< syscall number */
           long int arg1) /**< argument */
{
  long int ret = syscall_1_asm (syscall_no, arg1);

  LIBC_EXIT_ON_ERROR (ret);

  return ret;
} /* syscall_1 */

/**
 * System call with two arguments.
 *
 * @return syscall's return value
 */
static __attr_noinline___ long int
syscall_2 (long int syscall_no, /**< syscall number */
           long int arg1, /**< first argument */
           long int arg2) /**< second argument */
{
  long int ret = syscall_2_asm (syscall_no, arg1, arg2);

  LIBC_EXIT_ON_ERROR (ret);

  return ret;
} /* syscall_2 */

/**
 * System call with three arguments.
 *
 * @return syscall's return value
 */
static __attr_noinline___ long int
syscall_3 (long int syscall_no, /**< syscall number */
           long int arg1, /**< first argument */
           long int arg2, /**< second argument */
           long int arg3) /**< third argument */
{
  long int ret = syscall_3_asm (syscall_no, arg1, arg2, arg3);

  LIBC_EXIT_ON_ERROR (ret);

  return ret;
} /* syscall_3 */

/** Output of character. Writes the character c, cast to an unsigned char, to stdout.  */
int
putchar (int c)
{
  fwrite (&c, 1, sizeof (char), stdout);

  return c;
} /* putchar */

/**
 * Output specified string
 */
int
puts (const char *s) /**< string to print */
{
  while (*s)
  {
    putchar (*s);

    s++;
  }

  return 0;
} /* puts */

/**
 * Exit - cause normal process termination with specified status code
 */
void __attr_noreturn___ __attr_used___
exit (int status) /**< status code */
{
  syscall_1 (__NR_close, (long int)stdin);
  syscall_1 (__NR_close, (long int)stdout);
  syscall_1 (__NR_close, (long int)stderr);

  syscall_1 (__NR_exit_group, status);

  while (true)
  {
    /* unreachable */
  }
} /* exit */

/**
 * Abort current process, producing an abnormal program termination.
 * The function raises the SIGABRT signal.
 */
void __attr_noreturn___ __attr_used___
abort (void)
{
  syscall_1 (__NR_close, (long int)stdin);
  syscall_1 (__NR_close, (long int)stdout);
  syscall_1 (__NR_close, (long int)stderr);

  syscall_2 (__NR_kill, syscall_0 (__NR_getpid), SIGABRT);

  while (true)
  {
    /* unreachable */
  }
} /* abort */

/**
 * fopen
 *
 * @return FILE pointer - upon successful completion,
 *         NULL - otherwise
 */
FILE*
fopen (const char *path, /**< file path */
       const char *mode) /**< file open mode */
{
  bool may_read = false;
  bool may_write = false;
  bool truncate = false;
  bool create_if_not_exist = false;
  bool position_at_end = false;

  LIBC_ASSERT (path != NULL && mode != NULL);
  LIBC_ASSERT (mode[1] == '+' || mode[1] == '\0');

  switch (mode[0])
  {
    case 'r':
    {
      may_read = true;
      may_write = (mode[1] == '+');
      break;
    }
    case 'w':
    {
      may_write = true;
      truncate = true;
      create_if_not_exist = true;
      may_read = (mode[1] == '+');
      break;
    }
    case 'a':
    {
      may_write = true;
      position_at_end = true;
      create_if_not_exist = true;
      if (mode[1] == '+')
      {
        /* Not supported */
        LIBC_UNREACHABLE ();
      }
      break;
    }
    default:
    {
      LIBC_UNREACHABLE ();
    }
  }

  int flags = 0;
  int access = S_IRUSR | S_IWUSR;
  if (may_read && !may_write)
  {
    flags = O_RDONLY;
  }
  else if (!may_read && may_write)
  {
    flags = O_WRONLY;
  }
  else
  {
    LIBC_ASSERT (may_read && may_write);

    flags = O_RDWR;
  }

  if (truncate)
  {
    flags |= O_TRUNC;
  }

  if (create_if_not_exist)
  {
    flags |= O_CREAT;
  }

  if (position_at_end)
  {
    flags |= O_APPEND;
  }

  long int ret = syscall_3 (__NR_open, (long int) path, flags, access);

  return (void*) (uintptr_t) (ret);
} /* fopen */

/**
 * The rewind () function sets the file position indicator
 * for the stream pointed to by STREAM to the beginning of the file.
 */
void
rewind (FILE *stream) /**< stream pointer */
{
  syscall_3 (__NR_lseek, (long int) stream, 0, SEEK_SET);
} /* rewind */

/**
 * fclose
 *
 * @return 0 - upon successful completion,
 *         non-zero value - otherwise.
 */
int
fclose (FILE *fp) /**< stream pointer */
{
  syscall_2 (__NR_close, (long int)fp, 0);

  return 0;
} /* fclose */

/**
 * fseek
 */
int
fseek (FILE * fp, /**< stream pointer */
       long offset, /**< offset */
       int whence) /**< specifies position type
                    *   to add offset to */
{
  syscall_3 (__NR_lseek, (long int)fp, offset, whence);

  return 0;
} /* fseek */

/**
 * ftell
 */
long
ftell (FILE * fp) /**< stream pointer */
{
  long int ret = syscall_3 (__NR_lseek, (long int)fp, 0, SEEK_CUR);

  return ret;
} /* ftell */

/**
 * fread
 *
 * @return number of bytes read
 */
size_t
fread (void *ptr, /**< address of buffer to read to */
       size_t size, /**< size of elements to read */
       size_t nmemb, /**< number of elements to read */
       FILE *stream) /**< stream pointer */
{
  long int ret;
  size_t bytes_read = 0;

  do
  {
    ret = syscall_3 (__NR_read,
                     (long int) stream,
                     (long int) ((uint8_t*) ptr + bytes_read),
                     (long int) (size * nmemb - bytes_read));

    bytes_read += (size_t)ret;
  }
  while (bytes_read != size * nmemb && ret != 0);

  return bytes_read;
} /* fread */

/**
 * fwrite
 *
 * @return number of bytes written
 */
size_t
fwrite (const void *ptr, /**< data to write */
        size_t size, /**< size of elements to write */
        size_t nmemb, /**< number of elements */
        FILE *stream) /**< stream pointer */
{
  size_t bytes_written = 0;

  do
  {
    long int ret = syscall_3 (__NR_write,
                              (long int) stream,
                              (long int) ((uint8_t*) ptr + bytes_written),
                              (long int) (size * nmemb - bytes_written));

    bytes_written += (size_t)ret;
  }
  while (bytes_written != size * nmemb);

  return bytes_written;
} /* fwrite */

// FIXME
#if 0
/**
 * Setup new memory limits
 */
void
jrt_set_mem_limits (size_t data_size, /**< limit for data + bss + brk heap */
                    size_t stack_size) /**< limit for stack */
{
  struct
  {
    unsigned long long rlim_cur;
    unsigned long long rlim_max;
  } data_limit = { data_size, data_size };

  struct
  {
    unsigned long long rlim_cur;
    unsigned long long rlim_max;
  } stack_limit = { stack_size, stack_size };

  long int ret;

#ifdef __TARGET_HOST_x64
  ret = syscall_2 (__NR_setrlimit, RLIMIT_DATA, (intptr_t) &data_limit);
  LIBC_ASSERT (ret == 0);

  ret = syscall_2 (__NR_setrlimit, RLIMIT_STACK, (intptr_t) &stack_limit);
  LIBC_ASSERT (ret == 0);
#elif defined (__TARGET_HOST_ARMv7)
  ret = syscall_3 (__NR_prlimit64, 0, RLIMIT_DATA, (intptr_t) &data_limit);
  LIBC_ASSERT (ret == 0);

  ret = syscall_3 (__NR_prlimit64, 0, RLIMIT_STACK, (intptr_t) &stack_limit);
  LIBC_ASSERT (ret == 0);
#elif defined (__TARGET_HOST_x86)
# error "__TARGET_HOST_x86 case is not implemented"
#else /* !__TARGET_HOST_x64 && !__TARGET_HOST_ARMv7 && !__TARGET_HOST_x86 */
# error "!__TARGET_HOST_x64 && !__TARGET_HOST_ARMv7 && !__TARGET_HOST_x86"
#endif /* !__TARGET_HOST_x64 && !__TARGET_HOST_ARMv7 && !__TARGET_HOST_x86 */
} /* jrt_set_mem_limits */
#endif // FIXME
