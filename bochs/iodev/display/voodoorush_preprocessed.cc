# 0 "voodoorush.cc"
# 1 "/workspace/bochs/iodev/display//"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 0 "<command-line>" 2
# 1 "voodoorush.cc"
# 28 "voodoorush.cc"
# 1 "../iodev.h" 1
# 28 "../iodev.h"
# 1 "../../bochs.h" 1
# 31 "../../bochs.h"
# 1 "../../config.h" 1
# 435 "../../config.h"
  typedef unsigned char Bit8u;
  typedef signed char Bit8s;





  typedef unsigned short Bit16u;
  typedef signed short Bit16s;



  typedef unsigned int Bit32u;
  typedef signed int Bit32s;
# 457 "../../config.h"
  typedef unsigned long Bit64u;
  typedef signed long Bit64s;
# 477 "../../config.h"
typedef Bit32u bx_address;



typedef bx_address bx_lin_address;


typedef Bit64u bx_phy_address;
# 529 "../../config.h"
  typedef Bit64u bx_ptr_equiv_t;
# 32 "../../bochs.h" 2


extern "C" {
# 45 "../../bochs.h"
# 1 "/usr/lib/gcc/x86_64-linux-gnu/14/include/stdarg.h" 1 3 4
# 40 "/usr/lib/gcc/x86_64-linux-gnu/14/include/stdarg.h" 3 4

# 40 "/usr/lib/gcc/x86_64-linux-gnu/14/include/stdarg.h" 3 4
typedef __builtin_va_list __gnuc_va_list;
# 103 "/usr/lib/gcc/x86_64-linux-gnu/14/include/stdarg.h" 3 4
typedef __gnuc_va_list va_list;
# 46 "../../bochs.h" 2
# 1 "/usr/include/stdio.h" 1 3 4
# 28 "/usr/include/stdio.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/libc-header-start.h" 1 3 4
# 33 "/usr/include/x86_64-linux-gnu/bits/libc-header-start.h" 3 4
# 1 "/usr/include/features.h" 1 3 4
# 415 "/usr/include/features.h" 3 4
# 1 "/usr/include/features-time64.h" 1 3 4
# 20 "/usr/include/features-time64.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/wordsize.h" 1 3 4
# 21 "/usr/include/features-time64.h" 2 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/timesize.h" 1 3 4
# 19 "/usr/include/x86_64-linux-gnu/bits/timesize.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/wordsize.h" 1 3 4
# 20 "/usr/include/x86_64-linux-gnu/bits/timesize.h" 2 3 4
# 22 "/usr/include/features-time64.h" 2 3 4
# 416 "/usr/include/features.h" 2 3 4
# 523 "/usr/include/features.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/sys/cdefs.h" 1 3 4
# 730 "/usr/include/x86_64-linux-gnu/sys/cdefs.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/wordsize.h" 1 3 4
# 731 "/usr/include/x86_64-linux-gnu/sys/cdefs.h" 2 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/long-double.h" 1 3 4
# 732 "/usr/include/x86_64-linux-gnu/sys/cdefs.h" 2 3 4
# 524 "/usr/include/features.h" 2 3 4
# 547 "/usr/include/features.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/gnu/stubs.h" 1 3 4
# 10 "/usr/include/x86_64-linux-gnu/gnu/stubs.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/gnu/stubs-64.h" 1 3 4
# 11 "/usr/include/x86_64-linux-gnu/gnu/stubs.h" 2 3 4
# 548 "/usr/include/features.h" 2 3 4
# 34 "/usr/include/x86_64-linux-gnu/bits/libc-header-start.h" 2 3 4
# 29 "/usr/include/stdio.h" 2 3 4

extern "C" {



# 1 "/usr/lib/gcc/x86_64-linux-gnu/14/include/stddef.h" 1 3 4
# 214 "/usr/lib/gcc/x86_64-linux-gnu/14/include/stddef.h" 3 4
typedef long unsigned int size_t;
# 35 "/usr/include/stdio.h" 2 3 4




# 1 "/usr/include/x86_64-linux-gnu/bits/types.h" 1 3 4
# 27 "/usr/include/x86_64-linux-gnu/bits/types.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/wordsize.h" 1 3 4
# 28 "/usr/include/x86_64-linux-gnu/bits/types.h" 2 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/timesize.h" 1 3 4
# 19 "/usr/include/x86_64-linux-gnu/bits/timesize.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/wordsize.h" 1 3 4
# 20 "/usr/include/x86_64-linux-gnu/bits/timesize.h" 2 3 4
# 29 "/usr/include/x86_64-linux-gnu/bits/types.h" 2 3 4


typedef unsigned char __u_char;
typedef unsigned short int __u_short;
typedef unsigned int __u_int;
typedef unsigned long int __u_long;


typedef signed char __int8_t;
typedef unsigned char __uint8_t;
typedef signed short int __int16_t;
typedef unsigned short int __uint16_t;
typedef signed int __int32_t;
typedef unsigned int __uint32_t;

typedef signed long int __int64_t;
typedef unsigned long int __uint64_t;






typedef __int8_t __int_least8_t;
typedef __uint8_t __uint_least8_t;
typedef __int16_t __int_least16_t;
typedef __uint16_t __uint_least16_t;
typedef __int32_t __int_least32_t;
typedef __uint32_t __uint_least32_t;
typedef __int64_t __int_least64_t;
typedef __uint64_t __uint_least64_t;



typedef long int __quad_t;
typedef unsigned long int __u_quad_t;







typedef long int __intmax_t;
typedef unsigned long int __uintmax_t;
# 141 "/usr/include/x86_64-linux-gnu/bits/types.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/typesizes.h" 1 3 4
# 142 "/usr/include/x86_64-linux-gnu/bits/types.h" 2 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/time64.h" 1 3 4
# 143 "/usr/include/x86_64-linux-gnu/bits/types.h" 2 3 4


typedef unsigned long int __dev_t;
typedef unsigned int __uid_t;
typedef unsigned int __gid_t;
typedef unsigned long int __ino_t;
typedef unsigned long int __ino64_t;
typedef unsigned int __mode_t;
typedef unsigned long int __nlink_t;
typedef long int __off_t;
typedef long int __off64_t;
typedef int __pid_t;
typedef struct { int __val[2]; } __fsid_t;
typedef long int __clock_t;
typedef unsigned long int __rlim_t;
typedef unsigned long int __rlim64_t;
typedef unsigned int __id_t;
typedef long int __time_t;
typedef unsigned int __useconds_t;
typedef long int __suseconds_t;
typedef long int __suseconds64_t;

typedef int __daddr_t;
typedef int __key_t;


typedef int __clockid_t;


typedef void * __timer_t;


typedef long int __blksize_t;




typedef long int __blkcnt_t;
typedef long int __blkcnt64_t;


typedef unsigned long int __fsblkcnt_t;
typedef unsigned long int __fsblkcnt64_t;


typedef unsigned long int __fsfilcnt_t;
typedef unsigned long int __fsfilcnt64_t;


typedef long int __fsword_t;

typedef long int __ssize_t;


typedef long int __syscall_slong_t;

typedef unsigned long int __syscall_ulong_t;



typedef __off64_t __loff_t;
typedef char *__caddr_t;


typedef long int __intptr_t;


typedef unsigned int __socklen_t;




typedef int __sig_atomic_t;
# 40 "/usr/include/stdio.h" 2 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/types/__fpos_t.h" 1 3 4




# 1 "/usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h" 1 3 4
# 13 "/usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h" 3 4
typedef struct
{
  int __count;
  union
  {
    unsigned int __wch;
    char __wchb[4];
  } __value;
} __mbstate_t;
# 6 "/usr/include/x86_64-linux-gnu/bits/types/__fpos_t.h" 2 3 4




typedef struct _G_fpos_t
{
  __off_t __pos;
  __mbstate_t __state;
} __fpos_t;
# 41 "/usr/include/stdio.h" 2 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/types/__fpos64_t.h" 1 3 4
# 10 "/usr/include/x86_64-linux-gnu/bits/types/__fpos64_t.h" 3 4
typedef struct _G_fpos64_t
{
  __off64_t __pos;
  __mbstate_t __state;
} __fpos64_t;
# 42 "/usr/include/stdio.h" 2 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/types/__FILE.h" 1 3 4



struct _IO_FILE;
typedef struct _IO_FILE __FILE;
# 43 "/usr/include/stdio.h" 2 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/types/FILE.h" 1 3 4



struct _IO_FILE;


typedef struct _IO_FILE FILE;
# 44 "/usr/include/stdio.h" 2 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/types/struct_FILE.h" 1 3 4
# 36 "/usr/include/x86_64-linux-gnu/bits/types/struct_FILE.h" 3 4
struct _IO_FILE;
struct _IO_marker;
struct _IO_codecvt;
struct _IO_wide_data;




typedef void _IO_lock_t;





struct _IO_FILE
{
  int _flags;


  char *_IO_read_ptr;
  char *_IO_read_end;
  char *_IO_read_base;
  char *_IO_write_base;
  char *_IO_write_ptr;
  char *_IO_write_end;
  char *_IO_buf_base;
  char *_IO_buf_end;


  char *_IO_save_base;
  char *_IO_backup_base;
  char *_IO_save_end;

  struct _IO_marker *_markers;

  struct _IO_FILE *_chain;

  int _fileno;
  int _flags2:24;

  char _short_backupbuf[1];
  __off_t _old_offset;


  unsigned short _cur_column;
  signed char _vtable_offset;
  char _shortbuf[1];

  _IO_lock_t *_lock;







  __off64_t _offset;

  struct _IO_codecvt *_codecvt;
  struct _IO_wide_data *_wide_data;
  struct _IO_FILE *_freeres_list;
  void *_freeres_buf;
  struct _IO_FILE **_prevchain;
  int _mode;

  char _unused2[15 * sizeof (int) - 5 * sizeof (void *)];
};
# 45 "/usr/include/stdio.h" 2 3 4


# 1 "/usr/include/x86_64-linux-gnu/bits/types/cookie_io_functions_t.h" 1 3 4
# 27 "/usr/include/x86_64-linux-gnu/bits/types/cookie_io_functions_t.h" 3 4
typedef __ssize_t cookie_read_function_t (void *__cookie, char *__buf,
                                          size_t __nbytes);







typedef __ssize_t cookie_write_function_t (void *__cookie, const char *__buf,
                                           size_t __nbytes);







typedef int cookie_seek_function_t (void *__cookie, __off64_t *__pos, int __w);


typedef int cookie_close_function_t (void *__cookie);






typedef struct _IO_cookie_io_functions_t
{
  cookie_read_function_t *read;
  cookie_write_function_t *write;
  cookie_seek_function_t *seek;
  cookie_close_function_t *close;
} cookie_io_functions_t;
# 48 "/usr/include/stdio.h" 2 3 4
# 66 "/usr/include/stdio.h" 3 4
typedef __off64_t off_t;




typedef __off64_t off64_t;






typedef __ssize_t ssize_t;
# 87 "/usr/include/stdio.h" 3 4
typedef __fpos64_t fpos_t;


typedef __fpos64_t fpos64_t;
# 129 "/usr/include/stdio.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/stdio_lim.h" 1 3 4
# 130 "/usr/include/stdio.h" 2 3 4
# 149 "/usr/include/stdio.h" 3 4
extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;






extern int remove (const char *__filename) noexcept (true);

extern int rename (const char *__old, const char *__new) noexcept (true);



extern int renameat (int __oldfd, const char *__old, int __newfd,
       const char *__new) noexcept (true);
# 176 "/usr/include/stdio.h" 3 4
extern int renameat2 (int __oldfd, const char *__old, int __newfd,
        const char *__new, unsigned int __flags) noexcept (true);






extern int fclose (FILE *__stream) __attribute__ ((__nonnull__ (1)));
# 198 "/usr/include/stdio.h" 3 4
extern FILE *tmpfile (void) __asm__ ("" "tmpfile64")
  __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (fclose, 1))) __attribute__ ((__warn_unused_result__));






extern FILE *tmpfile64 (void)
   __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (fclose, 1))) __attribute__ ((__warn_unused_result__));



extern char *tmpnam (char[20]) noexcept (true) __attribute__ ((__warn_unused_result__));




extern char *tmpnam_r (char __s[20]) noexcept (true) __attribute__ ((__warn_unused_result__));
# 228 "/usr/include/stdio.h" 3 4
extern char *tempnam (const char *__dir, const char *__pfx)
   noexcept (true) __attribute__ ((__malloc__)) __attribute__ ((__warn_unused_result__)) __attribute__ ((__malloc__ (__builtin_free, 1)));






extern int fflush (FILE *__stream);
# 245 "/usr/include/stdio.h" 3 4
extern int fflush_unlocked (FILE *__stream);
# 255 "/usr/include/stdio.h" 3 4
extern int fcloseall (void);
# 276 "/usr/include/stdio.h" 3 4
extern FILE *fopen (const char *__restrict __filename, const char *__restrict __modes) __asm__ ("" "fopen64")

  __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (fclose, 1))) __attribute__ ((__warn_unused_result__));
extern FILE *freopen (const char *__restrict __filename, const char *__restrict __modes, FILE *__restrict __stream) __asm__ ("" "freopen64")


  __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (3)));






extern FILE *fopen64 (const char *__restrict __filename,
        const char *__restrict __modes)
  __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (fclose, 1))) __attribute__ ((__warn_unused_result__));
extern FILE *freopen64 (const char *__restrict __filename,
   const char *__restrict __modes,
   FILE *__restrict __stream) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (3)));




extern FILE *fdopen (int __fd, const char *__modes) noexcept (true)
  __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (fclose, 1))) __attribute__ ((__warn_unused_result__));





extern FILE *fopencookie (void *__restrict __magic_cookie,
     const char *__restrict __modes,
     cookie_io_functions_t __io_funcs) noexcept (true)
  __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (fclose, 1))) __attribute__ ((__warn_unused_result__));




extern FILE *fmemopen (void *__s, size_t __len, const char *__modes)
  noexcept (true) __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (fclose, 1))) __attribute__ ((__warn_unused_result__));




extern FILE *open_memstream (char **__bufloc, size_t *__sizeloc) noexcept (true)
  __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (fclose, 1))) __attribute__ ((__warn_unused_result__));
# 334 "/usr/include/stdio.h" 3 4
extern void setbuf (FILE *__restrict __stream, char *__restrict __buf) noexcept (true)
  __attribute__ ((__nonnull__ (1)));



extern int setvbuf (FILE *__restrict __stream, char *__restrict __buf,
      int __modes, size_t __n) noexcept (true) __attribute__ ((__nonnull__ (1)));




extern void setbuffer (FILE *__restrict __stream, char *__restrict __buf,
         size_t __size) noexcept (true) __attribute__ ((__nonnull__ (1)));


extern void setlinebuf (FILE *__stream) noexcept (true) __attribute__ ((__nonnull__ (1)));







extern int fprintf (FILE *__restrict __stream,
      const char *__restrict __format, ...) __attribute__ ((__nonnull__ (1)));




extern int printf (const char *__restrict __format, ...);

extern int sprintf (char *__restrict __s,
      const char *__restrict __format, ...) noexcept (true);





extern int vfprintf (FILE *__restrict __s, const char *__restrict __format,
       __gnuc_va_list __arg) __attribute__ ((__nonnull__ (1)));




extern int vprintf (const char *__restrict __format, __gnuc_va_list __arg);

extern int vsprintf (char *__restrict __s, const char *__restrict __format,
       __gnuc_va_list __arg) noexcept (true);



extern int snprintf (char *__restrict __s, size_t __maxlen,
       const char *__restrict __format, ...)
     noexcept (true) __attribute__ ((__format__ (__printf__, 3, 4)));

extern int vsnprintf (char *__restrict __s, size_t __maxlen,
        const char *__restrict __format, __gnuc_va_list __arg)
     noexcept (true) __attribute__ ((__format__ (__printf__, 3, 0)));





extern int vasprintf (char **__restrict __ptr, const char *__restrict __f,
        __gnuc_va_list __arg)
     noexcept (true) __attribute__ ((__format__ (__printf__, 2, 0))) __attribute__ ((__warn_unused_result__));
extern int __asprintf (char **__restrict __ptr,
         const char *__restrict __fmt, ...)
     noexcept (true) __attribute__ ((__format__ (__printf__, 2, 3))) __attribute__ ((__warn_unused_result__));
extern int asprintf (char **__restrict __ptr,
       const char *__restrict __fmt, ...)
     noexcept (true) __attribute__ ((__format__ (__printf__, 2, 3))) __attribute__ ((__warn_unused_result__));




extern int vdprintf (int __fd, const char *__restrict __fmt,
       __gnuc_va_list __arg)
     __attribute__ ((__format__ (__printf__, 2, 0)));
extern int dprintf (int __fd, const char *__restrict __fmt, ...)
     __attribute__ ((__format__ (__printf__, 2, 3)));







extern int fscanf (FILE *__restrict __stream,
     const char *__restrict __format, ...) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (1)));




extern int scanf (const char *__restrict __format, ...) __attribute__ ((__warn_unused_result__));

extern int sscanf (const char *__restrict __s,
     const char *__restrict __format, ...) noexcept (true);





# 1 "/usr/include/x86_64-linux-gnu/bits/floatn.h" 1 3 4
# 131 "/usr/include/x86_64-linux-gnu/bits/floatn.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/floatn-common.h" 1 3 4
# 24 "/usr/include/x86_64-linux-gnu/bits/floatn-common.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/long-double.h" 1 3 4
# 25 "/usr/include/x86_64-linux-gnu/bits/floatn-common.h" 2 3 4
# 132 "/usr/include/x86_64-linux-gnu/bits/floatn.h" 2 3 4
# 438 "/usr/include/stdio.h" 2 3 4




extern int fscanf (FILE *__restrict __stream, const char *__restrict __format, ...) __asm__ ("" "__isoc23_fscanf")

                          __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (1)));
extern int scanf (const char *__restrict __format, ...) __asm__ ("" "__isoc23_scanf")
                         __attribute__ ((__warn_unused_result__));
extern int sscanf (const char *__restrict __s, const char *__restrict __format, ...) noexcept (true) __asm__ ("" "__isoc23_sscanf")

                      ;
# 490 "/usr/include/stdio.h" 3 4
extern int vfscanf (FILE *__restrict __s, const char *__restrict __format,
      __gnuc_va_list __arg)
     __attribute__ ((__format__ (__scanf__, 2, 0))) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (1)));





extern int vscanf (const char *__restrict __format, __gnuc_va_list __arg)
     __attribute__ ((__format__ (__scanf__, 1, 0))) __attribute__ ((__warn_unused_result__));


extern int vsscanf (const char *__restrict __s,
      const char *__restrict __format, __gnuc_va_list __arg)
     noexcept (true) __attribute__ ((__format__ (__scanf__, 2, 0)));






extern int vfscanf (FILE *__restrict __s, const char *__restrict __format, __gnuc_va_list __arg) __asm__ ("" "__isoc23_vfscanf")



     __attribute__ ((__format__ (__scanf__, 2, 0))) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (1)));
extern int vscanf (const char *__restrict __format, __gnuc_va_list __arg) __asm__ ("" "__isoc23_vscanf")

     __attribute__ ((__format__ (__scanf__, 1, 0))) __attribute__ ((__warn_unused_result__));
extern int vsscanf (const char *__restrict __s, const char *__restrict __format, __gnuc_va_list __arg) noexcept (true) __asm__ ("" "__isoc23_vsscanf")



     __attribute__ ((__format__ (__scanf__, 2, 0)));
# 575 "/usr/include/stdio.h" 3 4
extern int fgetc (FILE *__stream) __attribute__ ((__nonnull__ (1)));
extern int getc (FILE *__stream) __attribute__ ((__nonnull__ (1)));





extern int getchar (void);






extern int getc_unlocked (FILE *__stream) __attribute__ ((__nonnull__ (1)));
extern int getchar_unlocked (void);
# 600 "/usr/include/stdio.h" 3 4
extern int fgetc_unlocked (FILE *__stream) __attribute__ ((__nonnull__ (1)));
# 611 "/usr/include/stdio.h" 3 4
extern int fputc (int __c, FILE *__stream) __attribute__ ((__nonnull__ (2)));
extern int putc (int __c, FILE *__stream) __attribute__ ((__nonnull__ (2)));





extern int putchar (int __c);
# 627 "/usr/include/stdio.h" 3 4
extern int fputc_unlocked (int __c, FILE *__stream) __attribute__ ((__nonnull__ (2)));







extern int putc_unlocked (int __c, FILE *__stream) __attribute__ ((__nonnull__ (2)));
extern int putchar_unlocked (int __c);






extern int getw (FILE *__stream) __attribute__ ((__nonnull__ (1)));


extern int putw (int __w, FILE *__stream) __attribute__ ((__nonnull__ (2)));







extern char *fgets (char *__restrict __s, int __n, FILE *__restrict __stream)
     __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (3)));
# 677 "/usr/include/stdio.h" 3 4
extern char *fgets_unlocked (char *__restrict __s, int __n,
        FILE *__restrict __stream) __attribute__ ((__warn_unused_result__))
    __attribute__ ((__nonnull__ (3)));
# 689 "/usr/include/stdio.h" 3 4
extern __ssize_t __getdelim (char **__restrict __lineptr,
                             size_t *__restrict __n, int __delimiter,
                             FILE *__restrict __stream) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (4)));
extern __ssize_t getdelim (char **__restrict __lineptr,
                           size_t *__restrict __n, int __delimiter,
                           FILE *__restrict __stream) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (4)));


extern __ssize_t getline (char **__restrict __lineptr,
                          size_t *__restrict __n,
                          FILE *__restrict __stream) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (3)));







extern int fputs (const char *__restrict __s, FILE *__restrict __stream)
  __attribute__ ((__nonnull__ (2)));





extern int puts (const char *__s);






extern int ungetc (int __c, FILE *__stream) __attribute__ ((__nonnull__ (2)));






extern size_t fread (void *__restrict __ptr, size_t __size,
       size_t __n, FILE *__restrict __stream) __attribute__ ((__warn_unused_result__))
  __attribute__ ((__nonnull__ (4)));




extern size_t fwrite (const void *__restrict __ptr, size_t __size,
        size_t __n, FILE *__restrict __s) __attribute__ ((__nonnull__ (4)));
# 745 "/usr/include/stdio.h" 3 4
extern int fputs_unlocked (const char *__restrict __s,
      FILE *__restrict __stream) __attribute__ ((__nonnull__ (2)));
# 756 "/usr/include/stdio.h" 3 4
extern size_t fread_unlocked (void *__restrict __ptr, size_t __size,
         size_t __n, FILE *__restrict __stream) __attribute__ ((__warn_unused_result__))
  __attribute__ ((__nonnull__ (4)));
extern size_t fwrite_unlocked (const void *__restrict __ptr, size_t __size,
          size_t __n, FILE *__restrict __stream)
  __attribute__ ((__nonnull__ (4)));







extern int fseek (FILE *__stream, long int __off, int __whence)
  __attribute__ ((__nonnull__ (1)));




extern long int ftell (FILE *__stream) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (1)));




extern void rewind (FILE *__stream) __attribute__ ((__nonnull__ (1)));
# 802 "/usr/include/stdio.h" 3 4
extern int fseeko (FILE *__stream, __off64_t __off, int __whence) __asm__ ("" "fseeko64")

                   __attribute__ ((__nonnull__ (1)));
extern __off64_t ftello (FILE *__stream) __asm__ ("" "ftello64")
  __attribute__ ((__nonnull__ (1)));
# 828 "/usr/include/stdio.h" 3 4
extern int fgetpos (FILE *__restrict __stream, fpos_t *__restrict __pos) __asm__ ("" "fgetpos64")

  __attribute__ ((__nonnull__ (1)));
extern int fsetpos (FILE *__stream, const fpos_t *__pos) __asm__ ("" "fsetpos64")

  __attribute__ ((__nonnull__ (1)));







extern int fseeko64 (FILE *__stream, __off64_t __off, int __whence)
  __attribute__ ((__nonnull__ (1)));
extern __off64_t ftello64 (FILE *__stream) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (1)));
extern int fgetpos64 (FILE *__restrict __stream, fpos64_t *__restrict __pos)
  __attribute__ ((__nonnull__ (1)));
extern int fsetpos64 (FILE *__stream, const fpos64_t *__pos) __attribute__ ((__nonnull__ (1)));



extern void clearerr (FILE *__stream) noexcept (true) __attribute__ ((__nonnull__ (1)));

extern int feof (FILE *__stream) noexcept (true) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (1)));

extern int ferror (FILE *__stream) noexcept (true) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (1)));



extern void clearerr_unlocked (FILE *__stream) noexcept (true) __attribute__ ((__nonnull__ (1)));
extern int feof_unlocked (FILE *__stream) noexcept (true) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (1)));
extern int ferror_unlocked (FILE *__stream) noexcept (true) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (1)));







extern void perror (const char *__s) __attribute__ ((__cold__));




extern int fileno (FILE *__stream) noexcept (true) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (1)));




extern int fileno_unlocked (FILE *__stream) noexcept (true) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (1)));
# 887 "/usr/include/stdio.h" 3 4
extern int pclose (FILE *__stream) __attribute__ ((__nonnull__ (1)));





extern FILE *popen (const char *__command, const char *__modes)
  __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (pclose, 1))) __attribute__ ((__warn_unused_result__));






extern char *ctermid (char *__s) noexcept (true)
  __attribute__ ((__access__ (__write_only__, 1)));





extern char *cuserid (char *__s)
  __attribute__ ((__access__ (__write_only__, 1)));




struct obstack;


extern int obstack_printf (struct obstack *__restrict __obstack,
      const char *__restrict __format, ...)
     noexcept (true) __attribute__ ((__format__ (__printf__, 2, 3)));
extern int obstack_vprintf (struct obstack *__restrict __obstack,
       const char *__restrict __format,
       __gnuc_va_list __args)
     noexcept (true) __attribute__ ((__format__ (__printf__, 2, 0)));







extern void flockfile (FILE *__stream) noexcept (true) __attribute__ ((__nonnull__ (1)));



extern int ftrylockfile (FILE *__stream) noexcept (true) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (1)));


extern void funlockfile (FILE *__stream) noexcept (true) __attribute__ ((__nonnull__ (1)));
# 949 "/usr/include/stdio.h" 3 4
extern int __uflow (FILE *);
extern int __overflow (FILE *, int);



# 1 "/usr/include/x86_64-linux-gnu/bits/stdio2-decl.h" 1 3 4
# 26 "/usr/include/x86_64-linux-gnu/bits/stdio2-decl.h" 3 4
extern int __sprintf_chk (char *__restrict __s, int __flag, size_t __slen,
     const char *__restrict __format, ...) noexcept (true)
    __attribute__ ((__access__ (__write_only__, 1, 3)));
extern int __vsprintf_chk (char *__restrict __s, int __flag, size_t __slen,
      const char *__restrict __format,
      __gnuc_va_list __ap) noexcept (true)
    __attribute__ ((__access__ (__write_only__, 1, 3)));



extern int __snprintf_chk (char *__restrict __s, size_t __n, int __flag,
      size_t __slen, const char *__restrict __format,
      ...) noexcept (true)
    __attribute__ ((__access__ (__write_only__, 1, 2)));
extern int __vsnprintf_chk (char *__restrict __s, size_t __n, int __flag,
       size_t __slen, const char *__restrict __format,
       __gnuc_va_list __ap) noexcept (true)
    __attribute__ ((__access__ (__write_only__, 1, 2)));





extern int __fprintf_chk (FILE *__restrict __stream, int __flag,
     const char *__restrict __format, ...)
    __attribute__ ((__nonnull__ (1)));
extern int __printf_chk (int __flag, const char *__restrict __format, ...);
extern int __vfprintf_chk (FILE *__restrict __stream, int __flag,
      const char *__restrict __format,
      __gnuc_va_list __ap) __attribute__ ((__nonnull__ (1)));
extern int __vprintf_chk (int __flag, const char *__restrict __format,
     __gnuc_va_list __ap);


extern int __dprintf_chk (int __fd, int __flag, const char *__restrict __fmt,
     ...) __attribute__ ((__format__ (__printf__, 3, 4)));
extern int __vdprintf_chk (int __fd, int __flag,
      const char *__restrict __fmt, __gnuc_va_list __arg)
     __attribute__ ((__format__ (__printf__, 3, 0)));




extern int __asprintf_chk (char **__restrict __ptr, int __flag,
      const char *__restrict __fmt, ...)
     noexcept (true) __attribute__ ((__format__ (__printf__, 3, 4))) __attribute__ ((__warn_unused_result__));
extern int __vasprintf_chk (char **__restrict __ptr, int __flag,
       const char *__restrict __fmt, __gnuc_va_list __arg)
     noexcept (true) __attribute__ ((__format__ (__printf__, 3, 0))) __attribute__ ((__warn_unused_result__));
extern int __obstack_printf_chk (struct obstack *__restrict __obstack,
     int __flag, const char *__restrict __format,
     ...)
     noexcept (true) __attribute__ ((__format__ (__printf__, 3, 4)));
extern int __obstack_vprintf_chk (struct obstack *__restrict __obstack,
      int __flag,
      const char *__restrict __format,
      __gnuc_va_list __args)
     noexcept (true) __attribute__ ((__format__ (__printf__, 3, 0)));
# 96 "/usr/include/x86_64-linux-gnu/bits/stdio2-decl.h" 3 4
extern char *__fgets_alias (char *__restrict __s, int __n, FILE *__restrict __stream) __asm__ ("" "fgets")


    __attribute__ ((__warn_unused_result__)) __attribute__ ((__access__ (__write_only__, 1, 2)));
extern char *__fgets_chk_warn (char *__restrict __s, size_t __size, int __n, FILE *__restrict __stream) __asm__ ("" "__fgets_chk")


     __attribute__ ((__warn_unused_result__)) __attribute__((__warning__ ("fgets called with bigger size than length " "of destination buffer")))
                                 ;

extern char *__fgets_chk (char *__restrict __s, size_t __size, int __n,
     FILE *__restrict __stream)
    __attribute__ ((__warn_unused_result__)) __attribute__ ((__access__ (__write_only__, 1, 3))) __attribute__ ((__nonnull__ (4)));

extern size_t __fread_alias (void *__restrict __ptr, size_t __size, size_t __n, FILE *__restrict __stream) __asm__ ("" "fread")


            __attribute__ ((__warn_unused_result__));
extern size_t __fread_chk_warn (void *__restrict __ptr, size_t __ptrlen, size_t __size, size_t __n, FILE *__restrict __stream) __asm__ ("" "__fread_chk")




     __attribute__ ((__warn_unused_result__)) __attribute__((__warning__ ("fread called with bigger size * nmemb than length " "of destination buffer")))
                                 ;

extern size_t __fread_chk (void *__restrict __ptr, size_t __ptrlen,
      size_t __size, size_t __n,
      FILE *__restrict __stream) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (5)));


extern char *__fgets_unlocked_alias (char *__restrict __s, int __n, FILE *__restrict __stream) __asm__ ("" "fgets_unlocked")


    __attribute__ ((__warn_unused_result__)) __attribute__ ((__access__ (__write_only__, 1, 2)));
extern char *__fgets_unlocked_chk_warn (char *__restrict __s, size_t __size, int __n, FILE *__restrict __stream) __asm__ ("" "__fgets_unlocked_chk")


     __attribute__ ((__warn_unused_result__)) __attribute__((__warning__ ("fgets_unlocked called with bigger size than length " "of destination buffer")))
                                 ;


extern char *__fgets_unlocked_chk (char *__restrict __s, size_t __size,
       int __n, FILE *__restrict __stream)
    __attribute__ ((__warn_unused_result__)) __attribute__ ((__access__ (__write_only__, 1, 3))) __attribute__ ((__nonnull__ (4)));




extern size_t __fread_unlocked_alias (void *__restrict __ptr, size_t __size, size_t __n, FILE *__restrict __stream) __asm__ ("" "fread_unlocked")


                     __attribute__ ((__warn_unused_result__));
extern size_t __fread_unlocked_chk_warn (void *__restrict __ptr, size_t __ptrlen, size_t __size, size_t __n, FILE *__restrict __stream) __asm__ ("" "__fread_unlocked_chk")




     __attribute__ ((__warn_unused_result__)) __attribute__((__warning__ ("fread_unlocked called with bigger size * nmemb than " "length of destination buffer")))
                                        ;

extern size_t __fread_unlocked_chk (void *__restrict __ptr, size_t __ptrlen,
        size_t __size, size_t __n,
        FILE *__restrict __stream)
    __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (5)));
# 955 "/usr/include/stdio.h" 2 3 4
# 966 "/usr/include/stdio.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/stdio.h" 1 3 4
# 46 "/usr/include/x86_64-linux-gnu/bits/stdio.h" 3 4
extern __inline __attribute__ ((__gnu_inline__)) int
getchar (void)
{
  return getc (stdin);
}




extern __inline __attribute__ ((__gnu_inline__)) int
fgetc_unlocked (FILE *__fp)
{
  return (__builtin_expect (((__fp)->_IO_read_ptr >= (__fp)->_IO_read_end), 0) ? __uflow (__fp) : *(unsigned char *) (__fp)->_IO_read_ptr++);
}





extern __inline __attribute__ ((__gnu_inline__)) int
getc_unlocked (FILE *__fp)
{
  return (__builtin_expect (((__fp)->_IO_read_ptr >= (__fp)->_IO_read_end), 0) ? __uflow (__fp) : *(unsigned char *) (__fp)->_IO_read_ptr++);
}


extern __inline __attribute__ ((__gnu_inline__)) int
getchar_unlocked (void)
{
  return (__builtin_expect (((stdin)->_IO_read_ptr >= (stdin)->_IO_read_end), 0) ? __uflow (stdin) : *(unsigned char *) (stdin)->_IO_read_ptr++);
}




extern __inline __attribute__ ((__gnu_inline__)) int
putchar (int __c)
{
  return putc (__c, stdout);
}




extern __inline __attribute__ ((__gnu_inline__)) int
fputc_unlocked (int __c, FILE *__stream)
{
  return (__builtin_expect (((__stream)->_IO_write_ptr >= (__stream)->_IO_write_end), 0) ? __overflow (__stream, (unsigned char) (__c)) : (unsigned char) (*(__stream)->_IO_write_ptr++ = (__c)));
}





extern __inline __attribute__ ((__gnu_inline__)) int
putc_unlocked (int __c, FILE *__stream)
{
  return (__builtin_expect (((__stream)->_IO_write_ptr >= (__stream)->_IO_write_end), 0) ? __overflow (__stream, (unsigned char) (__c)) : (unsigned char) (*(__stream)->_IO_write_ptr++ = (__c)));
}


extern __inline __attribute__ ((__gnu_inline__)) int
putchar_unlocked (int __c)
{
  return (__builtin_expect (((stdout)->_IO_write_ptr >= (stdout)->_IO_write_end), 0) ? __overflow (stdout, (unsigned char) (__c)) : (unsigned char) (*(stdout)->_IO_write_ptr++ = (__c)));
}





extern __inline __attribute__ ((__gnu_inline__)) __ssize_t
getline (char **__lineptr, size_t *__n, FILE *__stream)
{
  return __getdelim (__lineptr, __n, '\n', __stream);
}





extern __inline __attribute__ ((__gnu_inline__)) int
__attribute__ ((__leaf__)) feof_unlocked (FILE *__stream) noexcept (true)
{
  return (((__stream)->_flags & 0x0010) != 0);
}


extern __inline __attribute__ ((__gnu_inline__)) int
__attribute__ ((__leaf__)) ferror_unlocked (FILE *__stream) noexcept (true)
{
  return (((__stream)->_flags & 0x0020) != 0);
}
# 967 "/usr/include/stdio.h" 2 3 4



# 1 "/usr/include/x86_64-linux-gnu/bits/stdio2.h" 1 3 4
# 27 "/usr/include/x86_64-linux-gnu/bits/stdio2.h" 3 4
extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) int
__attribute__ ((__leaf__)) sprintf (char *__restrict __s, const char *__restrict __fmt, ...) noexcept (true)
{
  return __builtin___sprintf_chk (__s, 3 - 1,
      __builtin_dynamic_object_size (__s, 1), __fmt,
      __builtin_va_arg_pack ());
}
# 54 "/usr/include/x86_64-linux-gnu/bits/stdio2.h" 3 4
extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) int
__attribute__ ((__leaf__)) vsprintf (char * __restrict __s, const char *__restrict __fmt, __gnuc_va_list __ap) noexcept (true)

{
  return __builtin___vsprintf_chk (__s, 3 - 1,
       __builtin_dynamic_object_size (__s, 1), __fmt, __ap);
}



extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) int
__attribute__ ((__leaf__)) snprintf (char *__restrict __s, size_t __n, const char *__restrict __fmt, ...) noexcept (true)

{
  return __builtin___snprintf_chk (__s, __n, 3 - 1,
       __builtin_dynamic_object_size (__s, 1), __fmt,
       __builtin_va_arg_pack ());
}
# 92 "/usr/include/x86_64-linux-gnu/bits/stdio2.h" 3 4
extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) int
__attribute__ ((__leaf__)) vsnprintf (char * __restrict __s, size_t __n, const char *__restrict __fmt, __gnuc_va_list __ap) noexcept (true)


    


{
  return __builtin___vsnprintf_chk (__s, __n, 3 - 1,
        __builtin_dynamic_object_size (__s, 1), __fmt, __ap);
}





extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) __attribute__ ((__nonnull__ (1))) int
fprintf (FILE *__restrict __stream, const char *__restrict __fmt, ...)
{
  return __fprintf_chk (__stream, 3 - 1, __fmt,
   __builtin_va_arg_pack ());
}

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) int
printf (const char *__restrict __fmt, ...)
{
  return __printf_chk (3 - 1, __fmt, __builtin_va_arg_pack ());
}
# 151 "/usr/include/x86_64-linux-gnu/bits/stdio2.h" 3 4
extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) int
vprintf (const char * __restrict __fmt,
  __gnuc_va_list __ap)
{

  return __vfprintf_chk (stdout, 3 - 1, __fmt, __ap);



}

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) __attribute__ ((__nonnull__ (1))) int
vfprintf (FILE *__restrict __stream,
   const char *__restrict __fmt, __gnuc_va_list __ap)
{
  return __vfprintf_chk (__stream, 3 - 1, __fmt, __ap);
}



extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) int
dprintf (int __fd, const char *__restrict __fmt, ...)
{
  return __dprintf_chk (__fd, 3 - 1, __fmt,
   __builtin_va_arg_pack ());
}
# 194 "/usr/include/x86_64-linux-gnu/bits/stdio2.h" 3 4
extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) int
vdprintf (int __fd, const char *__restrict __fmt, __gnuc_va_list __ap)
{
  return __vdprintf_chk (__fd, 3 - 1, __fmt, __ap);
}




extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) int
__attribute__ ((__leaf__)) asprintf (char **__restrict __ptr, const char *__restrict __fmt, ...) noexcept (true)
{
  return __asprintf_chk (__ptr, 3 - 1, __fmt,
    __builtin_va_arg_pack ());
}

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) int
__attribute__ ((__leaf__)) __asprintf (char **__restrict __ptr, const char *__restrict __fmt, ...) noexcept (true)

{
  return __asprintf_chk (__ptr, 3 - 1, __fmt,
    __builtin_va_arg_pack ());
}

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) int
__attribute__ ((__leaf__)) obstack_printf (struct obstack *__restrict __obstack, const char *__restrict __fmt, ...) noexcept (true)

{
  return __obstack_printf_chk (__obstack, 3 - 1, __fmt,
          __builtin_va_arg_pack ());
}
# 271 "/usr/include/x86_64-linux-gnu/bits/stdio2.h" 3 4
extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) int
__attribute__ ((__leaf__)) vasprintf (char **__restrict __ptr, const char *__restrict __fmt, __gnuc_va_list __ap) noexcept (true)

{
  return __vasprintf_chk (__ptr, 3 - 1, __fmt, __ap);
}

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) int
__attribute__ ((__leaf__)) obstack_vprintf (struct obstack *__restrict __obstack, const char *__restrict __fmt, __gnuc_va_list __ap) noexcept (true)

{
  return __obstack_vprintf_chk (__obstack, 3 - 1, __fmt,
    __ap);
}
# 303 "/usr/include/x86_64-linux-gnu/bits/stdio2.h" 3 4
extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) __attribute__ ((__warn_unused_result__))
__attribute__ ((__nonnull__ (3))) char *
fgets (char * __restrict __s, int __n,
       FILE *__restrict __stream)
    


{
  size_t __sz = __builtin_dynamic_object_size (__s, 1);
  if (((__builtin_constant_p (__sz) && (__sz) == (long unsigned int) -1) || (((__typeof (__n)) 0 < (__typeof (__n)) -1 || (__builtin_constant_p (__n) && (__n) > 0)) && __builtin_constant_p ((((long unsigned int) (__n)) <= ((__sz)) / ((sizeof (char))))) && (((long unsigned int) (__n)) <= ((__sz)) / ((sizeof (char)))))))
    return __fgets_alias (__s, __n, __stream);

  if ((((__typeof (__n)) 0 < (__typeof (__n)) -1 || (__builtin_constant_p (__n) && (__n) > 0)) && __builtin_constant_p ((((long unsigned int) (__n)) <= (__sz) / (sizeof (char)))) && !(((long unsigned int) (__n)) <= (__sz) / (sizeof (char)))))
    return __fgets_chk_warn (__s, __sz, __n, __stream);

  return __fgets_chk (__s, __sz, __n, __stream);
}

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (4))) size_t
fread (void * __restrict __ptr,
       size_t __size, size_t __n, FILE *__restrict __stream)
    



{
  size_t __sz = __builtin_dynamic_object_size (__ptr, 0);
  if (((__builtin_constant_p (__sz) && (__sz) == (long unsigned int) -1) || (((__typeof (__n)) 0 < (__typeof (__n)) -1 || (__builtin_constant_p (__n) && (__n) > 0)) && __builtin_constant_p ((((long unsigned int) (__n)) <= ((__sz)) / ((__size)))) && (((long unsigned int) (__n)) <= ((__sz)) / ((__size))))))
    return __fread_alias (__ptr, __size, __n, __stream);

  if ((((__typeof (__n)) 0 < (__typeof (__n)) -1 || (__builtin_constant_p (__n) && (__n) > 0)) && __builtin_constant_p ((((long unsigned int) (__n)) <= (__sz) / (__size))) && !(((long unsigned int) (__n)) <= (__sz) / (__size))))
    return __fread_chk_warn (__ptr, __sz, __size, __n, __stream);

  return __fread_chk (__ptr, __sz, __size, __n, __stream);
}


extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) __attribute__ ((__warn_unused_result__))
__attribute__ ((__nonnull__ (3))) char *
fgets_unlocked (char * __restrict __s,
  int __n, FILE *__restrict __stream)
    


{
  size_t __sz = __builtin_dynamic_object_size (__s, 1);
  if (((__builtin_constant_p (__sz) && (__sz) == (long unsigned int) -1) || (((__typeof (__n)) 0 < (__typeof (__n)) -1 || (__builtin_constant_p (__n) && (__n) > 0)) && __builtin_constant_p ((((long unsigned int) (__n)) <= ((__sz)) / ((sizeof (char))))) && (((long unsigned int) (__n)) <= ((__sz)) / ((sizeof (char)))))))
    return __fgets_unlocked_alias (__s, __n, __stream);

  if ((((__typeof (__n)) 0 < (__typeof (__n)) -1 || (__builtin_constant_p (__n) && (__n) > 0)) && __builtin_constant_p ((((long unsigned int) (__n)) <= (__sz) / (sizeof (char)))) && !(((long unsigned int) (__n)) <= (__sz) / (sizeof (char)))))
    return __fgets_unlocked_chk_warn (__s, __sz, __n, __stream);

  return __fgets_unlocked_chk (__s, __sz, __n, __stream);
}




extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (4))) size_t
fread_unlocked (void * __restrict __ptr,
  size_t __size, size_t __n, FILE *__restrict __stream)
    



{
  size_t __sz = __builtin_dynamic_object_size (__ptr, 0);
  if (((__builtin_constant_p (__sz) && (__sz) == (long unsigned int) -1) || (((__typeof (__n)) 0 < (__typeof (__n)) -1 || (__builtin_constant_p (__n) && (__n) > 0)) && __builtin_constant_p ((((long unsigned int) (__n)) <= ((__sz)) / ((__size)))) && (((long unsigned int) (__n)) <= ((__sz)) / ((__size))))))
    {

      if (__builtin_constant_p (__size)
   && __builtin_constant_p (__n)
   && (__size | __n) < (((size_t) 1) << (8 * sizeof (size_t) / 2))
   && __size * __n <= 8)
 {
   size_t __cnt = __size * __n;
   char *__cptr = (char *) __ptr;
   if (__cnt == 0)
     return 0;

   for (; __cnt > 0; --__cnt)
     {
       int __c = getc_unlocked (__stream);
       if (__c == (-1))
  break;
       *__cptr++ = __c;
     }
   return (__cptr - (char *) __ptr) / __size;
 }

      return __fread_unlocked_alias (__ptr, __size, __n, __stream);
    }

  if ((((__typeof (__n)) 0 < (__typeof (__n)) -1 || (__builtin_constant_p (__n) && (__n) > 0)) && __builtin_constant_p ((((long unsigned int) (__n)) <= (__sz) / (__size))) && !(((long unsigned int) (__n)) <= (__sz) / (__size))))
    return __fread_unlocked_chk_warn (__ptr, __sz, __size, __n, __stream);

  return __fread_unlocked_chk (__ptr, __sz, __size, __n, __stream);

}
# 971 "/usr/include/stdio.h" 2 3 4


}
# 47 "../../bochs.h" 2
# 1 "/usr/include/c++/14/stdlib.h" 1 3
# 36 "/usr/include/c++/14/stdlib.h" 3
# 1 "/usr/include/c++/14/cstdlib" 1 3
# 39 "/usr/include/c++/14/cstdlib" 3
       
# 40 "/usr/include/c++/14/cstdlib" 3

# 1 "/usr/include/x86_64-linux-gnu/c++/14/bits/c++config.h" 1 3
# 33 "/usr/include/x86_64-linux-gnu/c++/14/bits/c++config.h" 3
       
# 34 "/usr/include/x86_64-linux-gnu/c++/14/bits/c++config.h" 3
# 308 "/usr/include/x86_64-linux-gnu/c++/14/bits/c++config.h" 3
namespace std
{
  typedef long unsigned int size_t;
  typedef long int ptrdiff_t;


  typedef decltype(nullptr) nullptr_t;


#pragma GCC visibility push(default)


  extern "C++" __attribute__ ((__noreturn__, __always_inline__))
  inline void __terminate() noexcept
  {
    void terminate() noexcept __attribute__ ((__noreturn__,__cold__));
    terminate();
  }
#pragma GCC visibility pop
}
# 341 "/usr/include/x86_64-linux-gnu/c++/14/bits/c++config.h" 3
namespace std
{
  inline namespace __cxx11 __attribute__((__abi_tag__ ("cxx11"))) { }
}
namespace __gnu_cxx
{
  inline namespace __cxx11 __attribute__((__abi_tag__ ("cxx11"))) { }
}
# 534 "/usr/include/x86_64-linux-gnu/c++/14/bits/c++config.h" 3
namespace std
{
#pragma GCC visibility push(default)




  __attribute__((__always_inline__))
  constexpr inline bool
  __is_constant_evaluated() noexcept
  {





    return __builtin_is_constant_evaluated();



  }
#pragma GCC visibility pop
}
# 573 "/usr/include/x86_64-linux-gnu/c++/14/bits/c++config.h" 3
namespace std
{
#pragma GCC visibility push(default)

  extern "C++" __attribute__ ((__noreturn__))
  void
  __glibcxx_assert_fail
    (const char* __file, int __line, const char* __function,
     const char* __condition)
  noexcept;
#pragma GCC visibility pop
}
# 604 "/usr/include/x86_64-linux-gnu/c++/14/bits/c++config.h" 3
namespace std
{
  __attribute__((__always_inline__,__visibility__("default")))
  inline void
  __glibcxx_assert_fail()
  { }
}
# 683 "/usr/include/x86_64-linux-gnu/c++/14/bits/c++config.h" 3
# 1 "/usr/include/x86_64-linux-gnu/c++/14/bits/os_defines.h" 1 3
# 684 "/usr/include/x86_64-linux-gnu/c++/14/bits/c++config.h" 2 3


# 1 "/usr/include/x86_64-linux-gnu/c++/14/bits/cpu_defines.h" 1 3
# 687 "/usr/include/x86_64-linux-gnu/c++/14/bits/c++config.h" 2 3
# 828 "/usr/include/x86_64-linux-gnu/c++/14/bits/c++config.h" 3
namespace __gnu_cxx
{
  typedef __decltype(0.0bf16) __bfloat16_t;
}
# 890 "/usr/include/x86_64-linux-gnu/c++/14/bits/c++config.h" 3
# 1 "/usr/include/c++/14/pstl/pstl_config.h" 1 3
# 891 "/usr/include/x86_64-linux-gnu/c++/14/bits/c++config.h" 2 3
# 42 "/usr/include/c++/14/cstdlib" 2 3
# 79 "/usr/include/c++/14/cstdlib" 3
# 1 "/usr/include/stdlib.h" 1 3 4
# 26 "/usr/include/stdlib.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/libc-header-start.h" 1 3 4
# 27 "/usr/include/stdlib.h" 2 3 4





# 1 "/usr/lib/gcc/x86_64-linux-gnu/14/include/stddef.h" 1 3 4
# 33 "/usr/include/stdlib.h" 2 3 4

extern "C" {





# 1 "/usr/include/x86_64-linux-gnu/bits/waitflags.h" 1 3 4
# 41 "/usr/include/stdlib.h" 2 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/waitstatus.h" 1 3 4
# 42 "/usr/include/stdlib.h" 2 3 4
# 59 "/usr/include/stdlib.h" 3 4
typedef struct
  {
    int quot;
    int rem;
  } div_t;



typedef struct
  {
    long int quot;
    long int rem;
  } ldiv_t;





__extension__ typedef struct
  {
    long long int quot;
    long long int rem;
  } lldiv_t;
# 98 "/usr/include/stdlib.h" 3 4
extern size_t __ctype_get_mb_cur_max (void) noexcept (true) __attribute__ ((__warn_unused_result__));



extern double atof (const char *__nptr)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));

extern int atoi (const char *__nptr)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));

extern long int atol (const char *__nptr)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));



__extension__ extern long long int atoll (const char *__nptr)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));



extern double strtod (const char *__restrict __nptr,
        char **__restrict __endptr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));



extern float strtof (const char *__restrict __nptr,
       char **__restrict __endptr) noexcept (true) __attribute__ ((__nonnull__ (1)));

extern long double strtold (const char *__restrict __nptr,
       char **__restrict __endptr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));
# 141 "/usr/include/stdlib.h" 3 4
extern _Float32 strtof32 (const char *__restrict __nptr,
     char **__restrict __endptr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));



extern _Float64 strtof64 (const char *__restrict __nptr,
     char **__restrict __endptr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));



extern _Float128 strtof128 (const char *__restrict __nptr,
       char **__restrict __endptr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));



extern _Float32x strtof32x (const char *__restrict __nptr,
       char **__restrict __endptr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));



extern _Float64x strtof64x (const char *__restrict __nptr,
       char **__restrict __endptr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));
# 177 "/usr/include/stdlib.h" 3 4
extern long int strtol (const char *__restrict __nptr,
   char **__restrict __endptr, int __base)
     noexcept (true) __attribute__ ((__nonnull__ (1)));

extern unsigned long int strtoul (const char *__restrict __nptr,
      char **__restrict __endptr, int __base)
     noexcept (true) __attribute__ ((__nonnull__ (1)));



__extension__
extern long long int strtoq (const char *__restrict __nptr,
        char **__restrict __endptr, int __base)
     noexcept (true) __attribute__ ((__nonnull__ (1)));

__extension__
extern unsigned long long int strtouq (const char *__restrict __nptr,
           char **__restrict __endptr, int __base)
     noexcept (true) __attribute__ ((__nonnull__ (1)));




__extension__
extern long long int strtoll (const char *__restrict __nptr,
         char **__restrict __endptr, int __base)
     noexcept (true) __attribute__ ((__nonnull__ (1)));

__extension__
extern unsigned long long int strtoull (const char *__restrict __nptr,
     char **__restrict __endptr, int __base)
     noexcept (true) __attribute__ ((__nonnull__ (1)));






extern long int strtol (const char *__restrict __nptr, char **__restrict __endptr, int __base) noexcept (true) __asm__ ("" "__isoc23_strtol")


     __attribute__ ((__nonnull__ (1)));
extern unsigned long int strtoul (const char *__restrict __nptr, char **__restrict __endptr, int __base) noexcept (true) __asm__ ("" "__isoc23_strtoul")



     __attribute__ ((__nonnull__ (1)));

__extension__
extern long long int strtoq (const char *__restrict __nptr, char **__restrict __endptr, int __base) noexcept (true) __asm__ ("" "__isoc23_strtoll")


     __attribute__ ((__nonnull__ (1)));
__extension__
extern unsigned long long int strtouq (const char *__restrict __nptr, char **__restrict __endptr, int __base) noexcept (true) __asm__ ("" "__isoc23_strtoull")



     __attribute__ ((__nonnull__ (1)));

__extension__
extern long long int strtoll (const char *__restrict __nptr, char **__restrict __endptr, int __base) noexcept (true) __asm__ ("" "__isoc23_strtoll")


     __attribute__ ((__nonnull__ (1)));
__extension__
extern unsigned long long int strtoull (const char *__restrict __nptr, char **__restrict __endptr, int __base) noexcept (true) __asm__ ("" "__isoc23_strtoull")



     __attribute__ ((__nonnull__ (1)));
# 278 "/usr/include/stdlib.h" 3 4
extern int strfromd (char *__dest, size_t __size, const char *__format,
       double __f)
     noexcept (true) __attribute__ ((__nonnull__ (3)));

extern int strfromf (char *__dest, size_t __size, const char *__format,
       float __f)
     noexcept (true) __attribute__ ((__nonnull__ (3)));

extern int strfroml (char *__dest, size_t __size, const char *__format,
       long double __f)
     noexcept (true) __attribute__ ((__nonnull__ (3)));
# 298 "/usr/include/stdlib.h" 3 4
extern int strfromf32 (char *__dest, size_t __size, const char * __format,
         _Float32 __f)
     noexcept (true) __attribute__ ((__nonnull__ (3)));



extern int strfromf64 (char *__dest, size_t __size, const char * __format,
         _Float64 __f)
     noexcept (true) __attribute__ ((__nonnull__ (3)));



extern int strfromf128 (char *__dest, size_t __size, const char * __format,
   _Float128 __f)
     noexcept (true) __attribute__ ((__nonnull__ (3)));



extern int strfromf32x (char *__dest, size_t __size, const char * __format,
   _Float32x __f)
     noexcept (true) __attribute__ ((__nonnull__ (3)));



extern int strfromf64x (char *__dest, size_t __size, const char * __format,
   _Float64x __f)
     noexcept (true) __attribute__ ((__nonnull__ (3)));
# 338 "/usr/include/stdlib.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/types/locale_t.h" 1 3 4
# 22 "/usr/include/x86_64-linux-gnu/bits/types/locale_t.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/types/__locale_t.h" 1 3 4
# 27 "/usr/include/x86_64-linux-gnu/bits/types/__locale_t.h" 3 4
struct __locale_struct
{

  struct __locale_data *__locales[13];


  const unsigned short int *__ctype_b;
  const int *__ctype_tolower;
  const int *__ctype_toupper;


  const char *__names[13];
};

typedef struct __locale_struct *__locale_t;
# 23 "/usr/include/x86_64-linux-gnu/bits/types/locale_t.h" 2 3 4

typedef __locale_t locale_t;
# 339 "/usr/include/stdlib.h" 2 3 4

extern long int strtol_l (const char *__restrict __nptr,
     char **__restrict __endptr, int __base,
     locale_t __loc) noexcept (true) __attribute__ ((__nonnull__ (1, 4)));

extern unsigned long int strtoul_l (const char *__restrict __nptr,
        char **__restrict __endptr,
        int __base, locale_t __loc)
     noexcept (true) __attribute__ ((__nonnull__ (1, 4)));

__extension__
extern long long int strtoll_l (const char *__restrict __nptr,
    char **__restrict __endptr, int __base,
    locale_t __loc)
     noexcept (true) __attribute__ ((__nonnull__ (1, 4)));

__extension__
extern unsigned long long int strtoull_l (const char *__restrict __nptr,
       char **__restrict __endptr,
       int __base, locale_t __loc)
     noexcept (true) __attribute__ ((__nonnull__ (1, 4)));





extern long int strtol_l (const char *__restrict __nptr, char **__restrict __endptr, int __base, locale_t __loc) noexcept (true) __asm__ ("" "__isoc23_strtol_l")



     __attribute__ ((__nonnull__ (1, 4)));
extern unsigned long int strtoul_l (const char *__restrict __nptr, char **__restrict __endptr, int __base, locale_t __loc) noexcept (true) __asm__ ("" "__isoc23_strtoul_l")




     __attribute__ ((__nonnull__ (1, 4)));
__extension__
extern long long int strtoll_l (const char *__restrict __nptr, char **__restrict __endptr, int __base, locale_t __loc) noexcept (true) __asm__ ("" "__isoc23_strtoll_l")




     __attribute__ ((__nonnull__ (1, 4)));
__extension__
extern unsigned long long int strtoull_l (const char *__restrict __nptr, char **__restrict __endptr, int __base, locale_t __loc) noexcept (true) __asm__ ("" "__isoc23_strtoull_l")




     __attribute__ ((__nonnull__ (1, 4)));
# 415 "/usr/include/stdlib.h" 3 4
extern double strtod_l (const char *__restrict __nptr,
   char **__restrict __endptr, locale_t __loc)
     noexcept (true) __attribute__ ((__nonnull__ (1, 3)));

extern float strtof_l (const char *__restrict __nptr,
         char **__restrict __endptr, locale_t __loc)
     noexcept (true) __attribute__ ((__nonnull__ (1, 3)));

extern long double strtold_l (const char *__restrict __nptr,
         char **__restrict __endptr,
         locale_t __loc)
     noexcept (true) __attribute__ ((__nonnull__ (1, 3)));
# 436 "/usr/include/stdlib.h" 3 4
extern _Float32 strtof32_l (const char *__restrict __nptr,
       char **__restrict __endptr,
       locale_t __loc)
     noexcept (true) __attribute__ ((__nonnull__ (1, 3)));



extern _Float64 strtof64_l (const char *__restrict __nptr,
       char **__restrict __endptr,
       locale_t __loc)
     noexcept (true) __attribute__ ((__nonnull__ (1, 3)));



extern _Float128 strtof128_l (const char *__restrict __nptr,
         char **__restrict __endptr,
         locale_t __loc)
     noexcept (true) __attribute__ ((__nonnull__ (1, 3)));



extern _Float32x strtof32x_l (const char *__restrict __nptr,
         char **__restrict __endptr,
         locale_t __loc)
     noexcept (true) __attribute__ ((__nonnull__ (1, 3)));



extern _Float64x strtof64x_l (const char *__restrict __nptr,
         char **__restrict __endptr,
         locale_t __loc)
     noexcept (true) __attribute__ ((__nonnull__ (1, 3)));
# 480 "/usr/include/stdlib.h" 3 4
extern __inline __attribute__ ((__gnu_inline__)) int
__attribute__ ((__leaf__)) atoi (const char *__nptr) noexcept (true)
{
  return (int) strtol (__nptr, (char **) __null, 10);
}
extern __inline __attribute__ ((__gnu_inline__)) long int
__attribute__ ((__leaf__)) atol (const char *__nptr) noexcept (true)
{
  return strtol (__nptr, (char **) __null, 10);
}


__extension__ extern __inline __attribute__ ((__gnu_inline__)) long long int
__attribute__ ((__leaf__)) atoll (const char *__nptr) noexcept (true)
{
  return strtoll (__nptr, (char **) __null, 10);
}
# 505 "/usr/include/stdlib.h" 3 4
extern char *l64a (long int __n) noexcept (true) __attribute__ ((__warn_unused_result__));


extern long int a64l (const char *__s)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));




# 1 "/usr/include/x86_64-linux-gnu/sys/types.h" 1 3 4
# 27 "/usr/include/x86_64-linux-gnu/sys/types.h" 3 4
extern "C" {





typedef __u_char u_char;
typedef __u_short u_short;
typedef __u_int u_int;
typedef __u_long u_long;
typedef __quad_t quad_t;
typedef __u_quad_t u_quad_t;
typedef __fsid_t fsid_t;


typedef __loff_t loff_t;






typedef __ino64_t ino_t;




typedef __ino64_t ino64_t;




typedef __dev_t dev_t;




typedef __gid_t gid_t;




typedef __mode_t mode_t;




typedef __nlink_t nlink_t;




typedef __uid_t uid_t;
# 97 "/usr/include/x86_64-linux-gnu/sys/types.h" 3 4
typedef __pid_t pid_t;





typedef __id_t id_t;
# 114 "/usr/include/x86_64-linux-gnu/sys/types.h" 3 4
typedef __daddr_t daddr_t;
typedef __caddr_t caddr_t;





typedef __key_t key_t;




# 1 "/usr/include/x86_64-linux-gnu/bits/types/clock_t.h" 1 3 4






typedef __clock_t clock_t;
# 127 "/usr/include/x86_64-linux-gnu/sys/types.h" 2 3 4

# 1 "/usr/include/x86_64-linux-gnu/bits/types/clockid_t.h" 1 3 4






typedef __clockid_t clockid_t;
# 129 "/usr/include/x86_64-linux-gnu/sys/types.h" 2 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/types/time_t.h" 1 3 4
# 10 "/usr/include/x86_64-linux-gnu/bits/types/time_t.h" 3 4
typedef __time_t time_t;
# 130 "/usr/include/x86_64-linux-gnu/sys/types.h" 2 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/types/timer_t.h" 1 3 4






typedef __timer_t timer_t;
# 131 "/usr/include/x86_64-linux-gnu/sys/types.h" 2 3 4



typedef __useconds_t useconds_t;



typedef __suseconds_t suseconds_t;





# 1 "/usr/lib/gcc/x86_64-linux-gnu/14/include/stddef.h" 1 3 4
# 145 "/usr/include/x86_64-linux-gnu/sys/types.h" 2 3 4



typedef unsigned long int ulong;
typedef unsigned short int ushort;
typedef unsigned int uint;




# 1 "/usr/include/x86_64-linux-gnu/bits/stdint-intn.h" 1 3 4
# 24 "/usr/include/x86_64-linux-gnu/bits/stdint-intn.h" 3 4
typedef __int8_t int8_t;
typedef __int16_t int16_t;
typedef __int32_t int32_t;
typedef __int64_t int64_t;
# 156 "/usr/include/x86_64-linux-gnu/sys/types.h" 2 3 4


typedef __uint8_t u_int8_t;
typedef __uint16_t u_int16_t;
typedef __uint32_t u_int32_t;
typedef __uint64_t u_int64_t;


typedef int register_t __attribute__ ((__mode__ (__word__)));
# 176 "/usr/include/x86_64-linux-gnu/sys/types.h" 3 4
# 1 "/usr/include/endian.h" 1 3 4
# 24 "/usr/include/endian.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/endian.h" 1 3 4
# 35 "/usr/include/x86_64-linux-gnu/bits/endian.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/endianness.h" 1 3 4
# 36 "/usr/include/x86_64-linux-gnu/bits/endian.h" 2 3 4
# 25 "/usr/include/endian.h" 2 3 4
# 35 "/usr/include/endian.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/byteswap.h" 1 3 4
# 33 "/usr/include/x86_64-linux-gnu/bits/byteswap.h" 3 4
static __inline __uint16_t
__bswap_16 (__uint16_t __bsx)
{

  return __builtin_bswap16 (__bsx);



}






static __inline __uint32_t
__bswap_32 (__uint32_t __bsx)
{

  return __builtin_bswap32 (__bsx);



}
# 69 "/usr/include/x86_64-linux-gnu/bits/byteswap.h" 3 4
__extension__ static __inline __uint64_t
__bswap_64 (__uint64_t __bsx)
{

  return __builtin_bswap64 (__bsx);



}
# 36 "/usr/include/endian.h" 2 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/uintn-identity.h" 1 3 4
# 32 "/usr/include/x86_64-linux-gnu/bits/uintn-identity.h" 3 4
static __inline __uint16_t
__uint16_identity (__uint16_t __x)
{
  return __x;
}

static __inline __uint32_t
__uint32_identity (__uint32_t __x)
{
  return __x;
}

static __inline __uint64_t
__uint64_identity (__uint64_t __x)
{
  return __x;
}
# 37 "/usr/include/endian.h" 2 3 4
# 177 "/usr/include/x86_64-linux-gnu/sys/types.h" 2 3 4


# 1 "/usr/include/x86_64-linux-gnu/sys/select.h" 1 3 4
# 30 "/usr/include/x86_64-linux-gnu/sys/select.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/select.h" 1 3 4
# 31 "/usr/include/x86_64-linux-gnu/sys/select.h" 2 3 4


# 1 "/usr/include/x86_64-linux-gnu/bits/types/sigset_t.h" 1 3 4



# 1 "/usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h" 1 3 4




typedef struct
{
  unsigned long int __val[(1024 / (8 * sizeof (unsigned long int)))];
} __sigset_t;
# 5 "/usr/include/x86_64-linux-gnu/bits/types/sigset_t.h" 2 3 4


typedef __sigset_t sigset_t;
# 34 "/usr/include/x86_64-linux-gnu/sys/select.h" 2 3 4



# 1 "/usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h" 1 3 4







struct timeval
{




  __time_t tv_sec;
  __suseconds_t tv_usec;

};
# 38 "/usr/include/x86_64-linux-gnu/sys/select.h" 2 3 4

# 1 "/usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h" 1 3 4
# 11 "/usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h" 3 4
struct timespec
{



  __time_t tv_sec;




  __syscall_slong_t tv_nsec;
# 31 "/usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h" 3 4
};
# 40 "/usr/include/x86_64-linux-gnu/sys/select.h" 2 3 4
# 49 "/usr/include/x86_64-linux-gnu/sys/select.h" 3 4
typedef long int __fd_mask;
# 59 "/usr/include/x86_64-linux-gnu/sys/select.h" 3 4
typedef struct
  {



    __fd_mask fds_bits[1024 / (8 * (int) sizeof (__fd_mask))];





  } fd_set;






typedef __fd_mask fd_mask;
# 91 "/usr/include/x86_64-linux-gnu/sys/select.h" 3 4
extern "C" {
# 102 "/usr/include/x86_64-linux-gnu/sys/select.h" 3 4
extern int select (int __nfds, fd_set *__restrict __readfds,
     fd_set *__restrict __writefds,
     fd_set *__restrict __exceptfds,
     struct timeval *__restrict __timeout);
# 127 "/usr/include/x86_64-linux-gnu/sys/select.h" 3 4
extern int pselect (int __nfds, fd_set *__restrict __readfds,
      fd_set *__restrict __writefds,
      fd_set *__restrict __exceptfds,
      const struct timespec *__restrict __timeout,
      const __sigset_t *__restrict __sigmask);
# 150 "/usr/include/x86_64-linux-gnu/sys/select.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/select2.h" 1 3 4
# 23 "/usr/include/x86_64-linux-gnu/bits/select2.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/select-decl.h" 1 3 4
# 27 "/usr/include/x86_64-linux-gnu/bits/select-decl.h" 3 4
extern long int __fdelt_chk (long int __d);
extern long int __fdelt_warn (long int __d)
  __attribute__((__warning__ ("bit outside of fd_set selected")));
# 24 "/usr/include/x86_64-linux-gnu/bits/select2.h" 2 3 4
# 151 "/usr/include/x86_64-linux-gnu/sys/select.h" 2 3 4


}
# 180 "/usr/include/x86_64-linux-gnu/sys/types.h" 2 3 4





typedef __blksize_t blksize_t;
# 205 "/usr/include/x86_64-linux-gnu/sys/types.h" 3 4
typedef __blkcnt64_t blkcnt_t;



typedef __fsblkcnt64_t fsblkcnt_t;



typedef __fsfilcnt64_t fsfilcnt_t;





typedef __blkcnt64_t blkcnt64_t;
typedef __fsblkcnt64_t fsblkcnt64_t;
typedef __fsfilcnt64_t fsfilcnt64_t;





# 1 "/usr/include/x86_64-linux-gnu/bits/pthreadtypes.h" 1 3 4
# 23 "/usr/include/x86_64-linux-gnu/bits/pthreadtypes.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/thread-shared-types.h" 1 3 4
# 44 "/usr/include/x86_64-linux-gnu/bits/thread-shared-types.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h" 1 3 4
# 21 "/usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/wordsize.h" 1 3 4
# 22 "/usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h" 2 3 4
# 45 "/usr/include/x86_64-linux-gnu/bits/thread-shared-types.h" 2 3 4

# 1 "/usr/include/x86_64-linux-gnu/bits/atomic_wide_counter.h" 1 3 4
# 25 "/usr/include/x86_64-linux-gnu/bits/atomic_wide_counter.h" 3 4
typedef union
{
  __extension__ unsigned long long int __value64;
  struct
  {
    unsigned int __low;
    unsigned int __high;
  } __value32;
} __atomic_wide_counter;
# 47 "/usr/include/x86_64-linux-gnu/bits/thread-shared-types.h" 2 3 4




typedef struct __pthread_internal_list
{
  struct __pthread_internal_list *__prev;
  struct __pthread_internal_list *__next;
} __pthread_list_t;

typedef struct __pthread_internal_slist
{
  struct __pthread_internal_slist *__next;
} __pthread_slist_t;
# 76 "/usr/include/x86_64-linux-gnu/bits/thread-shared-types.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/struct_mutex.h" 1 3 4
# 22 "/usr/include/x86_64-linux-gnu/bits/struct_mutex.h" 3 4
struct __pthread_mutex_s
{
  int __lock;
  unsigned int __count;
  int __owner;

  unsigned int __nusers;



  int __kind;

  short __spins;
  short __elision;
  __pthread_list_t __list;
# 53 "/usr/include/x86_64-linux-gnu/bits/struct_mutex.h" 3 4
};
# 77 "/usr/include/x86_64-linux-gnu/bits/thread-shared-types.h" 2 3 4
# 89 "/usr/include/x86_64-linux-gnu/bits/thread-shared-types.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/struct_rwlock.h" 1 3 4
# 23 "/usr/include/x86_64-linux-gnu/bits/struct_rwlock.h" 3 4
struct __pthread_rwlock_arch_t
{
  unsigned int __readers;
  unsigned int __writers;
  unsigned int __wrphase_futex;
  unsigned int __writers_futex;
  unsigned int __pad3;
  unsigned int __pad4;

  int __cur_writer;
  int __shared;
  signed char __rwelision;




  unsigned char __pad1[7];


  unsigned long int __pad2;


  unsigned int __flags;
# 55 "/usr/include/x86_64-linux-gnu/bits/struct_rwlock.h" 3 4
};
# 90 "/usr/include/x86_64-linux-gnu/bits/thread-shared-types.h" 2 3 4




struct __pthread_cond_s
{
  __atomic_wide_counter __wseq;
  __atomic_wide_counter __g1_start;
  unsigned int __g_size[2] ;
  unsigned int __g1_orig_size;
  unsigned int __wrefs;
  unsigned int __g_signals[2];
  unsigned int __unused_initialized_1;
  unsigned int __unused_initialized_2;
};

typedef unsigned int __tss_t;
typedef unsigned long int __thrd_t;

typedef struct
{
  int __data ;
} __once_flag;
# 24 "/usr/include/x86_64-linux-gnu/bits/pthreadtypes.h" 2 3 4



typedef unsigned long int pthread_t;




typedef union
{
  char __size[4];
  int __align;
} pthread_mutexattr_t;




typedef union
{
  char __size[4];
  int __align;
} pthread_condattr_t;



typedef unsigned int pthread_key_t;



typedef int pthread_once_t;


union pthread_attr_t
{
  char __size[56];
  long int __align;
};

typedef union pthread_attr_t pthread_attr_t;




typedef union
{
  struct __pthread_mutex_s __data;
  char __size[40];
  long int __align;
} pthread_mutex_t;


typedef union
{
  struct __pthread_cond_s __data;
  char __size[48];
  __extension__ long long int __align;
} pthread_cond_t;





typedef union
{
  struct __pthread_rwlock_arch_t __data;
  char __size[56];
  long int __align;
} pthread_rwlock_t;

typedef union
{
  char __size[8];
  long int __align;
} pthread_rwlockattr_t;





typedef volatile int pthread_spinlock_t;




typedef union
{
  char __size[32];
  long int __align;
} pthread_barrier_t;

typedef union
{
  char __size[4];
  int __align;
} pthread_barrierattr_t;
# 228 "/usr/include/x86_64-linux-gnu/sys/types.h" 2 3 4


}
# 515 "/usr/include/stdlib.h" 2 3 4






extern long int random (void) noexcept (true);


extern void srandom (unsigned int __seed) noexcept (true);





extern char *initstate (unsigned int __seed, char *__statebuf,
   size_t __statelen) noexcept (true) __attribute__ ((__nonnull__ (2)));



extern char *setstate (char *__statebuf) noexcept (true) __attribute__ ((__nonnull__ (1)));







struct random_data
  {
    int32_t *fptr;
    int32_t *rptr;
    int32_t *state;
    int rand_type;
    int rand_deg;
    int rand_sep;
    int32_t *end_ptr;
  };

extern int random_r (struct random_data *__restrict __buf,
       int32_t *__restrict __result) noexcept (true) __attribute__ ((__nonnull__ (1, 2)));

extern int srandom_r (unsigned int __seed, struct random_data *__buf)
     noexcept (true) __attribute__ ((__nonnull__ (2)));

extern int initstate_r (unsigned int __seed, char *__restrict __statebuf,
   size_t __statelen,
   struct random_data *__restrict __buf)
     noexcept (true) __attribute__ ((__nonnull__ (2, 4)));

extern int setstate_r (char *__restrict __statebuf,
         struct random_data *__restrict __buf)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));





extern int rand (void) noexcept (true);

extern void srand (unsigned int __seed) noexcept (true);



extern int rand_r (unsigned int *__seed) noexcept (true);







extern double drand48 (void) noexcept (true);
extern double erand48 (unsigned short int __xsubi[3]) noexcept (true) __attribute__ ((__nonnull__ (1)));


extern long int lrand48 (void) noexcept (true);
extern long int nrand48 (unsigned short int __xsubi[3])
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern long int mrand48 (void) noexcept (true);
extern long int jrand48 (unsigned short int __xsubi[3])
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern void srand48 (long int __seedval) noexcept (true);
extern unsigned short int *seed48 (unsigned short int __seed16v[3])
     noexcept (true) __attribute__ ((__nonnull__ (1)));
extern void lcong48 (unsigned short int __param[7]) noexcept (true) __attribute__ ((__nonnull__ (1)));





struct drand48_data
  {
    unsigned short int __x[3];
    unsigned short int __old_x[3];
    unsigned short int __c;
    unsigned short int __init;
    __extension__ unsigned long long int __a;

  };


extern int drand48_r (struct drand48_data *__restrict __buffer,
        double *__restrict __result) noexcept (true) __attribute__ ((__nonnull__ (1, 2)));
extern int erand48_r (unsigned short int __xsubi[3],
        struct drand48_data *__restrict __buffer,
        double *__restrict __result) noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern int lrand48_r (struct drand48_data *__restrict __buffer,
        long int *__restrict __result)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));
extern int nrand48_r (unsigned short int __xsubi[3],
        struct drand48_data *__restrict __buffer,
        long int *__restrict __result)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern int mrand48_r (struct drand48_data *__restrict __buffer,
        long int *__restrict __result)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));
extern int jrand48_r (unsigned short int __xsubi[3],
        struct drand48_data *__restrict __buffer,
        long int *__restrict __result)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern int srand48_r (long int __seedval, struct drand48_data *__buffer)
     noexcept (true) __attribute__ ((__nonnull__ (2)));

extern int seed48_r (unsigned short int __seed16v[3],
       struct drand48_data *__buffer) noexcept (true) __attribute__ ((__nonnull__ (1, 2)));

extern int lcong48_r (unsigned short int __param[7],
        struct drand48_data *__buffer)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern __uint32_t arc4random (void)
     noexcept (true) __attribute__ ((__warn_unused_result__));


extern void arc4random_buf (void *__buf, size_t __size)
     noexcept (true) __attribute__ ((__nonnull__ (1)));



extern __uint32_t arc4random_uniform (__uint32_t __upper_bound)
     noexcept (true) __attribute__ ((__warn_unused_result__));




extern void *malloc (size_t __size) noexcept (true) __attribute__ ((__malloc__))
     __attribute__ ((__alloc_size__ (1))) __attribute__ ((__warn_unused_result__));

extern void *calloc (size_t __nmemb, size_t __size)
     noexcept (true) __attribute__ ((__malloc__)) __attribute__ ((__alloc_size__ (1, 2))) __attribute__ ((__warn_unused_result__));






extern void *realloc (void *__ptr, size_t __size)
     noexcept (true) __attribute__ ((__warn_unused_result__)) __attribute__ ((__alloc_size__ (2)));


extern void free (void *__ptr) noexcept (true);







extern void *reallocarray (void *__ptr, size_t __nmemb, size_t __size)
     noexcept (true) __attribute__ ((__warn_unused_result__))
     __attribute__ ((__alloc_size__ (2, 3)))
    __attribute__ ((__malloc__ (__builtin_free, 1)));


extern void *reallocarray (void *__ptr, size_t __nmemb, size_t __size)
     noexcept (true) __attribute__ ((__malloc__ (reallocarray, 1)));



# 1 "/usr/include/alloca.h" 1 3 4
# 24 "/usr/include/alloca.h" 3 4
# 1 "/usr/lib/gcc/x86_64-linux-gnu/14/include/stddef.h" 1 3 4
# 25 "/usr/include/alloca.h" 2 3 4

extern "C" {





extern void *alloca (size_t __size) noexcept (true);





}
# 707 "/usr/include/stdlib.h" 2 3 4





extern void *valloc (size_t __size) noexcept (true) __attribute__ ((__malloc__))
     __attribute__ ((__alloc_size__ (1))) __attribute__ ((__warn_unused_result__));




extern int posix_memalign (void **__memptr, size_t __alignment, size_t __size)
     noexcept (true) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));




extern void *aligned_alloc (size_t __alignment, size_t __size)
     noexcept (true) __attribute__ ((__malloc__)) __attribute__ ((__alloc_align__ (1)))
     __attribute__ ((__alloc_size__ (2))) __attribute__ ((__warn_unused_result__));



extern void abort (void) noexcept (true) __attribute__ ((__noreturn__)) __attribute__ ((__cold__));



extern int atexit (void (*__func) (void)) noexcept (true) __attribute__ ((__nonnull__ (1)));




extern "C++" int at_quick_exit (void (*__func) (void))
     noexcept (true) __asm ("at_quick_exit") __attribute__ ((__nonnull__ (1)));
# 749 "/usr/include/stdlib.h" 3 4
extern int on_exit (void (*__func) (int __status, void *__arg), void *__arg)
     noexcept (true) __attribute__ ((__nonnull__ (1)));





extern void exit (int __status) noexcept (true) __attribute__ ((__noreturn__));





extern void quick_exit (int __status) noexcept (true) __attribute__ ((__noreturn__));





extern void _Exit (int __status) noexcept (true) __attribute__ ((__noreturn__));




extern char *getenv (const char *__name) noexcept (true) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));




extern char *secure_getenv (const char *__name)
     noexcept (true) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));






extern int putenv (char *__string) noexcept (true) __attribute__ ((__nonnull__ (1)));





extern int setenv (const char *__name, const char *__value, int __replace)
     noexcept (true) __attribute__ ((__nonnull__ (2)));


extern int unsetenv (const char *__name) noexcept (true) __attribute__ ((__nonnull__ (1)));






extern int clearenv (void) noexcept (true);
# 814 "/usr/include/stdlib.h" 3 4
extern char *mktemp (char *__template) noexcept (true) __attribute__ ((__nonnull__ (1)));
# 830 "/usr/include/stdlib.h" 3 4
extern int mkstemp (char *__template) __asm__ ("" "mkstemp64")
     __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));





extern int mkstemp64 (char *__template) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));
# 852 "/usr/include/stdlib.h" 3 4
extern int mkstemps (char *__template, int __suffixlen) __asm__ ("" "mkstemps64")
                     __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));





extern int mkstemps64 (char *__template, int __suffixlen)
     __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));
# 870 "/usr/include/stdlib.h" 3 4
extern char *mkdtemp (char *__template) noexcept (true) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));
# 884 "/usr/include/stdlib.h" 3 4
extern int mkostemp (char *__template, int __flags) __asm__ ("" "mkostemp64")
     __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));





extern int mkostemp64 (char *__template, int __flags) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));
# 905 "/usr/include/stdlib.h" 3 4
extern int mkostemps (char *__template, int __suffixlen, int __flags) __asm__ ("" "mkostemps64")

     __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));





extern int mkostemps64 (char *__template, int __suffixlen, int __flags)
     __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));
# 923 "/usr/include/stdlib.h" 3 4
extern int system (const char *__command) __attribute__ ((__warn_unused_result__));





extern char *canonicalize_file_name (const char *__name)
     noexcept (true) __attribute__ ((__nonnull__ (1))) __attribute__ ((__malloc__))
     __attribute__ ((__malloc__ (__builtin_free, 1))) __attribute__ ((__warn_unused_result__));
# 940 "/usr/include/stdlib.h" 3 4
extern char *realpath (const char *__restrict __name,
         char *__restrict __resolved) noexcept (true) __attribute__ ((__warn_unused_result__));






typedef int (*__compar_fn_t) (const void *, const void *);


typedef __compar_fn_t comparison_fn_t;



typedef int (*__compar_d_fn_t) (const void *, const void *, void *);




extern void *bsearch (const void *__key, const void *__base,
        size_t __nmemb, size_t __size, __compar_fn_t __compar)
     __attribute__ ((__nonnull__ (1, 2, 5))) __attribute__ ((__warn_unused_result__));


# 1 "/usr/include/x86_64-linux-gnu/bits/stdlib-bsearch.h" 1 3 4
# 19 "/usr/include/x86_64-linux-gnu/bits/stdlib-bsearch.h" 3 4
extern __inline __attribute__ ((__gnu_inline__)) void *
bsearch (const void *__key, const void *__base, size_t __nmemb, size_t __size,
  __compar_fn_t __compar)
{
  const void *__p;
  int __comparison;

  while (__nmemb)
    {
      __p = (const void *) (((const char *) __base) + ((__nmemb >> 1) * __size));
      __comparison = (*__compar) (__key, __p);
      if (__comparison == 0)
 {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"

   return (void *) __p;

#pragma GCC diagnostic pop

 }
      if (__comparison > 0)
 {
   __base = ((const char *) __p) + __size;
   --__nmemb;
 }
      __nmemb >>= 1;
    }

  return __null;
}
# 966 "/usr/include/stdlib.h" 2 3 4




extern void qsort (void *__base, size_t __nmemb, size_t __size,
     __compar_fn_t __compar) __attribute__ ((__nonnull__ (1, 4)));

extern void qsort_r (void *__base, size_t __nmemb, size_t __size,
       __compar_d_fn_t __compar, void *__arg)
  __attribute__ ((__nonnull__ (1, 4)));




extern int abs (int __x) noexcept (true) __attribute__ ((__const__)) __attribute__ ((__warn_unused_result__));
extern long int labs (long int __x) noexcept (true) __attribute__ ((__const__)) __attribute__ ((__warn_unused_result__));


__extension__ extern long long int llabs (long long int __x)
     noexcept (true) __attribute__ ((__const__)) __attribute__ ((__warn_unused_result__));






extern div_t div (int __numer, int __denom)
     noexcept (true) __attribute__ ((__const__)) __attribute__ ((__warn_unused_result__));
extern ldiv_t ldiv (long int __numer, long int __denom)
     noexcept (true) __attribute__ ((__const__)) __attribute__ ((__warn_unused_result__));


__extension__ extern lldiv_t lldiv (long long int __numer,
        long long int __denom)
     noexcept (true) __attribute__ ((__const__)) __attribute__ ((__warn_unused_result__));
# 1012 "/usr/include/stdlib.h" 3 4
extern char *ecvt (double __value, int __ndigit, int *__restrict __decpt,
     int *__restrict __sign) noexcept (true) __attribute__ ((__nonnull__ (3, 4))) __attribute__ ((__warn_unused_result__));




extern char *fcvt (double __value, int __ndigit, int *__restrict __decpt,
     int *__restrict __sign) noexcept (true) __attribute__ ((__nonnull__ (3, 4))) __attribute__ ((__warn_unused_result__));




extern char *gcvt (double __value, int __ndigit, char *__buf)
     noexcept (true) __attribute__ ((__nonnull__ (3))) __attribute__ ((__warn_unused_result__));




extern char *qecvt (long double __value, int __ndigit,
      int *__restrict __decpt, int *__restrict __sign)
     noexcept (true) __attribute__ ((__nonnull__ (3, 4))) __attribute__ ((__warn_unused_result__));
extern char *qfcvt (long double __value, int __ndigit,
      int *__restrict __decpt, int *__restrict __sign)
     noexcept (true) __attribute__ ((__nonnull__ (3, 4))) __attribute__ ((__warn_unused_result__));
extern char *qgcvt (long double __value, int __ndigit, char *__buf)
     noexcept (true) __attribute__ ((__nonnull__ (3))) __attribute__ ((__warn_unused_result__));




extern int ecvt_r (double __value, int __ndigit, int *__restrict __decpt,
     int *__restrict __sign, char *__restrict __buf,
     size_t __len) noexcept (true) __attribute__ ((__nonnull__ (3, 4, 5)));
extern int fcvt_r (double __value, int __ndigit, int *__restrict __decpt,
     int *__restrict __sign, char *__restrict __buf,
     size_t __len) noexcept (true) __attribute__ ((__nonnull__ (3, 4, 5)));

extern int qecvt_r (long double __value, int __ndigit,
      int *__restrict __decpt, int *__restrict __sign,
      char *__restrict __buf, size_t __len)
     noexcept (true) __attribute__ ((__nonnull__ (3, 4, 5)));
extern int qfcvt_r (long double __value, int __ndigit,
      int *__restrict __decpt, int *__restrict __sign,
      char *__restrict __buf, size_t __len)
     noexcept (true) __attribute__ ((__nonnull__ (3, 4, 5)));





extern int mblen (const char *__s, size_t __n) noexcept (true);


extern int mbtowc (wchar_t *__restrict __pwc,
     const char *__restrict __s, size_t __n) noexcept (true);


extern int wctomb (char *__s, wchar_t __wchar) noexcept (true);



extern size_t mbstowcs (wchar_t *__restrict __pwcs,
   const char *__restrict __s, size_t __n) noexcept (true)
    __attribute__ ((__access__ (__read_only__, 2)));

extern size_t wcstombs (char *__restrict __s,
   const wchar_t *__restrict __pwcs, size_t __n)
     noexcept (true)
 
  __attribute__ ((__access__ (__read_only__, 2)));






extern int rpmatch (const char *__response) noexcept (true) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));
# 1099 "/usr/include/stdlib.h" 3 4
extern int getsubopt (char **__restrict __optionp,
        char *const *__restrict __tokens,
        char **__restrict __valuep)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2, 3))) __attribute__ ((__warn_unused_result__));







extern int posix_openpt (int __oflag) __attribute__ ((__warn_unused_result__));







extern int grantpt (int __fd) noexcept (true);



extern int unlockpt (int __fd) noexcept (true);




extern char *ptsname (int __fd) noexcept (true) __attribute__ ((__warn_unused_result__));






extern int ptsname_r (int __fd, char *__buf, size_t __buflen)
     noexcept (true) __attribute__ ((__nonnull__ (2))) ;


extern int getpt (void);






extern int getloadavg (double __loadavg[], int __nelem)
     noexcept (true) __attribute__ ((__nonnull__ (1)));
# 1155 "/usr/include/stdlib.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/stdlib-float.h" 1 3 4
# 24 "/usr/include/x86_64-linux-gnu/bits/stdlib-float.h" 3 4
extern __inline __attribute__ ((__gnu_inline__)) double
__attribute__ ((__leaf__)) atof (const char *__nptr) noexcept (true)
{
  return strtod (__nptr, (char **) __null);
}
# 1156 "/usr/include/stdlib.h" 2 3 4



# 1 "/usr/include/x86_64-linux-gnu/bits/stdlib.h" 1 3 4
# 23 "/usr/include/x86_64-linux-gnu/bits/stdlib.h" 3 4
extern char *__realpath_chk (const char *__restrict __name,
        char *__restrict __resolved,
        size_t __resolvedlen) noexcept (true) __attribute__ ((__warn_unused_result__));
extern char *__realpath_alias (const char *__restrict __name, char *__restrict __resolved) noexcept (true) __asm__ ("" "realpath")

                                                 __attribute__ ((__warn_unused_result__));
extern char *__realpath_chk_warn (const char *__restrict __name, char *__restrict __resolved, size_t __resolvedlen) noexcept (true) __asm__ ("" "__realpath_chk")


                                                __attribute__ ((__warn_unused_result__))
     __attribute__((__warning__ ("second argument of realpath must be either NULL or at " "least PATH_MAX bytes long buffer")))
                                      ;

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) __attribute__ ((__warn_unused_result__)) char *
__attribute__ ((__leaf__)) realpath (const char *__restrict __name, char * __restrict __resolved) noexcept (true)







{
  size_t __sz = __builtin_dynamic_object_size (__resolved, 1);

  if (__sz == (size_t) -1)
    return __realpath_alias (__name, __resolved);





  return __realpath_chk (__name, __resolved, __sz);
}


extern int __ptsname_r_chk (int __fd, char *__buf, size_t __buflen,
       size_t __nreal) noexcept (true) __attribute__ ((__nonnull__ (2)))
    __attribute__ ((__access__ (__write_only__, 2, 3)));
extern int __ptsname_r_alias (int __fd, char *__buf, size_t __buflen) noexcept (true) __asm__ ("" "ptsname_r")

     __attribute__ ((__nonnull__ (2))) __attribute__ ((__access__ (__write_only__, 2, 3)));
extern int __ptsname_r_chk_warn (int __fd, char *__buf, size_t __buflen, size_t __nreal) noexcept (true) __asm__ ("" "__ptsname_r_chk")


     __attribute__ ((__nonnull__ (2))) __attribute__((__warning__ ("ptsname_r called with buflen bigger than " "size of buf")))
                   ;

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) int
__attribute__ ((__leaf__)) ptsname_r (int __fd, char * __buf, size_t __buflen) noexcept (true)


    


{
  return (((__builtin_constant_p (__builtin_dynamic_object_size (__buf, 1)) && (__builtin_dynamic_object_size (__buf, 1)) == (long unsigned int) -1) || (((__typeof (__buflen)) 0 < (__typeof (__buflen)) -1 || (__builtin_constant_p (__buflen) && (__buflen) > 0)) && __builtin_constant_p ((((long unsigned int) (__buflen)) <= ((__builtin_dynamic_object_size (__buf, 1))) / ((sizeof (char))))) && (((long unsigned int) (__buflen)) <= ((__builtin_dynamic_object_size (__buf, 1))) / ((sizeof (char)))))) ? __ptsname_r_alias (__fd, __buf, __buflen) : ((((__typeof (__buflen)) 0 < (__typeof (__buflen)) -1 || (__builtin_constant_p (__buflen) && (__buflen) > 0)) && __builtin_constant_p ((((long unsigned int) (__buflen)) <= (__builtin_dynamic_object_size (__buf, 1)) / (sizeof (char)))) && !(((long unsigned int) (__buflen)) <= (__builtin_dynamic_object_size (__buf, 1)) / (sizeof (char)))) ? __ptsname_r_chk_warn (__fd, __buf, __buflen, __builtin_dynamic_object_size (__buf, 1)) : __ptsname_r_chk (__fd, __buf, __buflen, __builtin_dynamic_object_size (__buf, 1))))

                           ;
}


extern int __wctomb_chk (char *__s, wchar_t __wchar, size_t __buflen)
  noexcept (true) __attribute__ ((__warn_unused_result__));
extern int __wctomb_alias (char *__s, wchar_t __wchar) noexcept (true) __asm__ ("" "wctomb")
              __attribute__ ((__warn_unused_result__));

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) __attribute__ ((__warn_unused_result__)) int
__attribute__ ((__leaf__)) wctomb (char * __s, wchar_t __wchar) noexcept (true)
{







  if (__builtin_dynamic_object_size (__s, 1) != (size_t) -1
      && 16 > __builtin_dynamic_object_size (__s, 1))
    return __wctomb_chk (__s, __wchar, __builtin_dynamic_object_size (__s, 1));
  return __wctomb_alias (__s, __wchar);
}


extern size_t __mbstowcs_chk (wchar_t *__restrict __dst,
         const char *__restrict __src,
         size_t __len, size_t __dstlen) noexcept (true)
    __attribute__ ((__access__ (__write_only__, 1, 3))) __attribute__ ((__access__ (__read_only__, 2)));
extern size_t __mbstowcs_nulldst (wchar_t *__restrict __dst, const char *__restrict __src, size_t __len) noexcept (true) __asm__ ("" "mbstowcs")



    __attribute__ ((__access__ (__read_only__, 2)));
extern size_t __mbstowcs_alias (wchar_t *__restrict __dst, const char *__restrict __src, size_t __len) noexcept (true) __asm__ ("" "mbstowcs")



    __attribute__ ((__access__ (__write_only__, 1, 3))) __attribute__ ((__access__ (__read_only__, 2)));
extern size_t __mbstowcs_chk_warn (wchar_t *__restrict __dst, const char *__restrict __src, size_t __len, size_t __dstlen) noexcept (true) __asm__ ("" "__mbstowcs_chk")



     __attribute__((__warning__ ("mbstowcs called with dst buffer smaller than len " "* sizeof (wchar_t)")))
                        ;

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) size_t
__attribute__ ((__leaf__)) mbstowcs (wchar_t * __restrict __dst, const char *__restrict __src, size_t __len) noexcept (true)


    


{
  if (__builtin_constant_p (__dst == __null) && __dst == __null)
    return __mbstowcs_nulldst (__dst, __src, __len);
  else
    return (((__builtin_constant_p (__builtin_dynamic_object_size (__dst, 1)) && (__builtin_dynamic_object_size (__dst, 1)) == (long unsigned int) -1) || (((__typeof (__len)) 0 < (__typeof (__len)) -1 || (__builtin_constant_p (__len) && (__len) > 0)) && __builtin_constant_p ((((long unsigned int) (__len)) <= ((__builtin_dynamic_object_size (__dst, 1))) / ((sizeof (wchar_t))))) && (((long unsigned int) (__len)) <= ((__builtin_dynamic_object_size (__dst, 1))) / ((sizeof (wchar_t)))))) ? __mbstowcs_alias (__dst, __src, __len) : ((((__typeof (__len)) 0 < (__typeof (__len)) -1 || (__builtin_constant_p (__len) && (__len) > 0)) && __builtin_constant_p ((((long unsigned int) (__len)) <= (__builtin_dynamic_object_size (__dst, 1)) / (sizeof (wchar_t)))) && !(((long unsigned int) (__len)) <= (__builtin_dynamic_object_size (__dst, 1)) / (sizeof (wchar_t)))) ? __mbstowcs_chk_warn (__dst, __src, __len, (__builtin_dynamic_object_size (__dst, 1)) / (sizeof (wchar_t))) : __mbstowcs_chk (__dst, __src, __len, (__builtin_dynamic_object_size (__dst, 1)) / (sizeof (wchar_t)))))
                                                      ;
}

extern size_t __wcstombs_chk (char *__restrict __dst,
         const wchar_t *__restrict __src,
         size_t __len, size_t __dstlen) noexcept (true)
  __attribute__ ((__access__ (__write_only__, 1, 3))) __attribute__ ((__access__ (__read_only__, 2)));
extern size_t __wcstombs_alias (char *__restrict __dst, const wchar_t *__restrict __src, size_t __len) noexcept (true) __asm__ ("" "wcstombs")



  __attribute__ ((__access__ (__write_only__, 1, 3))) __attribute__ ((__access__ (__read_only__, 2)));
extern size_t __wcstombs_chk_warn (char *__restrict __dst, const wchar_t *__restrict __src, size_t __len, size_t __dstlen) noexcept (true) __asm__ ("" "__wcstombs_chk")



     __attribute__((__warning__ ("wcstombs called with dst buffer smaller than len")));

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) size_t
__attribute__ ((__leaf__)) wcstombs (char * __restrict __dst, const wchar_t *__restrict __src, size_t __len) noexcept (true)


{
  return (((__builtin_constant_p (__builtin_dynamic_object_size (__dst, 1)) && (__builtin_dynamic_object_size (__dst, 1)) == (long unsigned int) -1) || (((__typeof (__len)) 0 < (__typeof (__len)) -1 || (__builtin_constant_p (__len) && (__len) > 0)) && __builtin_constant_p ((((long unsigned int) (__len)) <= ((__builtin_dynamic_object_size (__dst, 1))) / ((sizeof (char))))) && (((long unsigned int) (__len)) <= ((__builtin_dynamic_object_size (__dst, 1))) / ((sizeof (char)))))) ? __wcstombs_alias (__dst, __src, __len) : ((((__typeof (__len)) 0 < (__typeof (__len)) -1 || (__builtin_constant_p (__len) && (__len) > 0)) && __builtin_constant_p ((((long unsigned int) (__len)) <= (__builtin_dynamic_object_size (__dst, 1)) / (sizeof (char)))) && !(((long unsigned int) (__len)) <= (__builtin_dynamic_object_size (__dst, 1)) / (sizeof (char)))) ? __wcstombs_chk_warn (__dst, __src, __len, __builtin_dynamic_object_size (__dst, 1)) : __wcstombs_chk (__dst, __src, __len, __builtin_dynamic_object_size (__dst, 1))))

                         ;
}
# 1160 "/usr/include/stdlib.h" 2 3 4







}
# 80 "/usr/include/c++/14/cstdlib" 2 3

# 1 "/usr/include/c++/14/bits/std_abs.h" 1 3
# 33 "/usr/include/c++/14/bits/std_abs.h" 3
       
# 34 "/usr/include/c++/14/bits/std_abs.h" 3
# 46 "/usr/include/c++/14/bits/std_abs.h" 3
extern "C++"
{
namespace std __attribute__ ((__visibility__ ("default")))
{


  using ::abs;


  inline long
  abs(long __i) { return __builtin_labs(__i); }



  inline long long
  abs(long long __x) { return __builtin_llabs (__x); }
# 70 "/usr/include/c++/14/bits/std_abs.h" 3
  inline constexpr double
  abs(double __x)
  { return __builtin_fabs(__x); }

  inline constexpr float
  abs(float __x)
  { return __builtin_fabsf(__x); }

  inline constexpr long double
  abs(long double __x)
  { return __builtin_fabsl(__x); }



  __extension__ inline constexpr __int128
  abs(__int128 __x) { return __x >= 0 ? __x : -__x; }
# 135 "/usr/include/c++/14/bits/std_abs.h" 3
  __extension__ inline constexpr
  __float128
  abs(__float128 __x)
  {



    return __builtin_fabsf128(__x);




  }



}
}
# 82 "/usr/include/c++/14/cstdlib" 2 3
# 125 "/usr/include/c++/14/cstdlib" 3
extern "C++"
{
namespace std __attribute__ ((__visibility__ ("default")))
{


  using ::div_t;
  using ::ldiv_t;

  using ::abort;

  using ::aligned_alloc;

  using ::atexit;


  using ::at_quick_exit;


  using ::atof;
  using ::atoi;
  using ::atol;
  using ::bsearch;
  using ::calloc;
  using ::div;
  using ::exit;
  using ::free;
  using ::getenv;
  using ::labs;
  using ::ldiv;
  using ::malloc;

  using ::mblen;
  using ::mbstowcs;
  using ::mbtowc;

  using ::qsort;


  using ::quick_exit;


  using ::rand;
  using ::realloc;
  using ::srand;
  using ::strtod;
  using ::strtol;
  using ::strtoul;
  using ::system;

  using ::wcstombs;
  using ::wctomb;



  inline ldiv_t
  div(long __i, long __j) noexcept { return ldiv(__i, __j); }




}
# 199 "/usr/include/c++/14/cstdlib" 3
namespace __gnu_cxx __attribute__ ((__visibility__ ("default")))
{



  using ::lldiv_t;





  using ::_Exit;



#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wlong-long"
  using ::llabs;

  inline lldiv_t
  div(long long __n, long long __d)
  { lldiv_t __q; __q.quot = __n / __d; __q.rem = __n % __d; return __q; }

  using ::lldiv;
#pragma GCC diagnostic pop
# 234 "/usr/include/c++/14/cstdlib" 3
  using ::atoll;
  using ::strtoll;
  using ::strtoull;

  using ::strtof;
  using ::strtold;


}

namespace std
{

  using ::__gnu_cxx::lldiv_t;

  using ::__gnu_cxx::_Exit;

  using ::__gnu_cxx::llabs;
  using ::__gnu_cxx::div;
  using ::__gnu_cxx::lldiv;

  using ::__gnu_cxx::atoll;
  using ::__gnu_cxx::strtof;
  using ::__gnu_cxx::strtoll;
  using ::__gnu_cxx::strtoull;
  using ::__gnu_cxx::strtold;
}
# 278 "/usr/include/c++/14/cstdlib" 3
}
# 37 "/usr/include/c++/14/stdlib.h" 2 3

using std::abort;
using std::atexit;
using std::exit;


  using std::at_quick_exit;


  using std::quick_exit;


  using std::_Exit;




using std::div_t;
using std::ldiv_t;

using std::abs;
using std::atof;
using std::atoi;
using std::atol;
using std::bsearch;
using std::calloc;
using std::div;
using std::free;
using std::getenv;
using std::labs;
using std::ldiv;
using std::malloc;

using std::mblen;
using std::mbstowcs;
using std::mbtowc;

using std::qsort;
using std::rand;
using std::realloc;
using std::srand;
using std::strtod;
using std::strtol;
using std::strtoul;
using std::system;

using std::wcstombs;
using std::wctomb;
# 48 "../../bochs.h" 2
# 65 "../../bochs.h"
# 1 "/usr/include/assert.h" 1 3 4
# 66 "/usr/include/assert.h" 3 4
extern "C" {


extern void __assert_fail (const char *__assertion, const char *__file,
      unsigned int __line, const char *__function)
     noexcept (true) __attribute__ ((__noreturn__)) __attribute__ ((__cold__));


extern void __assert_perror_fail (int __errnum, const char *__file,
      unsigned int __line, const char *__function)
     noexcept (true) __attribute__ ((__noreturn__)) __attribute__ ((__cold__));




extern void __assert (const char *__assertion, const char *__file, int __line)
     noexcept (true) __attribute__ ((__noreturn__)) __attribute__ ((__cold__));


}
# 66 "../../bochs.h" 2
# 1 "/usr/include/errno.h" 1 3 4
# 28 "/usr/include/errno.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/errno.h" 1 3 4
# 26 "/usr/include/x86_64-linux-gnu/bits/errno.h" 3 4
# 1 "/usr/include/linux/errno.h" 1 3 4
# 1 "/usr/include/x86_64-linux-gnu/asm/errno.h" 1 3 4
# 1 "/usr/include/asm-generic/errno.h" 1 3 4




# 1 "/usr/include/asm-generic/errno-base.h" 1 3 4
# 6 "/usr/include/asm-generic/errno.h" 2 3 4
# 2 "/usr/include/x86_64-linux-gnu/asm/errno.h" 2 3 4
# 2 "/usr/include/linux/errno.h" 2 3 4
# 27 "/usr/include/x86_64-linux-gnu/bits/errno.h" 2 3 4
# 29 "/usr/include/errno.h" 2 3 4





extern "C" {


extern int *__errno_location (void) noexcept (true) __attribute__ ((__const__));







extern char *program_invocation_name;
extern char *program_invocation_short_name;

# 1 "/usr/include/x86_64-linux-gnu/bits/types/error_t.h" 1 3 4
# 22 "/usr/include/x86_64-linux-gnu/bits/types/error_t.h" 3 4
typedef int error_t;
# 49 "/usr/include/errno.h" 2 3 4



}
# 67 "../../bochs.h" 2


# 1 "/usr/include/unistd.h" 1 3 4
# 27 "/usr/include/unistd.h" 3 4
extern "C" {
# 202 "/usr/include/unistd.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/posix_opt.h" 1 3 4
# 203 "/usr/include/unistd.h" 2 3 4



# 1 "/usr/include/x86_64-linux-gnu/bits/environments.h" 1 3 4
# 22 "/usr/include/x86_64-linux-gnu/bits/environments.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/wordsize.h" 1 3 4
# 23 "/usr/include/x86_64-linux-gnu/bits/environments.h" 2 3 4
# 207 "/usr/include/unistd.h" 2 3 4
# 226 "/usr/include/unistd.h" 3 4
# 1 "/usr/lib/gcc/x86_64-linux-gnu/14/include/stddef.h" 1 3 4
# 227 "/usr/include/unistd.h" 2 3 4
# 267 "/usr/include/unistd.h" 3 4
typedef __intptr_t intptr_t;






typedef __socklen_t socklen_t;
# 287 "/usr/include/unistd.h" 3 4
extern int access (const char *__name, int __type) noexcept (true) __attribute__ ((__nonnull__ (1)));




extern int euidaccess (const char *__name, int __type)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int eaccess (const char *__name, int __type)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int execveat (int __fd, const char *__path, char *const __argv[],
                     char *const __envp[], int __flags)
    noexcept (true) __attribute__ ((__nonnull__ (2, 3)));






extern int faccessat (int __fd, const char *__file, int __type, int __flag)
     noexcept (true) __attribute__ ((__nonnull__ (2))) __attribute__ ((__warn_unused_result__));
# 342 "/usr/include/unistd.h" 3 4
extern __off64_t lseek (int __fd, __off64_t __offset, int __whence) noexcept (true) __asm__ ("" "lseek64")

             ;





extern __off64_t lseek64 (int __fd, __off64_t __offset, int __whence)
     noexcept (true);






extern int close (int __fd);




extern void closefrom (int __lowfd) noexcept (true);







extern ssize_t read (int __fd, void *__buf, size_t __nbytes) __attribute__ ((__warn_unused_result__))
    ;





extern ssize_t write (int __fd, const void *__buf, size_t __n) __attribute__ ((__warn_unused_result__))
    __attribute__ ((__access__ (__read_only__, 2, 3)));
# 404 "/usr/include/unistd.h" 3 4
extern ssize_t pread (int __fd, void *__buf, size_t __nbytes, __off64_t __offset) __asm__ ("" "pread64")

               __attribute__ ((__warn_unused_result__))
    ;
extern ssize_t pwrite (int __fd, const void *__buf, size_t __nbytes, __off64_t __offset) __asm__ ("" "pwrite64")

                __attribute__ ((__warn_unused_result__))
    __attribute__ ((__access__ (__read_only__, 2, 3)));
# 422 "/usr/include/unistd.h" 3 4
extern ssize_t pread64 (int __fd, void *__buf, size_t __nbytes,
   __off64_t __offset) __attribute__ ((__warn_unused_result__))
    ;


extern ssize_t pwrite64 (int __fd, const void *__buf, size_t __n,
    __off64_t __offset) __attribute__ ((__warn_unused_result__))
    __attribute__ ((__access__ (__read_only__, 2, 3)));







extern int pipe (int __pipedes[2]) noexcept (true) __attribute__ ((__warn_unused_result__));




extern int pipe2 (int __pipedes[2], int __flags) noexcept (true) __attribute__ ((__warn_unused_result__));
# 452 "/usr/include/unistd.h" 3 4
extern unsigned int alarm (unsigned int __seconds) noexcept (true);
# 464 "/usr/include/unistd.h" 3 4
extern unsigned int sleep (unsigned int __seconds);







extern __useconds_t ualarm (__useconds_t __value, __useconds_t __interval)
     noexcept (true);






extern int usleep (__useconds_t __useconds);
# 489 "/usr/include/unistd.h" 3 4
extern int pause (void);



extern int chown (const char *__file, __uid_t __owner, __gid_t __group)
     noexcept (true) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));



extern int fchown (int __fd, __uid_t __owner, __gid_t __group) noexcept (true) __attribute__ ((__warn_unused_result__));




extern int lchown (const char *__file, __uid_t __owner, __gid_t __group)
     noexcept (true) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));






extern int fchownat (int __fd, const char *__file, __uid_t __owner,
       __gid_t __group, int __flag)
     noexcept (true) __attribute__ ((__nonnull__ (2))) __attribute__ ((__warn_unused_result__));



extern int chdir (const char *__path) noexcept (true) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));



extern int fchdir (int __fd) noexcept (true) __attribute__ ((__warn_unused_result__));
# 531 "/usr/include/unistd.h" 3 4
extern char *getcwd (char *__buf, size_t __size) noexcept (true) __attribute__ ((__warn_unused_result__));





extern char *get_current_dir_name (void) noexcept (true);







extern char *getwd (char *__buf)
     noexcept (true) __attribute__ ((__nonnull__ (1))) __attribute__ ((__deprecated__)) __attribute__ ((__warn_unused_result__))
    __attribute__ ((__access__ (__write_only__, 1)));




extern int dup (int __fd) noexcept (true) __attribute__ ((__warn_unused_result__));


extern int dup2 (int __fd, int __fd2) noexcept (true);




extern int dup3 (int __fd, int __fd2, int __flags) noexcept (true);



extern char **__environ;

extern char **environ;





extern int execve (const char *__path, char *const __argv[],
     char *const __envp[]) noexcept (true) __attribute__ ((__nonnull__ (1, 2)));




extern int fexecve (int __fd, char *const __argv[], char *const __envp[])
     noexcept (true) __attribute__ ((__nonnull__ (2)));




extern int execv (const char *__path, char *const __argv[])
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));



extern int execle (const char *__path, const char *__arg, ...)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));



extern int execl (const char *__path, const char *__arg, ...)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));



extern int execvp (const char *__file, char *const __argv[])
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));




extern int execlp (const char *__file, const char *__arg, ...)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));




extern int execvpe (const char *__file, char *const __argv[],
      char *const __envp[])
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));





extern int nice (int __inc) noexcept (true) __attribute__ ((__warn_unused_result__));




extern void _exit (int __status) __attribute__ ((__noreturn__));





# 1 "/usr/include/x86_64-linux-gnu/bits/confname.h" 1 3 4
# 24 "/usr/include/x86_64-linux-gnu/bits/confname.h" 3 4
enum
  {
    _PC_LINK_MAX,

    _PC_MAX_CANON,

    _PC_MAX_INPUT,

    _PC_NAME_MAX,

    _PC_PATH_MAX,

    _PC_PIPE_BUF,

    _PC_CHOWN_RESTRICTED,

    _PC_NO_TRUNC,

    _PC_VDISABLE,

    _PC_SYNC_IO,

    _PC_ASYNC_IO,

    _PC_PRIO_IO,

    _PC_SOCK_MAXBUF,

    _PC_FILESIZEBITS,

    _PC_REC_INCR_XFER_SIZE,

    _PC_REC_MAX_XFER_SIZE,

    _PC_REC_MIN_XFER_SIZE,

    _PC_REC_XFER_ALIGN,

    _PC_ALLOC_SIZE_MIN,

    _PC_SYMLINK_MAX,

    _PC_2_SYMLINKS

  };


enum
  {
    _SC_ARG_MAX,

    _SC_CHILD_MAX,

    _SC_CLK_TCK,

    _SC_NGROUPS_MAX,

    _SC_OPEN_MAX,

    _SC_STREAM_MAX,

    _SC_TZNAME_MAX,

    _SC_JOB_CONTROL,

    _SC_SAVED_IDS,

    _SC_REALTIME_SIGNALS,

    _SC_PRIORITY_SCHEDULING,

    _SC_TIMERS,

    _SC_ASYNCHRONOUS_IO,

    _SC_PRIORITIZED_IO,

    _SC_SYNCHRONIZED_IO,

    _SC_FSYNC,

    _SC_MAPPED_FILES,

    _SC_MEMLOCK,

    _SC_MEMLOCK_RANGE,

    _SC_MEMORY_PROTECTION,

    _SC_MESSAGE_PASSING,

    _SC_SEMAPHORES,

    _SC_SHARED_MEMORY_OBJECTS,

    _SC_AIO_LISTIO_MAX,

    _SC_AIO_MAX,

    _SC_AIO_PRIO_DELTA_MAX,

    _SC_DELAYTIMER_MAX,

    _SC_MQ_OPEN_MAX,

    _SC_MQ_PRIO_MAX,

    _SC_VERSION,

    _SC_PAGESIZE,


    _SC_RTSIG_MAX,

    _SC_SEM_NSEMS_MAX,

    _SC_SEM_VALUE_MAX,

    _SC_SIGQUEUE_MAX,

    _SC_TIMER_MAX,




    _SC_BC_BASE_MAX,

    _SC_BC_DIM_MAX,

    _SC_BC_SCALE_MAX,

    _SC_BC_STRING_MAX,

    _SC_COLL_WEIGHTS_MAX,

    _SC_EQUIV_CLASS_MAX,

    _SC_EXPR_NEST_MAX,

    _SC_LINE_MAX,

    _SC_RE_DUP_MAX,

    _SC_CHARCLASS_NAME_MAX,


    _SC_2_VERSION,

    _SC_2_C_BIND,

    _SC_2_C_DEV,

    _SC_2_FORT_DEV,

    _SC_2_FORT_RUN,

    _SC_2_SW_DEV,

    _SC_2_LOCALEDEF,


    _SC_PII,

    _SC_PII_XTI,

    _SC_PII_SOCKET,

    _SC_PII_INTERNET,

    _SC_PII_OSI,

    _SC_POLL,

    _SC_SELECT,

    _SC_UIO_MAXIOV,

    _SC_IOV_MAX = _SC_UIO_MAXIOV,

    _SC_PII_INTERNET_STREAM,

    _SC_PII_INTERNET_DGRAM,

    _SC_PII_OSI_COTS,

    _SC_PII_OSI_CLTS,

    _SC_PII_OSI_M,

    _SC_T_IOV_MAX,



    _SC_THREADS,

    _SC_THREAD_SAFE_FUNCTIONS,

    _SC_GETGR_R_SIZE_MAX,

    _SC_GETPW_R_SIZE_MAX,

    _SC_LOGIN_NAME_MAX,

    _SC_TTY_NAME_MAX,

    _SC_THREAD_DESTRUCTOR_ITERATIONS,

    _SC_THREAD_KEYS_MAX,

    _SC_THREAD_STACK_MIN,

    _SC_THREAD_THREADS_MAX,

    _SC_THREAD_ATTR_STACKADDR,

    _SC_THREAD_ATTR_STACKSIZE,

    _SC_THREAD_PRIORITY_SCHEDULING,

    _SC_THREAD_PRIO_INHERIT,

    _SC_THREAD_PRIO_PROTECT,

    _SC_THREAD_PROCESS_SHARED,


    _SC_NPROCESSORS_CONF,

    _SC_NPROCESSORS_ONLN,

    _SC_PHYS_PAGES,

    _SC_AVPHYS_PAGES,

    _SC_ATEXIT_MAX,

    _SC_PASS_MAX,


    _SC_XOPEN_VERSION,

    _SC_XOPEN_XCU_VERSION,

    _SC_XOPEN_UNIX,

    _SC_XOPEN_CRYPT,

    _SC_XOPEN_ENH_I18N,

    _SC_XOPEN_SHM,


    _SC_2_CHAR_TERM,

    _SC_2_C_VERSION,

    _SC_2_UPE,


    _SC_XOPEN_XPG2,

    _SC_XOPEN_XPG3,

    _SC_XOPEN_XPG4,


    _SC_CHAR_BIT,

    _SC_CHAR_MAX,

    _SC_CHAR_MIN,

    _SC_INT_MAX,

    _SC_INT_MIN,

    _SC_LONG_BIT,

    _SC_WORD_BIT,

    _SC_MB_LEN_MAX,

    _SC_NZERO,

    _SC_SSIZE_MAX,

    _SC_SCHAR_MAX,

    _SC_SCHAR_MIN,

    _SC_SHRT_MAX,

    _SC_SHRT_MIN,

    _SC_UCHAR_MAX,

    _SC_UINT_MAX,

    _SC_ULONG_MAX,

    _SC_USHRT_MAX,


    _SC_NL_ARGMAX,

    _SC_NL_LANGMAX,

    _SC_NL_MSGMAX,

    _SC_NL_NMAX,

    _SC_NL_SETMAX,

    _SC_NL_TEXTMAX,


    _SC_XBS5_ILP32_OFF32,

    _SC_XBS5_ILP32_OFFBIG,

    _SC_XBS5_LP64_OFF64,

    _SC_XBS5_LPBIG_OFFBIG,


    _SC_XOPEN_LEGACY,

    _SC_XOPEN_REALTIME,

    _SC_XOPEN_REALTIME_THREADS,


    _SC_ADVISORY_INFO,

    _SC_BARRIERS,

    _SC_BASE,

    _SC_C_LANG_SUPPORT,

    _SC_C_LANG_SUPPORT_R,

    _SC_CLOCK_SELECTION,

    _SC_CPUTIME,

    _SC_THREAD_CPUTIME,

    _SC_DEVICE_IO,

    _SC_DEVICE_SPECIFIC,

    _SC_DEVICE_SPECIFIC_R,

    _SC_FD_MGMT,

    _SC_FIFO,

    _SC_PIPE,

    _SC_FILE_ATTRIBUTES,

    _SC_FILE_LOCKING,

    _SC_FILE_SYSTEM,

    _SC_MONOTONIC_CLOCK,

    _SC_MULTI_PROCESS,

    _SC_SINGLE_PROCESS,

    _SC_NETWORKING,

    _SC_READER_WRITER_LOCKS,

    _SC_SPIN_LOCKS,

    _SC_REGEXP,

    _SC_REGEX_VERSION,

    _SC_SHELL,

    _SC_SIGNALS,

    _SC_SPAWN,

    _SC_SPORADIC_SERVER,

    _SC_THREAD_SPORADIC_SERVER,

    _SC_SYSTEM_DATABASE,

    _SC_SYSTEM_DATABASE_R,

    _SC_TIMEOUTS,

    _SC_TYPED_MEMORY_OBJECTS,

    _SC_USER_GROUPS,

    _SC_USER_GROUPS_R,

    _SC_2_PBS,

    _SC_2_PBS_ACCOUNTING,

    _SC_2_PBS_LOCATE,

    _SC_2_PBS_MESSAGE,

    _SC_2_PBS_TRACK,

    _SC_SYMLOOP_MAX,

    _SC_STREAMS,

    _SC_2_PBS_CHECKPOINT,


    _SC_V6_ILP32_OFF32,

    _SC_V6_ILP32_OFFBIG,

    _SC_V6_LP64_OFF64,

    _SC_V6_LPBIG_OFFBIG,


    _SC_HOST_NAME_MAX,

    _SC_TRACE,

    _SC_TRACE_EVENT_FILTER,

    _SC_TRACE_INHERIT,

    _SC_TRACE_LOG,


    _SC_LEVEL1_ICACHE_SIZE,

    _SC_LEVEL1_ICACHE_ASSOC,

    _SC_LEVEL1_ICACHE_LINESIZE,

    _SC_LEVEL1_DCACHE_SIZE,

    _SC_LEVEL1_DCACHE_ASSOC,

    _SC_LEVEL1_DCACHE_LINESIZE,

    _SC_LEVEL2_CACHE_SIZE,

    _SC_LEVEL2_CACHE_ASSOC,

    _SC_LEVEL2_CACHE_LINESIZE,

    _SC_LEVEL3_CACHE_SIZE,

    _SC_LEVEL3_CACHE_ASSOC,

    _SC_LEVEL3_CACHE_LINESIZE,

    _SC_LEVEL4_CACHE_SIZE,

    _SC_LEVEL4_CACHE_ASSOC,

    _SC_LEVEL4_CACHE_LINESIZE,



    _SC_IPV6 = _SC_LEVEL1_ICACHE_SIZE + 50,

    _SC_RAW_SOCKETS,


    _SC_V7_ILP32_OFF32,

    _SC_V7_ILP32_OFFBIG,

    _SC_V7_LP64_OFF64,

    _SC_V7_LPBIG_OFFBIG,


    _SC_SS_REPL_MAX,


    _SC_TRACE_EVENT_NAME_MAX,

    _SC_TRACE_NAME_MAX,

    _SC_TRACE_SYS_MAX,

    _SC_TRACE_USER_EVENT_MAX,


    _SC_XOPEN_STREAMS,


    _SC_THREAD_ROBUST_PRIO_INHERIT,

    _SC_THREAD_ROBUST_PRIO_PROTECT,


    _SC_MINSIGSTKSZ,


    _SC_SIGSTKSZ

  };


enum
  {
    _CS_PATH,


    _CS_V6_WIDTH_RESTRICTED_ENVS,



    _CS_GNU_LIBC_VERSION,

    _CS_GNU_LIBPTHREAD_VERSION,


    _CS_V5_WIDTH_RESTRICTED_ENVS,



    _CS_V7_WIDTH_RESTRICTED_ENVS,



    _CS_LFS_CFLAGS = 1000,

    _CS_LFS_LDFLAGS,

    _CS_LFS_LIBS,

    _CS_LFS_LINTFLAGS,

    _CS_LFS64_CFLAGS,

    _CS_LFS64_LDFLAGS,

    _CS_LFS64_LIBS,

    _CS_LFS64_LINTFLAGS,


    _CS_XBS5_ILP32_OFF32_CFLAGS = 1100,

    _CS_XBS5_ILP32_OFF32_LDFLAGS,

    _CS_XBS5_ILP32_OFF32_LIBS,

    _CS_XBS5_ILP32_OFF32_LINTFLAGS,

    _CS_XBS5_ILP32_OFFBIG_CFLAGS,

    _CS_XBS5_ILP32_OFFBIG_LDFLAGS,

    _CS_XBS5_ILP32_OFFBIG_LIBS,

    _CS_XBS5_ILP32_OFFBIG_LINTFLAGS,

    _CS_XBS5_LP64_OFF64_CFLAGS,

    _CS_XBS5_LP64_OFF64_LDFLAGS,

    _CS_XBS5_LP64_OFF64_LIBS,

    _CS_XBS5_LP64_OFF64_LINTFLAGS,

    _CS_XBS5_LPBIG_OFFBIG_CFLAGS,

    _CS_XBS5_LPBIG_OFFBIG_LDFLAGS,

    _CS_XBS5_LPBIG_OFFBIG_LIBS,

    _CS_XBS5_LPBIG_OFFBIG_LINTFLAGS,


    _CS_POSIX_V6_ILP32_OFF32_CFLAGS,

    _CS_POSIX_V6_ILP32_OFF32_LDFLAGS,

    _CS_POSIX_V6_ILP32_OFF32_LIBS,

    _CS_POSIX_V6_ILP32_OFF32_LINTFLAGS,

    _CS_POSIX_V6_ILP32_OFFBIG_CFLAGS,

    _CS_POSIX_V6_ILP32_OFFBIG_LDFLAGS,

    _CS_POSIX_V6_ILP32_OFFBIG_LIBS,

    _CS_POSIX_V6_ILP32_OFFBIG_LINTFLAGS,

    _CS_POSIX_V6_LP64_OFF64_CFLAGS,

    _CS_POSIX_V6_LP64_OFF64_LDFLAGS,

    _CS_POSIX_V6_LP64_OFF64_LIBS,

    _CS_POSIX_V6_LP64_OFF64_LINTFLAGS,

    _CS_POSIX_V6_LPBIG_OFFBIG_CFLAGS,

    _CS_POSIX_V6_LPBIG_OFFBIG_LDFLAGS,

    _CS_POSIX_V6_LPBIG_OFFBIG_LIBS,

    _CS_POSIX_V6_LPBIG_OFFBIG_LINTFLAGS,


    _CS_POSIX_V7_ILP32_OFF32_CFLAGS,

    _CS_POSIX_V7_ILP32_OFF32_LDFLAGS,

    _CS_POSIX_V7_ILP32_OFF32_LIBS,

    _CS_POSIX_V7_ILP32_OFF32_LINTFLAGS,

    _CS_POSIX_V7_ILP32_OFFBIG_CFLAGS,

    _CS_POSIX_V7_ILP32_OFFBIG_LDFLAGS,

    _CS_POSIX_V7_ILP32_OFFBIG_LIBS,

    _CS_POSIX_V7_ILP32_OFFBIG_LINTFLAGS,

    _CS_POSIX_V7_LP64_OFF64_CFLAGS,

    _CS_POSIX_V7_LP64_OFF64_LDFLAGS,

    _CS_POSIX_V7_LP64_OFF64_LIBS,

    _CS_POSIX_V7_LP64_OFF64_LINTFLAGS,

    _CS_POSIX_V7_LPBIG_OFFBIG_CFLAGS,

    _CS_POSIX_V7_LPBIG_OFFBIG_LDFLAGS,

    _CS_POSIX_V7_LPBIG_OFFBIG_LIBS,

    _CS_POSIX_V7_LPBIG_OFFBIG_LINTFLAGS,


    _CS_V6_ENV,

    _CS_V7_ENV

  };
# 631 "/usr/include/unistd.h" 2 3 4


extern long int pathconf (const char *__path, int __name)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern long int fpathconf (int __fd, int __name) noexcept (true);


extern long int sysconf (int __name) noexcept (true);



extern size_t confstr (int __name, char *__buf, size_t __len) noexcept (true)
    ;




extern __pid_t getpid (void) noexcept (true);


extern __pid_t getppid (void) noexcept (true);


extern __pid_t getpgrp (void) noexcept (true);


extern __pid_t __getpgid (__pid_t __pid) noexcept (true);

extern __pid_t getpgid (__pid_t __pid) noexcept (true);






extern int setpgid (__pid_t __pid, __pid_t __pgid) noexcept (true);
# 682 "/usr/include/unistd.h" 3 4
extern int setpgrp (void) noexcept (true);






extern __pid_t setsid (void) noexcept (true);



extern __pid_t getsid (__pid_t __pid) noexcept (true);



extern __uid_t getuid (void) noexcept (true);


extern __uid_t geteuid (void) noexcept (true);


extern __gid_t getgid (void) noexcept (true);


extern __gid_t getegid (void) noexcept (true);




extern int getgroups (int __size, __gid_t __list[]) noexcept (true) __attribute__ ((__warn_unused_result__))
    ;


extern int group_member (__gid_t __gid) noexcept (true);






extern int setuid (__uid_t __uid) noexcept (true) __attribute__ ((__warn_unused_result__));




extern int setreuid (__uid_t __ruid, __uid_t __euid) noexcept (true) __attribute__ ((__warn_unused_result__));




extern int seteuid (__uid_t __uid) noexcept (true) __attribute__ ((__warn_unused_result__));






extern int setgid (__gid_t __gid) noexcept (true) __attribute__ ((__warn_unused_result__));




extern int setregid (__gid_t __rgid, __gid_t __egid) noexcept (true) __attribute__ ((__warn_unused_result__));




extern int setegid (__gid_t __gid) noexcept (true) __attribute__ ((__warn_unused_result__));





extern int getresuid (__uid_t *__ruid, __uid_t *__euid, __uid_t *__suid)
     noexcept (true);



extern int getresgid (__gid_t *__rgid, __gid_t *__egid, __gid_t *__sgid)
     noexcept (true);



extern int setresuid (__uid_t __ruid, __uid_t __euid, __uid_t __suid)
     noexcept (true) __attribute__ ((__warn_unused_result__));



extern int setresgid (__gid_t __rgid, __gid_t __egid, __gid_t __sgid)
     noexcept (true) __attribute__ ((__warn_unused_result__));






extern __pid_t fork (void) noexcept (true);







extern __pid_t vfork (void) noexcept (true);






extern __pid_t _Fork (void) noexcept (true);





extern char *ttyname (int __fd) noexcept (true);



extern int ttyname_r (int __fd, char *__buf, size_t __buflen)
     noexcept (true) __attribute__ ((__nonnull__ (2))) __attribute__ ((__warn_unused_result__))
     ;



extern int isatty (int __fd) noexcept (true);




extern int ttyslot (void) noexcept (true);




extern int link (const char *__from, const char *__to)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2))) __attribute__ ((__warn_unused_result__));




extern int linkat (int __fromfd, const char *__from, int __tofd,
     const char *__to, int __flags)
     noexcept (true) __attribute__ ((__nonnull__ (2, 4))) __attribute__ ((__warn_unused_result__));




extern int symlink (const char *__from, const char *__to)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2))) __attribute__ ((__warn_unused_result__));




extern ssize_t readlink (const char *__restrict __path,
    char *__restrict __buf, size_t __len)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2))) __attribute__ ((__warn_unused_result__))
     ;





extern int symlinkat (const char *__from, int __tofd,
        const char *__to) noexcept (true) __attribute__ ((__nonnull__ (1, 3))) __attribute__ ((__warn_unused_result__));


extern ssize_t readlinkat (int __fd, const char *__restrict __path,
      char *__restrict __buf, size_t __len)
     noexcept (true) __attribute__ ((__nonnull__ (2, 3))) __attribute__ ((__warn_unused_result__))
     ;



extern int unlink (const char *__name) noexcept (true) __attribute__ ((__nonnull__ (1)));



extern int unlinkat (int __fd, const char *__name, int __flag)
     noexcept (true) __attribute__ ((__nonnull__ (2)));



extern int rmdir (const char *__path) noexcept (true) __attribute__ ((__nonnull__ (1)));



extern __pid_t tcgetpgrp (int __fd) noexcept (true);


extern int tcsetpgrp (int __fd, __pid_t __pgrp_id) noexcept (true);






extern char *getlogin (void);







extern int getlogin_r (char *__name, size_t __name_len) __attribute__ ((__nonnull__ (1)))
    ;




extern int setlogin (const char *__name) noexcept (true) __attribute__ ((__nonnull__ (1)));







# 1 "/usr/include/x86_64-linux-gnu/bits/getopt_posix.h" 1 3 4
# 27 "/usr/include/x86_64-linux-gnu/bits/getopt_posix.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/getopt_core.h" 1 3 4
# 28 "/usr/include/x86_64-linux-gnu/bits/getopt_core.h" 3 4
extern "C" {







extern char *optarg;
# 50 "/usr/include/x86_64-linux-gnu/bits/getopt_core.h" 3 4
extern int optind;




extern int opterr;



extern int optopt;
# 91 "/usr/include/x86_64-linux-gnu/bits/getopt_core.h" 3 4
extern int getopt (int ___argc, char *const *___argv, const char *__shortopts)
       noexcept (true) __attribute__ ((__nonnull__ (2, 3)));

}
# 28 "/usr/include/x86_64-linux-gnu/bits/getopt_posix.h" 2 3 4

extern "C" {
# 49 "/usr/include/x86_64-linux-gnu/bits/getopt_posix.h" 3 4
}
# 904 "/usr/include/unistd.h" 2 3 4







extern int gethostname (char *__name, size_t __len) noexcept (true) __attribute__ ((__nonnull__ (1)))
    ;






extern int sethostname (const char *__name, size_t __len)
     noexcept (true) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__)) __attribute__ ((__access__ (__read_only__, 1, 2)));



extern int sethostid (long int __id) noexcept (true) __attribute__ ((__warn_unused_result__));





extern int getdomainname (char *__name, size_t __len)
     noexcept (true) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__))
     ;
extern int setdomainname (const char *__name, size_t __len)
     noexcept (true) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__)) __attribute__ ((__access__ (__read_only__, 1, 2)));




extern int vhangup (void) noexcept (true);


extern int revoke (const char *__file) noexcept (true) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));







extern int profil (unsigned short int *__sample_buffer, size_t __size,
     size_t __offset, unsigned int __scale)
     noexcept (true) __attribute__ ((__nonnull__ (1)));





extern int acct (const char *__name) noexcept (true);



extern char *getusershell (void) noexcept (true);
extern void endusershell (void) noexcept (true);
extern void setusershell (void) noexcept (true);





extern int daemon (int __nochdir, int __noclose) noexcept (true) __attribute__ ((__warn_unused_result__));






extern int chroot (const char *__path) noexcept (true) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));



extern char *getpass (const char *__prompt) __attribute__ ((__nonnull__ (1)));







extern int fsync (int __fd);





extern int syncfs (int __fd) noexcept (true);






extern long int gethostid (void);


extern void sync (void) noexcept (true);





extern int getpagesize (void) noexcept (true) __attribute__ ((__const__));




extern int getdtablesize (void) noexcept (true);
# 1030 "/usr/include/unistd.h" 3 4
extern int truncate (const char *__file, __off64_t __length) noexcept (true) __asm__ ("" "truncate64")

                  __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));





extern int truncate64 (const char *__file, __off64_t __length)
     noexcept (true) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));
# 1052 "/usr/include/unistd.h" 3 4
extern int ftruncate (int __fd, __off64_t __length) noexcept (true) __asm__ ("" "ftruncate64")
                   __attribute__ ((__warn_unused_result__));





extern int ftruncate64 (int __fd, __off64_t __length) noexcept (true) __attribute__ ((__warn_unused_result__));
# 1070 "/usr/include/unistd.h" 3 4
extern int brk (void *__addr) noexcept (true) __attribute__ ((__warn_unused_result__));





extern void *sbrk (intptr_t __delta) noexcept (true);
# 1091 "/usr/include/unistd.h" 3 4
extern long int syscall (long int __sysno, ...) noexcept (true);
# 1117 "/usr/include/unistd.h" 3 4
extern int lockf (int __fd, int __cmd, __off64_t __len) __asm__ ("" "lockf64")
                  __attribute__ ((__warn_unused_result__));





extern int lockf64 (int __fd, int __cmd, __off64_t __len) __attribute__ ((__warn_unused_result__));
# 1142 "/usr/include/unistd.h" 3 4
ssize_t copy_file_range (int __infd, __off64_t *__pinoff,
    int __outfd, __off64_t *__poutoff,
    size_t __length, unsigned int __flags);





extern int fdatasync (int __fildes);
# 1162 "/usr/include/unistd.h" 3 4
extern char *crypt (const char *__key, const char *__salt)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));







extern void swab (const void *__restrict __from, void *__restrict __to,
    ssize_t __n) noexcept (true) __attribute__ ((__nonnull__ (1, 2)))
    __attribute__ ((__access__ (__read_only__, 1, 3)))
    __attribute__ ((__access__ (__write_only__, 2, 3)));
# 1201 "/usr/include/unistd.h" 3 4
int getentropy (void *__buffer, size_t __length) __attribute__ ((__warn_unused_result__))
    __attribute__ ((__access__ (__write_only__, 1, 2)));
# 1211 "/usr/include/unistd.h" 3 4
extern int close_range (unsigned int __fd, unsigned int __max_fd,
   int __flags) noexcept (true);




# 1 "/usr/include/x86_64-linux-gnu/bits/unistd.h" 1 3 4
# 23 "/usr/include/x86_64-linux-gnu/bits/unistd.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/unistd-decl.h" 1 3 4
# 26 "/usr/include/x86_64-linux-gnu/bits/unistd-decl.h" 3 4
extern ssize_t __read_chk (int __fd, void *__buf, size_t __nbytes,
      size_t __buflen)
     __attribute__ ((__warn_unused_result__)) __attribute__ ((__access__ (__write_only__, 2, 3)));
extern ssize_t __read_alias (int __fd, void *__buf, size_t __nbytes) __asm__ ("" "read")

     __attribute__ ((__warn_unused_result__)) __attribute__ ((__access__ (__write_only__, 2, 3)));
extern ssize_t __read_chk_warn (int __fd, void *__buf, size_t __nbytes, size_t __buflen) __asm__ ("" "__read_chk")


     __attribute__ ((__warn_unused_result__)) __attribute__((__warning__ ("read called with bigger length than size of " "the destination buffer")))
                                  ;


extern ssize_t __pread_chk (int __fd, void *__buf, size_t __nbytes,
       __off_t __offset, size_t __bufsize)
     __attribute__ ((__warn_unused_result__)) __attribute__ ((__access__ (__write_only__, 2, 3)));
extern ssize_t __pread64_chk (int __fd, void *__buf, size_t __nbytes,
         __off64_t __offset, size_t __bufsize)
     __attribute__ ((__warn_unused_result__)) __attribute__ ((__access__ (__write_only__, 2, 3)));
extern ssize_t __pread_alias (int __fd, void *__buf, size_t __nbytes, __off_t __offset) __asm__ ("" "pread")


     __attribute__ ((__warn_unused_result__)) __attribute__ ((__access__ (__write_only__, 2, 3)));
extern ssize_t __pread64_alias (int __fd, void *__buf, size_t __nbytes, __off64_t __offset) __asm__ ("" "pread64")


     __attribute__ ((__warn_unused_result__)) __attribute__ ((__access__ (__write_only__, 2, 3)));
extern ssize_t __pread_chk_warn (int __fd, void *__buf, size_t __nbytes, __off_t __offset, size_t __bufsize) __asm__ ("" "__pread_chk")


     __attribute__ ((__warn_unused_result__)) __attribute__((__warning__ ("pread called with bigger length than size of " "the destination buffer")))
                                  ;
extern ssize_t __pread64_chk_warn (int __fd, void *__buf, size_t __nbytes, __off64_t __offset, size_t __bufsize) __asm__ ("" "__pread64_chk")



     __attribute__ ((__warn_unused_result__)) __attribute__((__warning__ ("pread64 called with bigger length than size of " "the destination buffer")))
                                  ;



extern ssize_t __readlink_chk (const char *__restrict __path,
          char *__restrict __buf, size_t __len,
          size_t __buflen)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2))) __attribute__ ((__warn_unused_result__)) __attribute__ ((__access__ (__write_only__, 2, 3)));
extern ssize_t __readlink_alias (const char *__restrict __path, char *__restrict __buf, size_t __len) noexcept (true) __asm__ ("" "readlink")


     __attribute__ ((__nonnull__ (1, 2))) __attribute__ ((__warn_unused_result__)) __attribute__ ((__access__ (__write_only__, 2, 3)));
extern ssize_t __readlink_chk_warn (const char *__restrict __path, char *__restrict __buf, size_t __len, size_t __buflen) noexcept (true) __asm__ ("" "__readlink_chk")



     __attribute__ ((__nonnull__ (1, 2))) __attribute__ ((__warn_unused_result__)) __attribute__((__warning__ ("readlink called with bigger length " "than size of destination buffer")))
                                         ;



extern ssize_t __readlinkat_chk (int __fd, const char *__restrict __path,
     char *__restrict __buf, size_t __len,
     size_t __buflen)
     noexcept (true) __attribute__ ((__nonnull__ (2, 3))) __attribute__ ((__warn_unused_result__)) __attribute__ ((__access__ (__write_only__, 3, 4)));
extern ssize_t __readlinkat_alias (int __fd, const char *__restrict __path, char *__restrict __buf, size_t __len) noexcept (true) __asm__ ("" "readlinkat")



     __attribute__ ((__nonnull__ (2, 3))) __attribute__ ((__warn_unused_result__)) __attribute__ ((__access__ (__write_only__, 3, 4)));
extern ssize_t __readlinkat_chk_warn (int __fd, const char *__restrict __path, char *__restrict __buf, size_t __len, size_t __buflen) noexcept (true) __asm__ ("" "__readlinkat_chk")



     __attribute__ ((__nonnull__ (2, 3))) __attribute__ ((__warn_unused_result__)) __attribute__((__warning__ ("readlinkat called with bigger " "length than size of destination " "buffer")))

                ;


extern char *__getcwd_chk (char *__buf, size_t __size, size_t __buflen)
     noexcept (true) __attribute__ ((__warn_unused_result__));
extern char *__getcwd_alias (char *__buf, size_t __size) noexcept (true) __asm__ ("" "getcwd")
                                              __attribute__ ((__warn_unused_result__));
extern char *__getcwd_chk_warn (char *__buf, size_t __size, size_t __buflen) noexcept (true) __asm__ ("" "__getcwd_chk")


     __attribute__ ((__warn_unused_result__)) __attribute__((__warning__ ("getcwd caller with bigger length than size of " "destination buffer")))
                              ;


extern char *__getwd_chk (char *__buf, size_t buflen)
     noexcept (true) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__)) __attribute__ ((__access__ (__write_only__, 1, 2)));
extern char *__getwd_warn (char *__buf) noexcept (true) __asm__ ("" "getwd")
     __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__)) __attribute__((__warning__ ("please use getcwd instead, as getwd " "doesn't specify buffer size")))
                                         ;


extern size_t __confstr_chk (int __name, char *__buf, size_t __len,
        size_t __buflen) noexcept (true)
     __attribute__ ((__access__ (__write_only__, 2, 3)));
extern size_t __confstr_alias (int __name, char *__buf, size_t __len) noexcept (true) __asm__ ("" "confstr")

     __attribute__ ((__access__ (__write_only__, 2, 3)));
extern size_t __confstr_chk_warn (int __name, char *__buf, size_t __len, size_t __buflen) noexcept (true) __asm__ ("" "__confstr_chk")


     __attribute__((__warning__ ("confstr called with bigger length than size of destination " "buffer")))
            ;


extern int __getgroups_chk (int __size, __gid_t __list[], size_t __listlen)
     noexcept (true) __attribute__ ((__warn_unused_result__)) __attribute__ ((__access__ (__write_only__, 2, 1)));
extern int __getgroups_alias (int __size, __gid_t __list[]) noexcept (true) __asm__ ("" "getgroups")
                 __attribute__ ((__warn_unused_result__)) __attribute__ ((__access__ (__write_only__, 2, 1)));
extern int __getgroups_chk_warn (int __size, __gid_t __list[], size_t __listlen) noexcept (true) __asm__ ("" "__getgroups_chk")


     __attribute__ ((__warn_unused_result__)) __attribute__((__warning__ ("getgroups called with bigger group count than what " "can fit into destination buffer")))
                                           ;


extern int __ttyname_r_chk (int __fd, char *__buf, size_t __buflen,
       size_t __nreal) noexcept (true) __attribute__ ((__nonnull__ (2)))
     __attribute__ ((__access__ (__write_only__, 2, 3)));
extern int __ttyname_r_alias (int __fd, char *__buf, size_t __buflen) noexcept (true) __asm__ ("" "ttyname_r")

     __attribute__ ((__nonnull__ (2)));
extern int __ttyname_r_chk_warn (int __fd, char *__buf, size_t __buflen, size_t __nreal) noexcept (true) __asm__ ("" "__ttyname_r_chk")


     __attribute__ ((__nonnull__ (2))) __attribute__((__warning__ ("ttyname_r called with bigger buflen than " "size of destination buffer")))
                                  ;



extern int __getlogin_r_chk (char *__buf, size_t __buflen, size_t __nreal)
     __attribute__ ((__nonnull__ (1))) __attribute__ ((__access__ (__write_only__, 1, 2)));
extern int __getlogin_r_alias (char *__buf, size_t __buflen) __asm__ ("" "getlogin_r")
                     __attribute__ ((__nonnull__ (1)));
extern int __getlogin_r_chk_warn (char *__buf, size_t __buflen, size_t __nreal) __asm__ ("" "__getlogin_r_chk")


     __attribute__ ((__nonnull__ (1))) __attribute__((__warning__ ("getlogin_r called with bigger buflen than " "size of destination buffer")))
                                  ;




extern int __gethostname_chk (char *__buf, size_t __buflen, size_t __nreal)
     noexcept (true) __attribute__ ((__nonnull__ (1))) __attribute__ ((__access__ (__write_only__, 1, 2)));
extern int __gethostname_alias (char *__buf, size_t __buflen) noexcept (true) __asm__ ("" "gethostname")

     __attribute__ ((__nonnull__ (1))) __attribute__ ((__access__ (__write_only__, 1, 2)));
extern int __gethostname_chk_warn (char *__buf, size_t __buflen, size_t __nreal) noexcept (true) __asm__ ("" "__gethostname_chk")


     __attribute__ ((__nonnull__ (1))) __attribute__((__warning__ ("gethostname called with bigger buflen than " "size of destination buffer")))
                                  ;




extern int __getdomainname_chk (char *__buf, size_t __buflen, size_t __nreal)
     noexcept (true) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__)) __attribute__ ((__access__ (__write_only__, 1, 2)));
extern int __getdomainname_alias (char *__buf, size_t __buflen) noexcept (true) __asm__ ("" "getdomainname")

                     __attribute__ ((__nonnull__ (1)))
     __attribute__ ((__warn_unused_result__)) __attribute__ ((__access__ (__write_only__, 1, 2)));
extern int __getdomainname_chk_warn (char *__buf, size_t __buflen, size_t __nreal) noexcept (true) __asm__ ("" "__getdomainname_chk")


     __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__)) __attribute__((__warning__ ("getdomainname called with bigger " "buflen than size of destination " "buffer")))

                    ;
# 24 "/usr/include/x86_64-linux-gnu/bits/unistd.h" 2 3 4

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) __attribute__ ((__warn_unused_result__)) ssize_t
read (int __fd, void * __buf, size_t __nbytes)
    



{
  return (((__builtin_constant_p (__builtin_dynamic_object_size (__buf, 0)) && (__builtin_dynamic_object_size (__buf, 0)) == (long unsigned int) -1) || (((__typeof (__nbytes)) 0 < (__typeof (__nbytes)) -1 || (__builtin_constant_p (__nbytes) && (__nbytes) > 0)) && __builtin_constant_p ((((long unsigned int) (__nbytes)) <= ((__builtin_dynamic_object_size (__buf, 0))) / ((sizeof (char))))) && (((long unsigned int) (__nbytes)) <= ((__builtin_dynamic_object_size (__buf, 0))) / ((sizeof (char)))))) ? __read_alias (__fd, __buf, __nbytes) : ((((__typeof (__nbytes)) 0 < (__typeof (__nbytes)) -1 || (__builtin_constant_p (__nbytes) && (__nbytes) > 0)) && __builtin_constant_p ((((long unsigned int) (__nbytes)) <= (__builtin_dynamic_object_size (__buf, 0)) / (sizeof (char)))) && !(((long unsigned int) (__nbytes)) <= (__builtin_dynamic_object_size (__buf, 0)) / (sizeof (char)))) ? __read_chk_warn (__fd, __buf, __nbytes, __builtin_dynamic_object_size (__buf, 0)) : __read_chk (__fd, __buf, __nbytes, __builtin_dynamic_object_size (__buf, 0))))

                           ;
}
# 51 "/usr/include/x86_64-linux-gnu/bits/unistd.h" 3 4
extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) __attribute__ ((__warn_unused_result__)) ssize_t
pread (int __fd, void * __buf,
       size_t __nbytes, __off64_t __offset)
    


{
  return (((__builtin_constant_p (__builtin_dynamic_object_size (__buf, 0)) && (__builtin_dynamic_object_size (__buf, 0)) == (long unsigned int) -1) || (((__typeof (__nbytes)) 0 < (__typeof (__nbytes)) -1 || (__builtin_constant_p (__nbytes) && (__nbytes) > 0)) && __builtin_constant_p ((((long unsigned int) (__nbytes)) <= ((__builtin_dynamic_object_size (__buf, 0))) / ((sizeof (char))))) && (((long unsigned int) (__nbytes)) <= ((__builtin_dynamic_object_size (__buf, 0))) / ((sizeof (char)))))) ? __pread64_alias (__fd, __buf, __nbytes, __offset) : ((((__typeof (__nbytes)) 0 < (__typeof (__nbytes)) -1 || (__builtin_constant_p (__nbytes) && (__nbytes) > 0)) && __builtin_constant_p ((((long unsigned int) (__nbytes)) <= (__builtin_dynamic_object_size (__buf, 0)) / (sizeof (char)))) && !(((long unsigned int) (__nbytes)) <= (__builtin_dynamic_object_size (__buf, 0)) / (sizeof (char)))) ? __pread64_chk_warn (__fd, __buf, __nbytes, __offset, __builtin_dynamic_object_size (__buf, 0)) : __pread64_chk (__fd, __buf, __nbytes, __offset, __builtin_dynamic_object_size (__buf, 0))))

                                     ;
}



extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) __attribute__ ((__warn_unused_result__)) ssize_t
pread64 (int __fd, void * __buf,
  size_t __nbytes, __off64_t __offset)
    


{
  return (((__builtin_constant_p (__builtin_dynamic_object_size (__buf, 0)) && (__builtin_dynamic_object_size (__buf, 0)) == (long unsigned int) -1) || (((__typeof (__nbytes)) 0 < (__typeof (__nbytes)) -1 || (__builtin_constant_p (__nbytes) && (__nbytes) > 0)) && __builtin_constant_p ((((long unsigned int) (__nbytes)) <= ((__builtin_dynamic_object_size (__buf, 0))) / ((sizeof (char))))) && (((long unsigned int) (__nbytes)) <= ((__builtin_dynamic_object_size (__buf, 0))) / ((sizeof (char)))))) ? __pread64_alias (__fd, __buf, __nbytes, __offset) : ((((__typeof (__nbytes)) 0 < (__typeof (__nbytes)) -1 || (__builtin_constant_p (__nbytes) && (__nbytes) > 0)) && __builtin_constant_p ((((long unsigned int) (__nbytes)) <= (__builtin_dynamic_object_size (__buf, 0)) / (sizeof (char)))) && !(((long unsigned int) (__nbytes)) <= (__builtin_dynamic_object_size (__buf, 0)) / (sizeof (char)))) ? __pread64_chk_warn (__fd, __buf, __nbytes, __offset, __builtin_dynamic_object_size (__buf, 0)) : __pread64_chk (__fd, __buf, __nbytes, __offset, __builtin_dynamic_object_size (__buf, 0))))

                                     ;
}




extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) __attribute__ ((__nonnull__ (1, 2))) __attribute__ ((__warn_unused_result__)) ssize_t
__attribute__ ((__leaf__)) readlink (const char *__restrict __path, char * __restrict __buf, size_t __len) noexcept (true)


    



{
  return (((__builtin_constant_p (__builtin_dynamic_object_size (__buf, 1)) && (__builtin_dynamic_object_size (__buf, 1)) == (long unsigned int) -1) || (((__typeof (__len)) 0 < (__typeof (__len)) -1 || (__builtin_constant_p (__len) && (__len) > 0)) && __builtin_constant_p ((((long unsigned int) (__len)) <= ((__builtin_dynamic_object_size (__buf, 1))) / ((sizeof (char))))) && (((long unsigned int) (__len)) <= ((__builtin_dynamic_object_size (__buf, 1))) / ((sizeof (char)))))) ? __readlink_alias (__path, __buf, __len) : ((((__typeof (__len)) 0 < (__typeof (__len)) -1 || (__builtin_constant_p (__len) && (__len) > 0)) && __builtin_constant_p ((((long unsigned int) (__len)) <= (__builtin_dynamic_object_size (__buf, 1)) / (sizeof (char)))) && !(((long unsigned int) (__len)) <= (__builtin_dynamic_object_size (__buf, 1)) / (sizeof (char)))) ? __readlink_chk_warn (__path, __buf, __len, __builtin_dynamic_object_size (__buf, 1)) : __readlink_chk (__path, __buf, __len, __builtin_dynamic_object_size (__buf, 1))))

                          ;
}



extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) __attribute__ ((__nonnull__ (2, 3))) __attribute__ ((__warn_unused_result__)) ssize_t
__attribute__ ((__leaf__)) readlinkat (int __fd, const char *__restrict __path, char * __restrict __buf, size_t __len) noexcept (true)


    


{
  return (((__builtin_constant_p (__builtin_dynamic_object_size (__buf, 1)) && (__builtin_dynamic_object_size (__buf, 1)) == (long unsigned int) -1) || (((__typeof (__len)) 0 < (__typeof (__len)) -1 || (__builtin_constant_p (__len) && (__len) > 0)) && __builtin_constant_p ((((long unsigned int) (__len)) <= ((__builtin_dynamic_object_size (__buf, 1))) / ((sizeof (char))))) && (((long unsigned int) (__len)) <= ((__builtin_dynamic_object_size (__buf, 1))) / ((sizeof (char)))))) ? __readlinkat_alias (__fd, __path, __buf, __len) : ((((__typeof (__len)) 0 < (__typeof (__len)) -1 || (__builtin_constant_p (__len) && (__len) > 0)) && __builtin_constant_p ((((long unsigned int) (__len)) <= (__builtin_dynamic_object_size (__buf, 1)) / (sizeof (char)))) && !(((long unsigned int) (__len)) <= (__builtin_dynamic_object_size (__buf, 1)) / (sizeof (char)))) ? __readlinkat_chk_warn (__fd, __path, __buf, __len, __builtin_dynamic_object_size (__buf, 1)) : __readlinkat_chk (__fd, __path, __buf, __len, __builtin_dynamic_object_size (__buf, 1))))

                                ;
}


extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) __attribute__ ((__warn_unused_result__)) char *
__attribute__ ((__leaf__)) getcwd (char * __buf, size_t __size) noexcept (true)
    


{
  return (((__builtin_constant_p (__builtin_dynamic_object_size (__buf, 1)) && (__builtin_dynamic_object_size (__buf, 1)) == (long unsigned int) -1) || (((__typeof (__size)) 0 < (__typeof (__size)) -1 || (__builtin_constant_p (__size) && (__size) > 0)) && __builtin_constant_p ((((long unsigned int) (__size)) <= ((__builtin_dynamic_object_size (__buf, 1))) / ((sizeof (char))))) && (((long unsigned int) (__size)) <= ((__builtin_dynamic_object_size (__buf, 1))) / ((sizeof (char)))))) ? __getcwd_alias (__buf, __size) : ((((__typeof (__size)) 0 < (__typeof (__size)) -1 || (__builtin_constant_p (__size) && (__size) > 0)) && __builtin_constant_p ((((long unsigned int) (__size)) <= (__builtin_dynamic_object_size (__buf, 1)) / (sizeof (char)))) && !(((long unsigned int) (__size)) <= (__builtin_dynamic_object_size (__buf, 1)) / (sizeof (char)))) ? __getcwd_chk_warn (__buf, __size, __builtin_dynamic_object_size (__buf, 1)) : __getcwd_chk (__buf, __size, __builtin_dynamic_object_size (__buf, 1))))

                   ;
}


extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) __attribute__ ((__nonnull__ (1)))
__attribute__ ((__deprecated__)) __attribute__ ((__warn_unused_result__)) char *
__attribute__ ((__leaf__)) getwd (char * __buf) noexcept (true)
{
  if (__builtin_dynamic_object_size (__buf, 1) != (size_t) -1)
    return __getwd_chk (__buf, __builtin_dynamic_object_size (__buf, 1));
  return __getwd_warn (__buf);
}


extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) size_t
__attribute__ ((__leaf__)) confstr (int __name, char * __buf, size_t __len) noexcept (true)

    


{
  return (((__builtin_constant_p (__builtin_dynamic_object_size (__buf, 1)) && (__builtin_dynamic_object_size (__buf, 1)) == (long unsigned int) -1) || (((__typeof (__len)) 0 < (__typeof (__len)) -1 || (__builtin_constant_p (__len) && (__len) > 0)) && __builtin_constant_p ((((long unsigned int) (__len)) <= ((__builtin_dynamic_object_size (__buf, 1))) / ((sizeof (char))))) && (((long unsigned int) (__len)) <= ((__builtin_dynamic_object_size (__buf, 1))) / ((sizeof (char)))))) ? __confstr_alias (__name, __buf, __len) : ((((__typeof (__len)) 0 < (__typeof (__len)) -1 || (__builtin_constant_p (__len) && (__len) > 0)) && __builtin_constant_p ((((long unsigned int) (__len)) <= (__builtin_dynamic_object_size (__buf, 1)) / (sizeof (char)))) && !(((long unsigned int) (__len)) <= (__builtin_dynamic_object_size (__buf, 1)) / (sizeof (char)))) ? __confstr_chk_warn (__name, __buf, __len, __builtin_dynamic_object_size (__buf, 1)) : __confstr_chk (__name, __buf, __len, __builtin_dynamic_object_size (__buf, 1))))

                          ;
}


extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) int
__attribute__ ((__leaf__)) getgroups (int __size, __gid_t * __list) noexcept (true)

    



{
  return (((__builtin_constant_p (__builtin_dynamic_object_size (__list, 1)) && (__builtin_dynamic_object_size (__list, 1)) == (long unsigned int) -1) || (((__typeof (__size)) 0 < (__typeof (__size)) -1 || (__builtin_constant_p (__size) && (__size) > 0)) && __builtin_constant_p ((((long unsigned int) (__size)) <= ((__builtin_dynamic_object_size (__list, 1))) / ((sizeof (__gid_t))))) && (((long unsigned int) (__size)) <= ((__builtin_dynamic_object_size (__list, 1))) / ((sizeof (__gid_t)))))) ? __getgroups_alias (__size, __list) : ((((__typeof (__size)) 0 < (__typeof (__size)) -1 || (__builtin_constant_p (__size) && (__size) > 0)) && __builtin_constant_p ((((long unsigned int) (__size)) <= (__builtin_dynamic_object_size (__list, 1)) / (sizeof (__gid_t)))) && !(((long unsigned int) (__size)) <= (__builtin_dynamic_object_size (__list, 1)) / (sizeof (__gid_t)))) ? __getgroups_chk_warn (__size, __list, __builtin_dynamic_object_size (__list, 1)) : __getgroups_chk (__size, __list, __builtin_dynamic_object_size (__list, 1))))

                    ;
}


extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) int
__attribute__ ((__leaf__)) ttyname_r (int __fd, char * __buf, size_t __buflen) noexcept (true)


    


{
  return (((__builtin_constant_p (__builtin_dynamic_object_size (__buf, 1)) && (__builtin_dynamic_object_size (__buf, 1)) == (long unsigned int) -1) || (((__typeof (__buflen)) 0 < (__typeof (__buflen)) -1 || (__builtin_constant_p (__buflen) && (__buflen) > 0)) && __builtin_constant_p ((((long unsigned int) (__buflen)) <= ((__builtin_dynamic_object_size (__buf, 1))) / ((sizeof (char))))) && (((long unsigned int) (__buflen)) <= ((__builtin_dynamic_object_size (__buf, 1))) / ((sizeof (char)))))) ? __ttyname_r_alias (__fd, __buf, __buflen) : ((((__typeof (__buflen)) 0 < (__typeof (__buflen)) -1 || (__builtin_constant_p (__buflen) && (__buflen) > 0)) && __builtin_constant_p ((((long unsigned int) (__buflen)) <= (__builtin_dynamic_object_size (__buf, 1)) / (sizeof (char)))) && !(((long unsigned int) (__buflen)) <= (__builtin_dynamic_object_size (__buf, 1)) / (sizeof (char)))) ? __ttyname_r_chk_warn (__fd, __buf, __buflen, __builtin_dynamic_object_size (__buf, 1)) : __ttyname_r_chk (__fd, __buf, __buflen, __builtin_dynamic_object_size (__buf, 1))))

                           ;
}



extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) int
getlogin_r (char * __buf, size_t __buflen)
    


{
  return (((__builtin_constant_p (__builtin_dynamic_object_size (__buf, 1)) && (__builtin_dynamic_object_size (__buf, 1)) == (long unsigned int) -1) || (((__typeof (__buflen)) 0 < (__typeof (__buflen)) -1 || (__builtin_constant_p (__buflen) && (__buflen) > 0)) && __builtin_constant_p ((((long unsigned int) (__buflen)) <= ((__builtin_dynamic_object_size (__buf, 1))) / ((sizeof (char))))) && (((long unsigned int) (__buflen)) <= ((__builtin_dynamic_object_size (__buf, 1))) / ((sizeof (char)))))) ? __getlogin_r_alias (__buf, __buflen) : ((((__typeof (__buflen)) 0 < (__typeof (__buflen)) -1 || (__builtin_constant_p (__buflen) && (__buflen) > 0)) && __builtin_constant_p ((((long unsigned int) (__buflen)) <= (__builtin_dynamic_object_size (__buf, 1)) / (sizeof (char)))) && !(((long unsigned int) (__buflen)) <= (__builtin_dynamic_object_size (__buf, 1)) / (sizeof (char)))) ? __getlogin_r_chk_warn (__buf, __buflen, __builtin_dynamic_object_size (__buf, 1)) : __getlogin_r_chk (__buf, __buflen, __builtin_dynamic_object_size (__buf, 1))))

                     ;
}




extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) int
__attribute__ ((__leaf__)) gethostname (char * __buf, size_t __buflen) noexcept (true)

    


{
  return (((__builtin_constant_p (__builtin_dynamic_object_size (__buf, 1)) && (__builtin_dynamic_object_size (__buf, 1)) == (long unsigned int) -1) || (((__typeof (__buflen)) 0 < (__typeof (__buflen)) -1 || (__builtin_constant_p (__buflen) && (__buflen) > 0)) && __builtin_constant_p ((((long unsigned int) (__buflen)) <= ((__builtin_dynamic_object_size (__buf, 1))) / ((sizeof (char))))) && (((long unsigned int) (__buflen)) <= ((__builtin_dynamic_object_size (__buf, 1))) / ((sizeof (char)))))) ? __gethostname_alias (__buf, __buflen) : ((((__typeof (__buflen)) 0 < (__typeof (__buflen)) -1 || (__builtin_constant_p (__buflen) && (__buflen) > 0)) && __builtin_constant_p ((((long unsigned int) (__buflen)) <= (__builtin_dynamic_object_size (__buf, 1)) / (sizeof (char)))) && !(((long unsigned int) (__buflen)) <= (__builtin_dynamic_object_size (__buf, 1)) / (sizeof (char)))) ? __gethostname_chk_warn (__buf, __buflen, __builtin_dynamic_object_size (__buf, 1)) : __gethostname_chk (__buf, __buflen, __builtin_dynamic_object_size (__buf, 1))))

                     ;
}




extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) int
__attribute__ ((__leaf__)) getdomainname (char * __buf, size_t __buflen) noexcept (true)

    


{
  return (((__builtin_constant_p (__builtin_dynamic_object_size (__buf, 1)) && (__builtin_dynamic_object_size (__buf, 1)) == (long unsigned int) -1) || (((__typeof (__buflen)) 0 < (__typeof (__buflen)) -1 || (__builtin_constant_p (__buflen) && (__buflen) > 0)) && __builtin_constant_p ((((long unsigned int) (__buflen)) <= ((__builtin_dynamic_object_size (__buf, 1))) / ((sizeof (char))))) && (((long unsigned int) (__buflen)) <= ((__builtin_dynamic_object_size (__buf, 1))) / ((sizeof (char)))))) ? __getdomainname_alias (__buf, __buflen) : ((((__typeof (__buflen)) 0 < (__typeof (__buflen)) -1 || (__builtin_constant_p (__buflen) && (__buflen) > 0)) && __builtin_constant_p ((((long unsigned int) (__buflen)) <= (__builtin_dynamic_object_size (__buf, 1)) / (sizeof (char)))) && !(((long unsigned int) (__buflen)) <= (__builtin_dynamic_object_size (__buf, 1)) / (sizeof (char)))) ? __getdomainname_chk_warn (__buf, __buflen, __builtin_dynamic_object_size (__buf, 1)) : __getdomainname_chk (__buf, __buflen, __builtin_dynamic_object_size (__buf, 1))))

                     ;
}
# 1218 "/usr/include/unistd.h" 2 3 4



# 1 "/usr/include/x86_64-linux-gnu/bits/unistd_ext.h" 1 3 4
# 34 "/usr/include/x86_64-linux-gnu/bits/unistd_ext.h" 3 4
extern __pid_t gettid (void) noexcept (true);



# 1 "/usr/include/linux/close_range.h" 1 3 4
# 39 "/usr/include/x86_64-linux-gnu/bits/unistd_ext.h" 2 3 4
# 1222 "/usr/include/unistd.h" 2 3 4

}
# 70 "../../bochs.h" 2



# 1 "/usr/include/time.h" 1 3 4
# 29 "/usr/include/time.h" 3 4
# 1 "/usr/lib/gcc/x86_64-linux-gnu/14/include/stddef.h" 1 3 4
# 30 "/usr/include/time.h" 2 3 4



# 1 "/usr/include/x86_64-linux-gnu/bits/time.h" 1 3 4
# 73 "/usr/include/x86_64-linux-gnu/bits/time.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/timex.h" 1 3 4
# 26 "/usr/include/x86_64-linux-gnu/bits/timex.h" 3 4
struct timex
{
# 58 "/usr/include/x86_64-linux-gnu/bits/timex.h" 3 4
  unsigned int modes;
  __syscall_slong_t offset;
  __syscall_slong_t freq;
  __syscall_slong_t maxerror;
  __syscall_slong_t esterror;
  int status;
  __syscall_slong_t constant;
  __syscall_slong_t precision;
  __syscall_slong_t tolerance;
  struct timeval time;
  __syscall_slong_t tick;
  __syscall_slong_t ppsfreq;
  __syscall_slong_t jitter;
  int shift;
  __syscall_slong_t stabil;
  __syscall_slong_t jitcnt;
  __syscall_slong_t calcnt;
  __syscall_slong_t errcnt;
  __syscall_slong_t stbcnt;

  int tai;


  int :32; int :32; int :32; int :32;
  int :32; int :32; int :32; int :32;
  int :32; int :32; int :32;

};
# 74 "/usr/include/x86_64-linux-gnu/bits/time.h" 2 3 4

extern "C" {


extern int clock_adjtime (__clockid_t __clock_id, struct timex *__utx) noexcept (true) __attribute__ ((__nonnull__ (2)));
# 90 "/usr/include/x86_64-linux-gnu/bits/time.h" 3 4
}
# 34 "/usr/include/time.h" 2 3 4





# 1 "/usr/include/x86_64-linux-gnu/bits/types/struct_tm.h" 1 3 4






struct tm
{
  int tm_sec;
  int tm_min;
  int tm_hour;
  int tm_mday;
  int tm_mon;
  int tm_year;
  int tm_wday;
  int tm_yday;
  int tm_isdst;


  long int tm_gmtoff;
  const char *tm_zone;




};
# 40 "/usr/include/time.h" 2 3 4
# 48 "/usr/include/time.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/types/struct_itimerspec.h" 1 3 4







struct itimerspec
  {
    struct timespec it_interval;
    struct timespec it_value;
  };
# 49 "/usr/include/time.h" 2 3 4
struct sigevent;
# 68 "/usr/include/time.h" 3 4
extern "C" {



extern clock_t clock (void) noexcept (true);



extern time_t time (time_t *__timer) noexcept (true);


extern double difftime (time_t __time1, time_t __time0);


extern time_t mktime (struct tm *__tp) noexcept (true);
# 99 "/usr/include/time.h" 3 4
extern size_t strftime (char *__restrict __s, size_t __maxsize,
   const char *__restrict __format,
   const struct tm *__restrict __tp)
   noexcept (true) __attribute__ ((__nonnull__ (1, 3, 4)));




extern char *strptime (const char *__restrict __s,
         const char *__restrict __fmt, struct tm *__tp)
     noexcept (true);






extern size_t strftime_l (char *__restrict __s, size_t __maxsize,
     const char *__restrict __format,
     const struct tm *__restrict __tp,
     locale_t __loc) noexcept (true);



extern char *strptime_l (const char *__restrict __s,
    const char *__restrict __fmt, struct tm *__tp,
    locale_t __loc) noexcept (true);






extern struct tm *gmtime (const time_t *__timer) noexcept (true);



extern struct tm *localtime (const time_t *__timer) noexcept (true);
# 154 "/usr/include/time.h" 3 4
extern struct tm *gmtime_r (const time_t *__restrict __timer,
       struct tm *__restrict __tp) noexcept (true);



extern struct tm *localtime_r (const time_t *__restrict __timer,
          struct tm *__restrict __tp) noexcept (true);
# 179 "/usr/include/time.h" 3 4
extern char *asctime (const struct tm *__tp) noexcept (true);



extern char *ctime (const time_t *__timer) noexcept (true);
# 197 "/usr/include/time.h" 3 4
extern char *asctime_r (const struct tm *__restrict __tp,
   char *__restrict __buf) noexcept (true);



extern char *ctime_r (const time_t *__restrict __timer,
        char *__restrict __buf) noexcept (true);
# 217 "/usr/include/time.h" 3 4
extern char *__tzname[2];
extern int __daylight;
extern long int __timezone;




extern char *tzname[2];



extern void tzset (void) noexcept (true);



extern int daylight;
extern long int timezone;
# 246 "/usr/include/time.h" 3 4
extern time_t timegm (struct tm *__tp) noexcept (true);
# 263 "/usr/include/time.h" 3 4
extern time_t timelocal (struct tm *__tp) noexcept (true);







extern int dysize (int __year) noexcept (true) __attribute__ ((__const__));
# 281 "/usr/include/time.h" 3 4
extern int nanosleep (const struct timespec *__requested_time,
        struct timespec *__remaining);


extern int clock_getres (clockid_t __clock_id, struct timespec *__res) noexcept (true);


extern int clock_gettime (clockid_t __clock_id, struct timespec *__tp)
     noexcept (true) __attribute__ ((__nonnull__ (2)));


extern int clock_settime (clockid_t __clock_id, const struct timespec *__tp)
     noexcept (true) __attribute__ ((__nonnull__ (2)));
# 323 "/usr/include/time.h" 3 4
extern int clock_nanosleep (clockid_t __clock_id, int __flags,
       const struct timespec *__req,
       struct timespec *__rem);
# 338 "/usr/include/time.h" 3 4
extern int clock_getcpuclockid (pid_t __pid, clockid_t *__clock_id) noexcept (true);




extern int timer_create (clockid_t __clock_id,
    struct sigevent *__restrict __evp,
    timer_t *__restrict __timerid) noexcept (true);


extern int timer_delete (timer_t __timerid) noexcept (true);



extern int timer_settime (timer_t __timerid, int __flags,
     const struct itimerspec *__restrict __value,
     struct itimerspec *__restrict __ovalue) noexcept (true);


extern int timer_gettime (timer_t __timerid, struct itimerspec *__value)
     noexcept (true);
# 376 "/usr/include/time.h" 3 4
extern int timer_getoverrun (timer_t __timerid) noexcept (true);






extern int timespec_get (struct timespec *__ts, int __base)
     noexcept (true) __attribute__ ((__nonnull__ (1)));
# 399 "/usr/include/time.h" 3 4
extern int timespec_getres (struct timespec *__ts, int __base)
     noexcept (true);
# 425 "/usr/include/time.h" 3 4
extern int getdate_err;
# 434 "/usr/include/time.h" 3 4
extern struct tm *getdate (const char *__string);
# 448 "/usr/include/time.h" 3 4
extern int getdate_r (const char *__restrict __string,
        struct tm *__restrict __resbufp);


}
# 74 "../../bochs.h" 2
# 87 "../../bochs.h"
# 1 "/usr/include/x86_64-linux-gnu/sys/time.h" 1 3 4
# 34 "/usr/include/x86_64-linux-gnu/sys/time.h" 3 4
extern "C" {
# 52 "/usr/include/x86_64-linux-gnu/sys/time.h" 3 4
struct timezone
  {
    int tz_minuteswest;
    int tz_dsttime;
  };
# 67 "/usr/include/x86_64-linux-gnu/sys/time.h" 3 4
extern int gettimeofday (struct timeval *__restrict __tv,
    void *__restrict __tz) noexcept (true) __attribute__ ((__nonnull__ (1)));
# 86 "/usr/include/x86_64-linux-gnu/sys/time.h" 3 4
extern int settimeofday (const struct timeval *__tv,
    const struct timezone *__tz)
     noexcept (true);





extern int adjtime (const struct timeval *__delta,
      struct timeval *__olddelta) noexcept (true);
# 114 "/usr/include/x86_64-linux-gnu/sys/time.h" 3 4
enum __itimer_which
  {

    ITIMER_REAL = 0,


    ITIMER_VIRTUAL = 1,



    ITIMER_PROF = 2

  };



struct itimerval
  {

    struct timeval it_interval;

    struct timeval it_value;
  };






typedef int __itimer_which_t;





extern int getitimer (__itimer_which_t __which,
        struct itimerval *__value) noexcept (true);




extern int setitimer (__itimer_which_t __which,
        const struct itimerval *__restrict __new,
        struct itimerval *__restrict __old) noexcept (true);




extern int utimes (const char *__file, const struct timeval __tvp[2])
     noexcept (true) __attribute__ ((__nonnull__ (1)));
# 189 "/usr/include/x86_64-linux-gnu/sys/time.h" 3 4
extern int lutimes (const char *__file, const struct timeval __tvp[2])
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int futimes (int __fd, const struct timeval __tvp[2]) noexcept (true);
# 214 "/usr/include/x86_64-linux-gnu/sys/time.h" 3 4
extern int futimesat (int __fd, const char *__file,
        const struct timeval __tvp[2]) noexcept (true);
# 258 "/usr/include/x86_64-linux-gnu/sys/time.h" 3 4
}
# 88 "../../bochs.h" 2


# 1 "/usr/include/x86_64-linux-gnu/sys/stat.h" 1 3 4
# 99 "/usr/include/x86_64-linux-gnu/sys/stat.h" 3 4
extern "C" {

# 1 "/usr/include/x86_64-linux-gnu/bits/stat.h" 1 3 4
# 25 "/usr/include/x86_64-linux-gnu/bits/stat.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/struct_stat.h" 1 3 4
# 26 "/usr/include/x86_64-linux-gnu/bits/struct_stat.h" 3 4
struct stat
  {



    __dev_t st_dev;




    __ino_t st_ino;







    __nlink_t st_nlink;
    __mode_t st_mode;

    __uid_t st_uid;
    __gid_t st_gid;

    int __pad0;

    __dev_t st_rdev;




    __off_t st_size;



    __blksize_t st_blksize;

    __blkcnt_t st_blocks;
# 74 "/usr/include/x86_64-linux-gnu/bits/struct_stat.h" 3 4
    struct timespec st_atim;
    struct timespec st_mtim;
    struct timespec st_ctim;
# 89 "/usr/include/x86_64-linux-gnu/bits/struct_stat.h" 3 4
    __syscall_slong_t __glibc_reserved[3];
# 99 "/usr/include/x86_64-linux-gnu/bits/struct_stat.h" 3 4
  };



struct stat64
  {



    __dev_t st_dev;

    __ino64_t st_ino;
    __nlink_t st_nlink;
    __mode_t st_mode;






    __uid_t st_uid;
    __gid_t st_gid;

    int __pad0;
    __dev_t st_rdev;
    __off_t st_size;





    __blksize_t st_blksize;
    __blkcnt64_t st_blocks;







    struct timespec st_atim;
    struct timespec st_mtim;
    struct timespec st_ctim;
# 151 "/usr/include/x86_64-linux-gnu/bits/struct_stat.h" 3 4
    __syscall_slong_t __glibc_reserved[3];




  };
# 26 "/usr/include/x86_64-linux-gnu/bits/stat.h" 2 3 4
# 102 "/usr/include/x86_64-linux-gnu/sys/stat.h" 2 3 4
# 227 "/usr/include/x86_64-linux-gnu/sys/stat.h" 3 4
extern int stat (const char *__restrict __file, struct stat *__restrict __buf) noexcept (true) __asm__ ("" "stat64")

     __attribute__ ((__nonnull__ (1, 2)));
extern int fstat (int __fd, struct stat *__buf) noexcept (true) __asm__ ("" "fstat64")
     __attribute__ ((__nonnull__ (2)));
# 240 "/usr/include/x86_64-linux-gnu/sys/stat.h" 3 4
extern int stat64 (const char *__restrict __file,
     struct stat64 *__restrict __buf) noexcept (true) __attribute__ ((__nonnull__ (1, 2)));
extern int fstat64 (int __fd, struct stat64 *__buf) noexcept (true) __attribute__ ((__nonnull__ (2)));
# 279 "/usr/include/x86_64-linux-gnu/sys/stat.h" 3 4
extern int fstatat (int __fd, const char *__restrict __file, struct stat *__restrict __buf, int __flag) noexcept (true) __asm__ ("" "fstatat64")


                 __attribute__ ((__nonnull__ (3)));
# 291 "/usr/include/x86_64-linux-gnu/sys/stat.h" 3 4
extern int fstatat64 (int __fd, const char *__restrict __file,
        struct stat64 *__restrict __buf, int __flag)
     noexcept (true) __attribute__ ((__nonnull__ (3)));
# 327 "/usr/include/x86_64-linux-gnu/sys/stat.h" 3 4
extern int lstat (const char *__restrict __file, struct stat *__restrict __buf) noexcept (true) __asm__ ("" "lstat64")


     __attribute__ ((__nonnull__ (1, 2)));







extern int lstat64 (const char *__restrict __file,
      struct stat64 *__restrict __buf)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));
# 352 "/usr/include/x86_64-linux-gnu/sys/stat.h" 3 4
extern int chmod (const char *__file, __mode_t __mode)
     noexcept (true) __attribute__ ((__nonnull__ (1)));





extern int lchmod (const char *__file, __mode_t __mode)
     noexcept (true) __attribute__ ((__nonnull__ (1)));




extern int fchmod (int __fd, __mode_t __mode) noexcept (true);





extern int fchmodat (int __fd, const char *__file, __mode_t __mode,
       int __flag)
     noexcept (true) __attribute__ ((__nonnull__ (2))) __attribute__ ((__warn_unused_result__));






extern __mode_t umask (__mode_t __mask) noexcept (true);




extern __mode_t getumask (void) noexcept (true);



extern int mkdir (const char *__path, __mode_t __mode)
     noexcept (true) __attribute__ ((__nonnull__ (1)));





extern int mkdirat (int __fd, const char *__path, __mode_t __mode)
     noexcept (true) __attribute__ ((__nonnull__ (2)));






extern int mknod (const char *__path, __mode_t __mode, __dev_t __dev)
     noexcept (true) __attribute__ ((__nonnull__ (1)));





extern int mknodat (int __fd, const char *__path, __mode_t __mode,
      __dev_t __dev) noexcept (true) __attribute__ ((__nonnull__ (2)));





extern int mkfifo (const char *__path, __mode_t __mode)
     noexcept (true) __attribute__ ((__nonnull__ (1)));





extern int mkfifoat (int __fd, const char *__path, __mode_t __mode)
     noexcept (true) __attribute__ ((__nonnull__ (2)));






extern int utimensat (int __fd, const char *__path,
        const struct timespec __times[2],
        int __flags)
     noexcept (true);
# 452 "/usr/include/x86_64-linux-gnu/sys/stat.h" 3 4
extern int futimens (int __fd, const struct timespec __times[2]) noexcept (true);
# 465 "/usr/include/x86_64-linux-gnu/sys/stat.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/statx.h" 1 3 4
# 31 "/usr/include/x86_64-linux-gnu/bits/statx.h" 3 4
# 1 "/usr/include/linux/stat.h" 1 3 4




# 1 "/usr/include/linux/types.h" 1 3 4




# 1 "/usr/include/x86_64-linux-gnu/asm/types.h" 1 3 4
# 1 "/usr/include/asm-generic/types.h" 1 3 4






# 1 "/usr/include/asm-generic/int-ll64.h" 1 3 4
# 12 "/usr/include/asm-generic/int-ll64.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/asm/bitsperlong.h" 1 3 4
# 11 "/usr/include/x86_64-linux-gnu/asm/bitsperlong.h" 3 4
# 1 "/usr/include/asm-generic/bitsperlong.h" 1 3 4
# 12 "/usr/include/x86_64-linux-gnu/asm/bitsperlong.h" 2 3 4
# 13 "/usr/include/asm-generic/int-ll64.h" 2 3 4







typedef __signed__ char __s8;
typedef unsigned char __u8;

typedef __signed__ short __s16;
typedef unsigned short __u16;

typedef __signed__ int __s32;
typedef unsigned int __u32;


__extension__ typedef __signed__ long long __s64;
__extension__ typedef unsigned long long __u64;
# 8 "/usr/include/asm-generic/types.h" 2 3 4
# 2 "/usr/include/x86_64-linux-gnu/asm/types.h" 2 3 4
# 6 "/usr/include/linux/types.h" 2 3 4



# 1 "/usr/include/linux/posix_types.h" 1 3 4




# 1 "/usr/include/linux/stddef.h" 1 3 4
# 6 "/usr/include/linux/posix_types.h" 2 3 4
# 25 "/usr/include/linux/posix_types.h" 3 4
typedef struct {
 unsigned long fds_bits[1024 / (8 * sizeof(long))];
} __kernel_fd_set;


typedef void (*__kernel_sighandler_t)(int);


typedef int __kernel_key_t;
typedef int __kernel_mqd_t;

# 1 "/usr/include/x86_64-linux-gnu/asm/posix_types.h" 1 3 4






# 1 "/usr/include/x86_64-linux-gnu/asm/posix_types_64.h" 1 3 4
# 11 "/usr/include/x86_64-linux-gnu/asm/posix_types_64.h" 3 4
typedef unsigned short __kernel_old_uid_t;
typedef unsigned short __kernel_old_gid_t;


typedef unsigned long __kernel_old_dev_t;


# 1 "/usr/include/asm-generic/posix_types.h" 1 3 4
# 15 "/usr/include/asm-generic/posix_types.h" 3 4
typedef long __kernel_long_t;
typedef unsigned long __kernel_ulong_t;



typedef __kernel_ulong_t __kernel_ino_t;



typedef unsigned int __kernel_mode_t;



typedef int __kernel_pid_t;



typedef int __kernel_ipc_pid_t;



typedef unsigned int __kernel_uid_t;
typedef unsigned int __kernel_gid_t;



typedef __kernel_long_t __kernel_suseconds_t;



typedef int __kernel_daddr_t;



typedef unsigned int __kernel_uid32_t;
typedef unsigned int __kernel_gid32_t;
# 72 "/usr/include/asm-generic/posix_types.h" 3 4
typedef __kernel_ulong_t __kernel_size_t;
typedef __kernel_long_t __kernel_ssize_t;
typedef __kernel_long_t __kernel_ptrdiff_t;




typedef struct {
 int val[2];
} __kernel_fsid_t;





typedef __kernel_long_t __kernel_off_t;
typedef long long __kernel_loff_t;
typedef __kernel_long_t __kernel_old_time_t;
typedef __kernel_long_t __kernel_time_t;
typedef long long __kernel_time64_t;
typedef __kernel_long_t __kernel_clock_t;
typedef int __kernel_timer_t;
typedef int __kernel_clockid_t;
typedef char * __kernel_caddr_t;
typedef unsigned short __kernel_uid16_t;
typedef unsigned short __kernel_gid16_t;
# 19 "/usr/include/x86_64-linux-gnu/asm/posix_types_64.h" 2 3 4
# 8 "/usr/include/x86_64-linux-gnu/asm/posix_types.h" 2 3 4
# 37 "/usr/include/linux/posix_types.h" 2 3 4
# 10 "/usr/include/linux/types.h" 2 3 4


typedef __signed__ __int128 __s128 __attribute__((aligned(16)));
typedef unsigned __int128 __u128 __attribute__((aligned(16)));
# 31 "/usr/include/linux/types.h" 3 4
typedef __u16 __le16;
typedef __u16 __be16;
typedef __u32 __le32;
typedef __u32 __be32;
typedef __u64 __le64;
typedef __u64 __be64;

typedef __u16 __sum16;
typedef __u32 __wsum;
# 55 "/usr/include/linux/types.h" 3 4
typedef unsigned __poll_t;
# 6 "/usr/include/linux/stat.h" 2 3 4
# 56 "/usr/include/linux/stat.h" 3 4
struct statx_timestamp {
 __s64 tv_sec;
 __u32 tv_nsec;
 __s32 __reserved;
};
# 99 "/usr/include/linux/stat.h" 3 4
struct statx {


 __u32 stx_mask;


 __u32 stx_blksize;


 __u64 stx_attributes;



 __u32 stx_nlink;


 __u32 stx_uid;


 __u32 stx_gid;


 __u16 stx_mode;
 __u16 __spare0[1];



 __u64 stx_ino;


 __u64 stx_size;


 __u64 stx_blocks;


 __u64 stx_attributes_mask;



 struct statx_timestamp stx_atime;


 struct statx_timestamp stx_btime;


 struct statx_timestamp stx_ctime;


 struct statx_timestamp stx_mtime;



 __u32 stx_rdev_major;
 __u32 stx_rdev_minor;


 __u32 stx_dev_major;
 __u32 stx_dev_minor;


 __u64 stx_mnt_id;


 __u32 stx_dio_mem_align;


 __u32 stx_dio_offset_align;



 __u64 stx_subvol;


 __u32 stx_atomic_write_unit_min;


 __u32 stx_atomic_write_unit_max;



 __u32 stx_atomic_write_segments_max;


 __u32 stx_dio_read_offset_align;


 __u64 __spare3[9];


};
# 32 "/usr/include/x86_64-linux-gnu/bits/statx.h" 2 3 4







# 1 "/usr/include/x86_64-linux-gnu/bits/statx-generic.h" 1 3 4
# 25 "/usr/include/x86_64-linux-gnu/bits/statx-generic.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/types/struct_statx_timestamp.h" 1 3 4
# 26 "/usr/include/x86_64-linux-gnu/bits/statx-generic.h" 2 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/types/struct_statx.h" 1 3 4
# 27 "/usr/include/x86_64-linux-gnu/bits/statx-generic.h" 2 3 4
# 62 "/usr/include/x86_64-linux-gnu/bits/statx-generic.h" 3 4
extern "C" {


int statx (int __dirfd, const char *__restrict __path, int __flags,
           unsigned int __mask, struct statx *__restrict __buf)
  noexcept (true) __attribute__ ((__nonnull__ (5)));

}
# 40 "/usr/include/x86_64-linux-gnu/bits/statx.h" 2 3 4
# 466 "/usr/include/x86_64-linux-gnu/sys/stat.h" 2 3 4


}
# 91 "../../bochs.h" 2

# 1 "/usr/include/ctype.h" 1 3 4
# 28 "/usr/include/ctype.h" 3 4
extern "C" {
# 46 "/usr/include/ctype.h" 3 4
enum
{
  _ISupper = ((0) < 8 ? ((1 << (0)) << 8) : ((1 << (0)) >> 8)),
  _ISlower = ((1) < 8 ? ((1 << (1)) << 8) : ((1 << (1)) >> 8)),
  _ISalpha = ((2) < 8 ? ((1 << (2)) << 8) : ((1 << (2)) >> 8)),
  _ISdigit = ((3) < 8 ? ((1 << (3)) << 8) : ((1 << (3)) >> 8)),
  _ISxdigit = ((4) < 8 ? ((1 << (4)) << 8) : ((1 << (4)) >> 8)),
  _ISspace = ((5) < 8 ? ((1 << (5)) << 8) : ((1 << (5)) >> 8)),
  _ISprint = ((6) < 8 ? ((1 << (6)) << 8) : ((1 << (6)) >> 8)),
  _ISgraph = ((7) < 8 ? ((1 << (7)) << 8) : ((1 << (7)) >> 8)),
  _ISblank = ((8) < 8 ? ((1 << (8)) << 8) : ((1 << (8)) >> 8)),
  _IScntrl = ((9) < 8 ? ((1 << (9)) << 8) : ((1 << (9)) >> 8)),
  _ISpunct = ((10) < 8 ? ((1 << (10)) << 8) : ((1 << (10)) >> 8)),
  _ISalnum = ((11) < 8 ? ((1 << (11)) << 8) : ((1 << (11)) >> 8))
};
# 79 "/usr/include/ctype.h" 3 4
extern const unsigned short int **__ctype_b_loc (void)
     noexcept (true) __attribute__ ((__const__));
extern const __int32_t **__ctype_tolower_loc (void)
     noexcept (true) __attribute__ ((__const__));
extern const __int32_t **__ctype_toupper_loc (void)
     noexcept (true) __attribute__ ((__const__));
# 108 "/usr/include/ctype.h" 3 4
extern int isalnum (int) noexcept (true);
extern int isalpha (int) noexcept (true);
extern int iscntrl (int) noexcept (true);
extern int isdigit (int) noexcept (true);
extern int islower (int) noexcept (true);
extern int isgraph (int) noexcept (true);
extern int isprint (int) noexcept (true);
extern int ispunct (int) noexcept (true);
extern int isspace (int) noexcept (true);
extern int isupper (int) noexcept (true);
extern int isxdigit (int) noexcept (true);



extern int tolower (int __c) noexcept (true);


extern int toupper (int __c) noexcept (true);




extern int isblank (int) noexcept (true);




extern int isctype (int __c, int __mask) noexcept (true);






extern int isascii (int __c) noexcept (true);



extern int toascii (int __c) noexcept (true);



extern int _toupper (int) noexcept (true);
extern int _tolower (int) noexcept (true);
# 251 "/usr/include/ctype.h" 3 4
extern int isalnum_l (int, locale_t) noexcept (true);
extern int isalpha_l (int, locale_t) noexcept (true);
extern int iscntrl_l (int, locale_t) noexcept (true);
extern int isdigit_l (int, locale_t) noexcept (true);
extern int islower_l (int, locale_t) noexcept (true);
extern int isgraph_l (int, locale_t) noexcept (true);
extern int isprint_l (int, locale_t) noexcept (true);
extern int ispunct_l (int, locale_t) noexcept (true);
extern int isspace_l (int, locale_t) noexcept (true);
extern int isupper_l (int, locale_t) noexcept (true);
extern int isxdigit_l (int, locale_t) noexcept (true);

extern int isblank_l (int, locale_t) noexcept (true);



extern int __tolower_l (int __c, locale_t __l) noexcept (true);
extern int tolower_l (int __c, locale_t __l) noexcept (true);


extern int __toupper_l (int __c, locale_t __l) noexcept (true);
extern int toupper_l (int __c, locale_t __l) noexcept (true);
# 327 "/usr/include/ctype.h" 3 4
}
# 93 "../../bochs.h" 2
# 1 "/usr/include/string.h" 1 3 4
# 26 "/usr/include/string.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/libc-header-start.h" 1 3 4
# 27 "/usr/include/string.h" 2 3 4

extern "C" {




# 1 "/usr/lib/gcc/x86_64-linux-gnu/14/include/stddef.h" 1 3 4
# 34 "/usr/include/string.h" 2 3 4
# 43 "/usr/include/string.h" 3 4
extern void *memcpy (void *__restrict __dest, const void *__restrict __src,
       size_t __n) noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern void *memmove (void *__dest, const void *__src, size_t __n)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));





extern void *memccpy (void *__restrict __dest, const void *__restrict __src,
        int __c, size_t __n)
    noexcept (true) __attribute__ ((__nonnull__ (1, 2))) __attribute__ ((__access__ (__write_only__, 1, 4)));




extern void *memset (void *__s, int __c, size_t __n) noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int memcmp (const void *__s1, const void *__s2, size_t __n)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
# 80 "/usr/include/string.h" 3 4
extern int __memcmpeq (const void *__s1, const void *__s2, size_t __n)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));



extern "C++"
{
extern void *memchr (void *__s, int __c, size_t __n)
      noexcept (true) __asm ("memchr") __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
extern const void *memchr (const void *__s, int __c, size_t __n)
      noexcept (true) __asm ("memchr") __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));


extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) void *
memchr (void *__s, int __c, size_t __n) noexcept (true)
{
  return __builtin_memchr (__s, __c, __n);
}

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) const void *
memchr (const void *__s, int __c, size_t __n) noexcept (true)
{
  return __builtin_memchr (__s, __c, __n);
}

}
# 115 "/usr/include/string.h" 3 4
extern "C++" void *rawmemchr (void *__s, int __c)
     noexcept (true) __asm ("rawmemchr") __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
extern "C++" const void *rawmemchr (const void *__s, int __c)
     noexcept (true) __asm ("rawmemchr") __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));







extern "C++" void *memrchr (void *__s, int __c, size_t __n)
      noexcept (true) __asm ("memrchr") __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)))
      __attribute__ ((__access__ (__read_only__, 1, 3)));
extern "C++" const void *memrchr (const void *__s, int __c, size_t __n)
      noexcept (true) __asm ("memrchr") __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)))
      __attribute__ ((__access__ (__read_only__, 1, 3)));
# 141 "/usr/include/string.h" 3 4
extern char *strcpy (char *__restrict __dest, const char *__restrict __src)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));

extern char *strncpy (char *__restrict __dest,
        const char *__restrict __src, size_t __n)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern char *strcat (char *__restrict __dest, const char *__restrict __src)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));

extern char *strncat (char *__restrict __dest, const char *__restrict __src,
        size_t __n) noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern int strcmp (const char *__s1, const char *__s2)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));

extern int strncmp (const char *__s1, const char *__s2, size_t __n)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));


extern int strcoll (const char *__s1, const char *__s2)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));

extern size_t strxfrm (char *__restrict __dest,
         const char *__restrict __src, size_t __n)
    noexcept (true) __attribute__ ((__nonnull__ (2))) __attribute__ ((__access__ (__write_only__, 1, 3)));






extern int strcoll_l (const char *__s1, const char *__s2, locale_t __l)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2, 3)));


extern size_t strxfrm_l (char *__dest, const char *__src, size_t __n,
    locale_t __l) noexcept (true) __attribute__ ((__nonnull__ (2, 4)))
     __attribute__ ((__access__ (__write_only__, 1, 3)));





extern char *strdup (const char *__s)
     noexcept (true) __attribute__ ((__malloc__)) __attribute__ ((__nonnull__ (1)));






extern char *strndup (const char *__string, size_t __n)
     noexcept (true) __attribute__ ((__malloc__)) __attribute__ ((__nonnull__ (1)));
# 224 "/usr/include/string.h" 3 4
extern "C++"
{
extern char *strchr (char *__s, int __c)
     noexcept (true) __asm ("strchr") __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
extern const char *strchr (const char *__s, int __c)
     noexcept (true) __asm ("strchr") __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));


extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) char *
strchr (char *__s, int __c) noexcept (true)
{
  return __builtin_strchr (__s, __c);
}

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) const char *
strchr (const char *__s, int __c) noexcept (true)
{
  return __builtin_strchr (__s, __c);
}

}






extern "C++"
{
extern char *strrchr (char *__s, int __c)
     noexcept (true) __asm ("strrchr") __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
extern const char *strrchr (const char *__s, int __c)
     noexcept (true) __asm ("strrchr") __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));


extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) char *
strrchr (char *__s, int __c) noexcept (true)
{
  return __builtin_strrchr (__s, __c);
}

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) const char *
strrchr (const char *__s, int __c) noexcept (true)
{
  return __builtin_strrchr (__s, __c);
}

}
# 281 "/usr/include/string.h" 3 4
extern "C++" char *strchrnul (char *__s, int __c)
     noexcept (true) __asm ("strchrnul") __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
extern "C++" const char *strchrnul (const char *__s, int __c)
     noexcept (true) __asm ("strchrnul") __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
# 293 "/usr/include/string.h" 3 4
extern size_t strcspn (const char *__s, const char *__reject)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));


extern size_t strspn (const char *__s, const char *__accept)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));


extern "C++"
{
extern char *strpbrk (char *__s, const char *__accept)
     noexcept (true) __asm ("strpbrk") __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
extern const char *strpbrk (const char *__s, const char *__accept)
     noexcept (true) __asm ("strpbrk") __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));


extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) char *
strpbrk (char *__s, const char *__accept) noexcept (true)
{
  return __builtin_strpbrk (__s, __accept);
}

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) const char *
strpbrk (const char *__s, const char *__accept) noexcept (true)
{
  return __builtin_strpbrk (__s, __accept);
}

}






extern "C++"
{
extern char *strstr (char *__haystack, const char *__needle)
     noexcept (true) __asm ("strstr") __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
extern const char *strstr (const char *__haystack, const char *__needle)
     noexcept (true) __asm ("strstr") __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));


extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) char *
strstr (char *__haystack, const char *__needle) noexcept (true)
{
  return __builtin_strstr (__haystack, __needle);
}

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) const char *
strstr (const char *__haystack, const char *__needle) noexcept (true)
{
  return __builtin_strstr (__haystack, __needle);
}

}







extern char *strtok (char *__restrict __s, const char *__restrict __delim)
     noexcept (true) __attribute__ ((__nonnull__ (2)));



extern char *__strtok_r (char *__restrict __s,
    const char *__restrict __delim,
    char **__restrict __save_ptr)
     noexcept (true) __attribute__ ((__nonnull__ (2, 3)));

extern char *strtok_r (char *__restrict __s, const char *__restrict __delim,
         char **__restrict __save_ptr)
     noexcept (true) __attribute__ ((__nonnull__ (2, 3)));





extern "C++" char *strcasestr (char *__haystack, const char *__needle)
     noexcept (true) __asm ("strcasestr") __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
extern "C++" const char *strcasestr (const char *__haystack,
         const char *__needle)
     noexcept (true) __asm ("strcasestr") __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
# 389 "/usr/include/string.h" 3 4
extern void *memmem (const void *__haystack, size_t __haystacklen,
       const void *__needle, size_t __needlelen)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 3)))
    __attribute__ ((__access__ (__read_only__, 1, 2)))
    __attribute__ ((__access__ (__read_only__, 3, 4)));



extern void *__mempcpy (void *__restrict __dest,
   const void *__restrict __src, size_t __n)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));
extern void *mempcpy (void *__restrict __dest,
        const void *__restrict __src, size_t __n)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));




extern size_t strlen (const char *__s)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));




extern size_t strnlen (const char *__string, size_t __maxlen)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));




extern char *strerror (int __errnum) noexcept (true);
# 444 "/usr/include/string.h" 3 4
extern char *strerror_r (int __errnum, char *__buf, size_t __buflen)
     noexcept (true) __attribute__ ((__nonnull__ (2))) __attribute__ ((__warn_unused_result__)) __attribute__ ((__access__ (__write_only__, 2, 3)));




extern const char *strerrordesc_np (int __err) noexcept (true);

extern const char *strerrorname_np (int __err) noexcept (true);





extern char *strerror_l (int __errnum, locale_t __l) noexcept (true);



# 1 "/usr/include/strings.h" 1 3 4
# 23 "/usr/include/strings.h" 3 4
# 1 "/usr/lib/gcc/x86_64-linux-gnu/14/include/stddef.h" 1 3 4
# 24 "/usr/include/strings.h" 2 3 4






extern "C" {



extern int bcmp (const void *__s1, const void *__s2, size_t __n)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));


extern void bcopy (const void *__src, void *__dest, size_t __n)
  noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern void bzero (void *__s, size_t __n) noexcept (true) __attribute__ ((__nonnull__ (1)));



extern "C++"
{
extern char *index (char *__s, int __c)
     noexcept (true) __asm ("index") __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
extern const char *index (const char *__s, int __c)
     noexcept (true) __asm ("index") __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));


extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) char *
index (char *__s, int __c) noexcept (true)
{
  return __builtin_index (__s, __c);
}

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) const char *
index (const char *__s, int __c) noexcept (true)
{
  return __builtin_index (__s, __c);
}

}







extern "C++"
{
extern char *rindex (char *__s, int __c)
     noexcept (true) __asm ("rindex") __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
extern const char *rindex (const char *__s, int __c)
     noexcept (true) __asm ("rindex") __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));


extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) char *
rindex (char *__s, int __c) noexcept (true)
{
  return __builtin_rindex (__s, __c);
}

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) const char *
rindex (const char *__s, int __c) noexcept (true)
{
  return __builtin_rindex (__s, __c);
}

}
# 104 "/usr/include/strings.h" 3 4
extern int ffs (int __i) noexcept (true) __attribute__ ((__const__));





extern int ffsl (long int __l) noexcept (true) __attribute__ ((__const__));
__extension__ extern int ffsll (long long int __ll)
     noexcept (true) __attribute__ ((__const__));



extern int strcasecmp (const char *__s1, const char *__s2)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));


extern int strncasecmp (const char *__s1, const char *__s2, size_t __n)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));






extern int strcasecmp_l (const char *__s1, const char *__s2, locale_t __loc)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2, 3)));



extern int strncasecmp_l (const char *__s1, const char *__s2,
     size_t __n, locale_t __loc)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2, 4)));


}





# 1 "/usr/include/x86_64-linux-gnu/bits/strings_fortified.h" 1 3 4
# 22 "/usr/include/x86_64-linux-gnu/bits/strings_fortified.h" 3 4
extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) void
__attribute__ ((__leaf__)) bcopy (const void *__src, void *__dest, size_t __len) noexcept (true)
{
  (void) __builtin___memmove_chk (__dest, __src, __len,
      __builtin_dynamic_object_size (__dest, 0));
}

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) void
__attribute__ ((__leaf__)) bzero (void *__dest, size_t __len) noexcept (true)
{
  (void) __builtin___memset_chk (__dest, '\0', __len,
     __builtin_dynamic_object_size (__dest, 0));
}
# 145 "/usr/include/strings.h" 2 3 4
# 463 "/usr/include/string.h" 2 3 4



extern void explicit_bzero (void *__s, size_t __n) noexcept (true) __attribute__ ((__nonnull__ (1)))
    ;



extern char *strsep (char **__restrict __stringp,
       const char *__restrict __delim)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));




extern char *strsignal (int __sig) noexcept (true);



extern const char *sigabbrev_np (int __sig) noexcept (true);


extern const char *sigdescr_np (int __sig) noexcept (true);



extern char *__stpcpy (char *__restrict __dest, const char *__restrict __src)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));
extern char *stpcpy (char *__restrict __dest, const char *__restrict __src)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));



extern char *__stpncpy (char *__restrict __dest,
   const char *__restrict __src, size_t __n)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));
extern char *stpncpy (char *__restrict __dest,
        const char *__restrict __src, size_t __n)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));




extern size_t strlcpy (char *__restrict __dest,
         const char *__restrict __src, size_t __n)
  noexcept (true) __attribute__ ((__nonnull__ (1, 2))) __attribute__ ((__access__ (__write_only__, 1, 3)));



extern size_t strlcat (char *__restrict __dest,
         const char *__restrict __src, size_t __n)
  noexcept (true) __attribute__ ((__nonnull__ (1, 2))) __attribute__ ((__access__ (__read_write__, 1, 3)));




extern int strverscmp (const char *__s1, const char *__s2)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));


extern char *strfry (char *__string) noexcept (true) __attribute__ ((__nonnull__ (1)));


extern void *memfrob (void *__s, size_t __n) noexcept (true) __attribute__ ((__nonnull__ (1)))
    __attribute__ ((__access__ (__read_write__, 1, 2)));







extern "C++" char *basename (char *__filename)
     noexcept (true) __asm ("basename") __attribute__ ((__nonnull__ (1)));
extern "C++" const char *basename (const char *__filename)
     noexcept (true) __asm ("basename") __attribute__ ((__nonnull__ (1)));
# 548 "/usr/include/string.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/string_fortified.h" 1 3 4
# 25 "/usr/include/x86_64-linux-gnu/bits/string_fortified.h" 3 4
extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) void *
__attribute__ ((__leaf__)) memcpy (void *__restrict __dest, const void *__restrict __src, size_t __len) noexcept (true)

{
  return __builtin___memcpy_chk (__dest, __src, __len,
     __builtin_dynamic_object_size (__dest, 0));
}

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) void *
__attribute__ ((__leaf__)) memmove (void *__dest, const void *__src, size_t __len) noexcept (true)
{
  return __builtin___memmove_chk (__dest, __src, __len,
      __builtin_dynamic_object_size (__dest, 0));
}


extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) void *
__attribute__ ((__leaf__)) mempcpy (void *__restrict __dest, const void *__restrict __src, size_t __len) noexcept (true)

{
  return __builtin___mempcpy_chk (__dest, __src, __len,
      __builtin_dynamic_object_size (__dest, 0));
}
# 56 "/usr/include/x86_64-linux-gnu/bits/string_fortified.h" 3 4
extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) void *
__attribute__ ((__leaf__)) memset (void *__dest, int __ch, size_t __len) noexcept (true)
{
  return __builtin___memset_chk (__dest, __ch, __len,
     __builtin_dynamic_object_size (__dest, 0));
}




void __explicit_bzero_chk (void *__dest, size_t __len, size_t __destlen)
  noexcept (true) __attribute__ ((__nonnull__ (1))) ;

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) void
__attribute__ ((__leaf__)) explicit_bzero (void *__dest, size_t __len) noexcept (true)
{
  __explicit_bzero_chk (__dest, __len, __builtin_dynamic_object_size (__dest, 0));
}


extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) char *
__attribute__ ((__leaf__)) strcpy (char * __restrict __dest, const char *__restrict __src) noexcept (true)

    
{
  return __builtin___strcpy_chk (__dest, __src, __builtin_dynamic_object_size (__dest, 1));
}


extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) char *
__attribute__ ((__leaf__)) stpcpy (char * __restrict __dest, const char *__restrict __src) noexcept (true)

    
{
  return __builtin___stpcpy_chk (__dest, __src, __builtin_dynamic_object_size (__dest, 1));
}



extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) char *
__attribute__ ((__leaf__)) strncpy (char * __restrict __dest, const char *__restrict __src, size_t __len) noexcept (true)

    
{
  return __builtin___strncpy_chk (__dest, __src, __len,
      __builtin_dynamic_object_size (__dest, 1));
}



extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) char *
__attribute__ ((__leaf__)) stpncpy (char * __dest, const char *__src, size_t __n) noexcept (true)

    
{
  return __builtin___stpncpy_chk (__dest, __src, __n,
      __builtin_dynamic_object_size (__dest, 1));
}
# 135 "/usr/include/x86_64-linux-gnu/bits/string_fortified.h" 3 4
extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) char *
__attribute__ ((__leaf__)) strcat (char * __restrict __dest, const char *__restrict __src) noexcept (true)

    
{
  return __builtin___strcat_chk (__dest, __src, __builtin_dynamic_object_size (__dest, 1));
}


extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) char *
__attribute__ ((__leaf__)) strncat (char * __restrict __dest, const char *__restrict __src, size_t __len) noexcept (true)

    
{
  return __builtin___strncat_chk (__dest, __src, __len,
      __builtin_dynamic_object_size (__dest, 1));
}


extern size_t __strlcpy_chk (char *__dest, const char *__src, size_t __n,
        size_t __destlen) noexcept (true);
extern size_t __strlcpy_alias (char *__dest, const char *__src, size_t __n) noexcept (true) __asm__ ("" "strlcpy")

                 ;

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) size_t
__attribute__ ((__leaf__)) strlcpy (char * __restrict __dest, const char *__restrict __src, size_t __n) noexcept (true)

    
{
  if (__builtin_dynamic_object_size (__dest, 1) != (size_t) -1
      && (!__builtin_constant_p (__n > __builtin_dynamic_object_size (__dest, 1))
   || __n > __builtin_dynamic_object_size (__dest, 1)))
    return __strlcpy_chk (__dest, __src, __n, __builtin_dynamic_object_size (__dest, 1));
  return __strlcpy_alias (__dest, __src, __n);
}

extern size_t __strlcat_chk (char *__dest, const char *__src, size_t __n,
        size_t __destlen) noexcept (true);
extern size_t __strlcat_alias (char *__dest, const char *__src, size_t __n) noexcept (true) __asm__ ("" "strlcat")

                 ;

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) size_t
__attribute__ ((__leaf__)) strlcat (char * __restrict __dest, const char *__restrict __src, size_t __n) noexcept (true)

{
  if (__builtin_dynamic_object_size (__dest, 1) != (size_t) -1
      && (!__builtin_constant_p (__n > __builtin_dynamic_object_size (__dest, 1))
   || __n > __builtin_dynamic_object_size (__dest, 1)))
    return __strlcat_chk (__dest, __src, __n, __builtin_dynamic_object_size (__dest, 1));
  return __strlcat_alias (__dest, __src, __n);
}
# 549 "/usr/include/string.h" 2 3 4



}
# 94 "../../bochs.h" 2
# 1 "/usr/include/fcntl.h" 1 3 4
# 28 "/usr/include/fcntl.h" 3 4
extern "C" {






# 1 "/usr/include/x86_64-linux-gnu/bits/fcntl.h" 1 3 4
# 35 "/usr/include/x86_64-linux-gnu/bits/fcntl.h" 3 4
struct flock
  {
    short int l_type;
    short int l_whence;




    __off64_t l_start;
    __off64_t l_len;

    __pid_t l_pid;
  };


struct flock64
  {
    short int l_type;
    short int l_whence;
    __off64_t l_start;
    __off64_t l_len;
    __pid_t l_pid;
  };



# 1 "/usr/include/x86_64-linux-gnu/bits/fcntl-linux.h" 1 3 4
# 38 "/usr/include/x86_64-linux-gnu/bits/fcntl-linux.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/types/struct_iovec.h" 1 3 4
# 23 "/usr/include/x86_64-linux-gnu/bits/types/struct_iovec.h" 3 4
# 1 "/usr/lib/gcc/x86_64-linux-gnu/14/include/stddef.h" 1 3 4
# 24 "/usr/include/x86_64-linux-gnu/bits/types/struct_iovec.h" 2 3 4


struct iovec
  {
    void *iov_base;
    size_t iov_len;
  };
# 39 "/usr/include/x86_64-linux-gnu/bits/fcntl-linux.h" 2 3 4
# 267 "/usr/include/x86_64-linux-gnu/bits/fcntl-linux.h" 3 4
enum __pid_type
  {
    F_OWNER_TID = 0,
    F_OWNER_PID,
    F_OWNER_PGRP,
    F_OWNER_GID = F_OWNER_PGRP
  };


struct f_owner_ex
  {
    enum __pid_type type;
    __pid_t pid;
  };
# 357 "/usr/include/x86_64-linux-gnu/bits/fcntl-linux.h" 3 4
# 1 "/usr/include/linux/falloc.h" 1 3 4
# 358 "/usr/include/x86_64-linux-gnu/bits/fcntl-linux.h" 2 3 4



struct file_handle
{
  unsigned int handle_bytes;
  int handle_type;

  unsigned char f_handle[0];
};
# 384 "/usr/include/x86_64-linux-gnu/bits/fcntl-linux.h" 3 4
extern "C" {




extern __ssize_t readahead (int __fd, __off64_t __offset, size_t __count)
    noexcept (true);






extern int sync_file_range (int __fd, __off64_t __offset, __off64_t __count,
       unsigned int __flags);






extern __ssize_t vmsplice (int __fdout, const struct iovec *__iov,
      size_t __count, unsigned int __flags);





extern __ssize_t splice (int __fdin, __off64_t *__offin, int __fdout,
    __off64_t *__offout, size_t __len,
    unsigned int __flags);





extern __ssize_t tee (int __fdin, int __fdout, size_t __len,
        unsigned int __flags);
# 431 "/usr/include/x86_64-linux-gnu/bits/fcntl-linux.h" 3 4
extern int fallocate (int __fd, int __mode, __off64_t __offset, __off64_t __len) __asm__ ("" "fallocate64")

                     ;





extern int fallocate64 (int __fd, int __mode, __off64_t __offset,
   __off64_t __len);




extern int name_to_handle_at (int __dfd, const char *__name,
         struct file_handle *__handle, int *__mnt_id,
         int __flags) noexcept (true);





extern int open_by_handle_at (int __mountdirfd, struct file_handle *__handle,
         int __flags);



}
# 62 "/usr/include/x86_64-linux-gnu/bits/fcntl.h" 2 3 4
# 36 "/usr/include/fcntl.h" 2 3 4
# 78 "/usr/include/fcntl.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/stat.h" 1 3 4
# 79 "/usr/include/fcntl.h" 2 3 4
# 180 "/usr/include/fcntl.h" 3 4
extern int fcntl (int __fd, int __cmd, ...) __asm__ ("" "fcntl64");





extern int fcntl64 (int __fd, int __cmd, ...);
# 212 "/usr/include/fcntl.h" 3 4
extern int open (const char *__file, int __oflag, ...) __asm__ ("" "open64")
     __attribute__ ((__nonnull__ (1)));





extern int open64 (const char *__file, int __oflag, ...) __attribute__ ((__nonnull__ (1)));
# 237 "/usr/include/fcntl.h" 3 4
extern int openat (int __fd, const char *__file, int __oflag, ...) __asm__ ("" "openat64")
                    __attribute__ ((__nonnull__ (2)));





extern int openat64 (int __fd, const char *__file, int __oflag, ...)
     __attribute__ ((__nonnull__ (2)));
# 258 "/usr/include/fcntl.h" 3 4
extern int creat (const char *__file, mode_t __mode) __asm__ ("" "creat64")
                  __attribute__ ((__nonnull__ (1)));





extern int creat64 (const char *__file, mode_t __mode) __attribute__ ((__nonnull__ (1)));
# 305 "/usr/include/fcntl.h" 3 4
extern int posix_fadvise (int __fd, __off64_t __offset, __off64_t __len, int __advise) noexcept (true) __asm__ ("" "posix_fadvise64")

                      ;





extern int posix_fadvise64 (int __fd, off64_t __offset, off64_t __len,
       int __advise) noexcept (true);
# 326 "/usr/include/fcntl.h" 3 4
extern int posix_fallocate (int __fd, __off64_t __offset, __off64_t __len) __asm__ ("" "posix_fallocate64")

                           ;





extern int posix_fallocate64 (int __fd, off64_t __offset, off64_t __len);






# 1 "/usr/include/x86_64-linux-gnu/bits/fcntl2.h" 1 3 4
# 30 "/usr/include/x86_64-linux-gnu/bits/fcntl2.h" 3 4
extern int __open_2 (const char *__path, int __oflag) __asm__ ("" "__open64_2")
                     __attribute__ ((__nonnull__ (1)));
extern int __open_alias (const char *__path, int __oflag, ...) __asm__ ("" "open64")
                 __attribute__ ((__nonnull__ (1)));



extern void __open_too_many_args (void) __attribute__((__error__ ("open can be called either with 2 or 3 arguments, not more")))
                                                                  ;
extern void __open_missing_mode (void) __attribute__((__error__ ("open with O_CREAT or O_TMPFILE in second argument needs 3 arguments")))
                                                                            ;

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) int
open (const char *__path, int __oflag, ...)
{
  if (__builtin_va_arg_pack_len () > 1)
    __open_too_many_args ();

  if (__builtin_constant_p (__oflag))
    {
      if ((((__oflag) & 0100) != 0 || ((__oflag) & (020000000 | 0200000)) == (020000000 | 0200000)) && __builtin_va_arg_pack_len () < 1)
 {
   __open_missing_mode ();
   return __open_2 (__path, __oflag);
 }
      return __open_alias (__path, __oflag, __builtin_va_arg_pack ());
    }

  if (__builtin_va_arg_pack_len () < 1)
    return __open_2 (__path, __oflag);

  return __open_alias (__path, __oflag, __builtin_va_arg_pack ());
}
# 82 "/usr/include/x86_64-linux-gnu/bits/fcntl2.h" 3 4
extern int __open64_2 (const char *__path, int __oflag) __attribute__ ((__nonnull__ (1)));
extern int __open64_alias (const char *__path, int __oflag, ...) __asm__ ("" "open64")
                   __attribute__ ((__nonnull__ (1)));

extern void __open64_too_many_args (void) __attribute__((__error__ ("open64 can be called either with 2 or 3 arguments, not more")))
                                                                    ;
extern void __open64_missing_mode (void) __attribute__((__error__ ("open64 with O_CREAT or O_TMPFILE in second argument needs 3 arguments")))
                                                                              ;

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) int
open64 (const char *__path, int __oflag, ...)
{
  if (__builtin_va_arg_pack_len () > 1)
    __open64_too_many_args ();

  if (__builtin_constant_p (__oflag))
    {
      if ((((__oflag) & 0100) != 0 || ((__oflag) & (020000000 | 0200000)) == (020000000 | 0200000)) && __builtin_va_arg_pack_len () < 1)
 {
   __open64_missing_mode ();
   return __open64_2 (__path, __oflag);
 }
      return __open64_alias (__path, __oflag, __builtin_va_arg_pack ());
    }

  if (__builtin_va_arg_pack_len () < 1)
    return __open64_2 (__path, __oflag);

  return __open64_alias (__path, __oflag, __builtin_va_arg_pack ());
}
# 144 "/usr/include/x86_64-linux-gnu/bits/fcntl2.h" 3 4
extern int __openat_2 (int __fd, const char *__path, int __oflag) __asm__ ("" "__openat64_2")

     __attribute__ ((__nonnull__ (2)));
extern int __openat_alias (int __fd, const char *__path, int __oflag, ...) __asm__ ("" "openat64")

     __attribute__ ((__nonnull__ (2)));



extern void __openat_too_many_args (void) __attribute__((__error__ ("openat can be called either with 3 or 4 arguments, not more")))
                                                                    ;
extern void __openat_missing_mode (void) __attribute__((__error__ ("openat with O_CREAT or O_TMPFILE in third argument needs 4 arguments")))
                                                                             ;

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) int
openat (int __fd, const char *__path, int __oflag, ...)
{
  if (__builtin_va_arg_pack_len () > 1)
    __openat_too_many_args ();

  if (__builtin_constant_p (__oflag))
    {
      if ((((__oflag) & 0100) != 0 || ((__oflag) & (020000000 | 0200000)) == (020000000 | 0200000)) && __builtin_va_arg_pack_len () < 1)
 {
   __openat_missing_mode ();
   return __openat_2 (__fd, __path, __oflag);
 }
      return __openat_alias (__fd, __path, __oflag, __builtin_va_arg_pack ());
    }

  if (__builtin_va_arg_pack_len () < 1)
    return __openat_2 (__fd, __path, __oflag);

  return __openat_alias (__fd, __path, __oflag, __builtin_va_arg_pack ());
}
# 204 "/usr/include/x86_64-linux-gnu/bits/fcntl2.h" 3 4
extern int __openat64_2 (int __fd, const char *__path, int __oflag)
     __attribute__ ((__nonnull__ (2)));
extern int __openat64_alias (int __fd, const char *__path, int __oflag, ...) __asm__ ("" "openat64")

     __attribute__ ((__nonnull__ (2)));
extern void __openat64_too_many_args (void) __attribute__((__error__ ("openat64 can be called either with 3 or 4 arguments, not more")))
                                                                      ;
extern void __openat64_missing_mode (void) __attribute__((__error__ ("openat64 with O_CREAT or O_TMPFILE in third argument needs 4 arguments")))
                                                                               ;


extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) int
openat64 (int __fd, const char *__path, int __oflag, ...)
{
  if (__builtin_va_arg_pack_len () > 1)
    __openat64_too_many_args ();

  if (__builtin_constant_p (__oflag))
    {
      if ((((__oflag) & 0100) != 0 || ((__oflag) & (020000000 | 0200000)) == (020000000 | 0200000)) && __builtin_va_arg_pack_len () < 1)
 {
   __openat64_missing_mode ();
   return __openat64_2 (__fd, __path, __oflag);
 }
      return __openat64_alias (__fd, __path, __oflag, __builtin_va_arg_pack ());
    }

  if (__builtin_va_arg_pack_len () < 1)
    return __openat64_2 (__fd, __path, __oflag);

  return __openat64_alias (__fd, __path, __oflag, __builtin_va_arg_pack ());
}
# 342 "/usr/include/fcntl.h" 2 3 4


}
# 95 "../../bochs.h" 2
# 1 "/usr/lib/gcc/x86_64-linux-gnu/14/include/limits.h" 1 3 4
# 34 "/usr/lib/gcc/x86_64-linux-gnu/14/include/limits.h" 3 4
# 1 "/usr/lib/gcc/x86_64-linux-gnu/14/include/syslimits.h" 1 3 4






# 1 "/usr/lib/gcc/x86_64-linux-gnu/14/include/limits.h" 1 3 4
# 210 "/usr/lib/gcc/x86_64-linux-gnu/14/include/limits.h" 3 4
# 1 "/usr/include/limits.h" 1 3 4
# 26 "/usr/include/limits.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/libc-header-start.h" 1 3 4
# 27 "/usr/include/limits.h" 2 3 4
# 195 "/usr/include/limits.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/posix1_lim.h" 1 3 4
# 27 "/usr/include/x86_64-linux-gnu/bits/posix1_lim.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/wordsize.h" 1 3 4
# 28 "/usr/include/x86_64-linux-gnu/bits/posix1_lim.h" 2 3 4
# 161 "/usr/include/x86_64-linux-gnu/bits/posix1_lim.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/local_lim.h" 1 3 4
# 38 "/usr/include/x86_64-linux-gnu/bits/local_lim.h" 3 4
# 1 "/usr/include/linux/limits.h" 1 3 4
# 39 "/usr/include/x86_64-linux-gnu/bits/local_lim.h" 2 3 4
# 81 "/usr/include/x86_64-linux-gnu/bits/local_lim.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/pthread_stack_min-dynamic.h" 1 3 4
# 23 "/usr/include/x86_64-linux-gnu/bits/pthread_stack_min-dynamic.h" 3 4
extern "C" {
extern long int __sysconf (int __name) noexcept (true);
}
# 82 "/usr/include/x86_64-linux-gnu/bits/local_lim.h" 2 3 4
# 162 "/usr/include/x86_64-linux-gnu/bits/posix1_lim.h" 2 3 4
# 196 "/usr/include/limits.h" 2 3 4



# 1 "/usr/include/x86_64-linux-gnu/bits/posix2_lim.h" 1 3 4
# 200 "/usr/include/limits.h" 2 3 4



# 1 "/usr/include/x86_64-linux-gnu/bits/xopen_lim.h" 1 3 4
# 64 "/usr/include/x86_64-linux-gnu/bits/xopen_lim.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/uio_lim.h" 1 3 4
# 65 "/usr/include/x86_64-linux-gnu/bits/xopen_lim.h" 2 3 4
# 204 "/usr/include/limits.h" 2 3 4
# 211 "/usr/lib/gcc/x86_64-linux-gnu/14/include/limits.h" 2 3 4
# 8 "/usr/lib/gcc/x86_64-linux-gnu/14/include/syslimits.h" 2 3 4
# 35 "/usr/lib/gcc/x86_64-linux-gnu/14/include/limits.h" 2 3 4
# 96 "../../bochs.h" 2






# 101 "../../bochs.h"
}


# 1 "../../osdep.h" 1
# 38 "../../osdep.h"
extern "C" {
# 194 "../../osdep.h"
  extern char *bx_strrev(char *str);
# 348 "../../osdep.h"
 extern Bit64u bx_get_realtime64_usec (void);
 extern void bx_init_realtime64_usec (void);
# 363 "../../osdep.h"
}
# 105 "../../bochs.h" 2

class bx_param_c;
class bx_list_c;
# 122 "../../bochs.h"
int bx_begin_simulation(int argc, char *argv[]);
void bx_stop_simulation();
char *bx_find_bochsrc(void);
const char *get_builtin_variable(const char *varname);
void get_bxshare_path(char *path);
int bx_parse_cmdline(int arg, int argc, char *argv[]);
int bx_read_configuration(const char *rcfile);
int bx_write_configuration(const char *rcfile, int overwrite);
void bx_reset_options(void);
void bx_set_log_actions_by_device(bool panic_flag);







int bx_parse_param_from_list(const char *context, const char *input, bx_list_c *list);
int bx_parse_nic_params(const char *context, const char *param, bx_list_c *base);
int bx_parse_usb_port_params(const char *context, const char *param,
                              int maxports, bx_list_c *base);
int bx_split_option_list(const char *msg, const char *rawopt, char **argv, int max_argv);
int bx_write_param_list(FILE *fp, bx_list_c *base, const char *optname, bool multiline);




Bit32u crc32(const Bit8u *buf, int len);


void print_tree(bx_param_c *node, int level = 0, bool xml = false);



void print_statistics_tree(bx_param_c *node, int level = 0);
# 267 "../../bochs.h"
# 1 "../../logio.h" 1
# 27 "../../logio.h"
typedef enum {
  LOGLEV_DEBUG = 0,
  LOGLEV_INFO,
  LOGLEV_WARN,
  LOGLEV_ERROR,
  LOGLEV_PANIC,
  N_LOGLEV
} bx_log_levels;


typedef enum {
  ACT_IGNORE = 0,
  ACT_REPORT,
  ACT_WARN,
  ACT_ASK,
  ACT_FATAL,
  N_ACT
} bx_log_actions;

typedef class logfunctions
{
  char *name;
  char *prefix;
  int onoff[N_LOGLEV];
  class iofunctions *logio;


  static int default_onoff[N_LOGLEV];
public:
  logfunctions(void);
  logfunctions(class iofunctions *);
  virtual ~logfunctions(void);

  void ldebug(const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));
  void info(const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));
  void lwarn(const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));
  void error(const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));
  void panic(const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));
  void fatal1(const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));
  void fatal(int level, const char *prefix, const char *fmt, va_list ap, int exit_status);
  void warn(int level, const char *prefix, const char *fmt, va_list ap);
  void ask(int level, const char *prefix, const char *fmt, va_list ap);
  void put(const char *p);
  void put(const char *n, const char *p);
  void setio(class iofunctions *);
  void setonoff(int loglev, int value) {
    
# 73 "../../logio.h" 3 4
   (static_cast <bool> (
# 73 "../../logio.h"
   loglev >= 0 && loglev < N_LOGLEV
# 73 "../../logio.h" 3 4
   ) ? void (0) : __assert_fail (
# 73 "../../logio.h"
   "loglev >= 0 && loglev < N_LOGLEV"
# 73 "../../logio.h" 3 4
   , __builtin_FILE (), __builtin_LINE (), __extension__ __PRETTY_FUNCTION__))
# 73 "../../logio.h"
                                            ;
    onoff[loglev] = value;
  }
  const char *get_name() const { return name; }
  const char *getprefix() const { return prefix; }
  int getonoff(int level) const {
    
# 79 "../../logio.h" 3 4
   (static_cast <bool> (
# 79 "../../logio.h"
   level>=0 && level<N_LOGLEV
# 79 "../../logio.h" 3 4
   ) ? void (0) : __assert_fail (
# 79 "../../logio.h"
   "level>=0 && level<N_LOGLEV"
# 79 "../../logio.h" 3 4
   , __builtin_FILE (), __builtin_LINE (), __extension__ __PRETTY_FUNCTION__))
# 79 "../../logio.h"
                                      ;
    return onoff[level];
  }
  static void set_default_action(int loglev, int action) {
    
# 83 "../../logio.h" 3 4
   (static_cast <bool> (
# 83 "../../logio.h"
   loglev >= 0 && loglev < N_LOGLEV
# 83 "../../logio.h" 3 4
   ) ? void (0) : __assert_fail (
# 83 "../../logio.h"
   "loglev >= 0 && loglev < N_LOGLEV"
# 83 "../../logio.h" 3 4
   , __builtin_FILE (), __builtin_LINE (), __extension__ __PRETTY_FUNCTION__))
# 83 "../../logio.h"
                                            ;
    
# 84 "../../logio.h" 3 4
   (static_cast <bool> (
# 84 "../../logio.h"
   action >= 0 && action < N_ACT
# 84 "../../logio.h" 3 4
   ) ? void (0) : __assert_fail (
# 84 "../../logio.h"
   "action >= 0 && action < N_ACT"
# 84 "../../logio.h" 3 4
   , __builtin_FILE (), __builtin_LINE (), __extension__ __PRETTY_FUNCTION__))
# 84 "../../logio.h"
                                         ;
    default_onoff[loglev] = action;
  }
  static int get_default_action(int loglev) {
    
# 88 "../../logio.h" 3 4
   (static_cast <bool> (
# 88 "../../logio.h"
   loglev >= 0 && loglev < N_LOGLEV
# 88 "../../logio.h" 3 4
   ) ? void (0) : __assert_fail (
# 88 "../../logio.h"
   "loglev >= 0 && loglev < N_LOGLEV"
# 88 "../../logio.h" 3 4
   , __builtin_FILE (), __builtin_LINE (), __extension__ __PRETTY_FUNCTION__))
# 88 "../../logio.h"
                                            ;
    return default_onoff[loglev];
  }
} logfunc_t;



class iofunctions {
  int magic;
  char logprefix[20 + 1];
  FILE *logfd;
  class logfunctions *log;
  void init(void);
  void flush(void);


public:
  iofunctions(void);
  iofunctions(FILE *);
  iofunctions(int);
  iofunctions(const char *);
 ~iofunctions(void);

  void out(int level, const char *pre, const char *fmt, va_list ap);

  void init_log(const char *fn);
  void init_log(int fd);
  void init_log(FILE *fs);
  void exit_log();
  void exit_log2();
  void set_log_prefix(const char *prefix);
  int get_n_logfns() const { return n_logfn; }
  logfunc_t *get_logfn(int index) { return logfn_list[index]; }
  void add_logfn(logfunc_t *fn);
  void remove_logfn(logfunc_t *fn);
  void set_log_action(int loglevel, int action);
  const char *getlevel(int i) const;
  const char *getaction(int i) const;
  int isaction(const char *val) const;

protected:
  int n_logfn;

  logfunc_t *logfn_list[512];
  const char *logfn;
};

typedef class iofunctions iofunc_t;
# 170 "../../logio.h"
 extern iofunc_t *io;
 extern logfunc_t *genlog;
# 268 "../../bochs.h" 2
# 306 "../../bochs.h"
typedef struct {
  bool interrupts;
  bool exceptions;
  bool print_timestamps;
  bool debugger_active;
# 322 "../../bochs.h"
  bool apic;




} bx_debug_t;


 void bx_show_ips_handler(void);

void bx_signal_handler(int signum);
 int bx_atexit(void);
 extern bx_debug_t bx_dbg;
 void bx_exit(int errcode);







 extern Bit8u bx_cpu_count;


 extern Bit32u apic_id_mask;



enum {
  BX_READ = 0,
  BX_WRITE = 1,
  BX_EXECUTE = 2,
  BX_RW = 3,






};
# 379 "../../bochs.h"
enum {
  BX_KBD_XT_TYPE = 0,
  BX_KBD_AT_TYPE = 1,
  BX_KBD_MF_TYPE = 2
};







void bx_center_print(FILE *file, const char *line, unsigned maxwidth);

# 1 "../../misc/bswap.h" 1
# 25 "../../misc/bswap.h"
inline Bit16u bx_bswap16(Bit16u val16)
{
  return (val16<<8) | (val16>>8);
}
# 62 "../../misc/bswap.h"
inline void WriteHostWordToLittleEndian(Bit16u *hostPtr, Bit16u nativeVar16)
{
  *(hostPtr) = nativeVar16;
}

inline void WriteHostDWordToLittleEndian(Bit32u *hostPtr, Bit32u nativeVar32)
{
  *(hostPtr) = nativeVar32;
}

inline void WriteHostQWordToLittleEndian(Bit64u *hostPtr, Bit64u nativeVar64)
{
# 85 "../../misc/bswap.h"
  *(hostPtr) = nativeVar64;

}

inline Bit16u ReadHostWordFromLittleEndian(Bit16u *hostPtr)
{
  return *(hostPtr);
}

inline Bit32u ReadHostDWordFromLittleEndian(Bit32u *hostPtr)
{
  return *(hostPtr);
}

inline Bit64u ReadHostQWordFromLittleEndian(Bit64u *hostPtr)
{
# 113 "../../misc/bswap.h"
  return *(hostPtr);

}
# 394 "../../bochs.h" 2
# 29 "../iodev.h" 2
# 1 "../../plugin.h" 1
# 33 "../../plugin.h"
# 1 "../../extplugin.h" 1
# 49 "../../extplugin.h"
typedef int ( *plugin_entry_t)(struct _plugin_t *plugin, Bit16u type, Bit8u mode);

typedef struct _plugin_t
{
# 61 "../../extplugin.h"
    const char *name;

    Bit16u type;
    Bit8u flags;
    plugin_entry_t plugin_entry;
    bool initialized;
    Bit16u loadtype;




} plugin_t;
# 34 "../../plugin.h" 2

class BX_MEM_C;
class bx_devices_c;
 extern logfunctions *pluginlog;


extern "C" {
# 277 "../../plugin.h"
# 1 "/usr/include/dlfcn.h" 1 3 4
# 24 "/usr/include/dlfcn.h" 3 4
# 1 "/usr/lib/gcc/x86_64-linux-gnu/14/include/stddef.h" 1 3 4
# 25 "/usr/include/dlfcn.h" 2 3 4


# 1 "/usr/include/x86_64-linux-gnu/bits/dlfcn.h" 1 3 4
# 57 "/usr/include/x86_64-linux-gnu/bits/dlfcn.h" 3 4

# 57 "/usr/include/x86_64-linux-gnu/bits/dlfcn.h" 3 4
extern "C" {


extern void _dl_mcount_wrapper_check (void *__selfpc) noexcept (true);

}
# 28 "/usr/include/dlfcn.h" 2 3 4



# 1 "/usr/include/x86_64-linux-gnu/bits/dl_find_object.h" 1 3 4
# 32 "/usr/include/dlfcn.h" 2 3 4


typedef long int Lmid_t;
# 52 "/usr/include/dlfcn.h" 3 4
extern "C" {



extern void *dlopen (const char *__file, int __mode) noexcept (true);



extern int dlclose (void *__handle) noexcept (true) __attribute__ ((__nonnull__ (1)));



extern void *dlsym (void *__restrict __handle,
      const char *__restrict __name) noexcept (true) __attribute__ ((__nonnull__ (2)));



extern void *dlmopen (Lmid_t __nsid, const char *__file, int __mode) noexcept (true);



extern void *dlvsym (void *__restrict __handle,
       const char *__restrict __name,
       const char *__restrict __version)
     noexcept (true) __attribute__ ((__nonnull__ (2, 3)));





extern char *dlerror (void) noexcept (true);





typedef struct
{
  const char *dli_fname;
  void *dli_fbase;
  const char *dli_sname;
  void *dli_saddr;
} Dl_info;



extern int dladdr (const void *__address, Dl_info *__info)
     noexcept (true) __attribute__ ((__nonnull__ (2)));


extern int dladdr1 (const void *__address, Dl_info *__info,
      void **__extra_info, int __flags) noexcept (true) __attribute__ ((__nonnull__ (2)));




enum
  {

    RTLD_DL_SYMENT = 1,


    RTLD_DL_LINKMAP = 2
  };







extern int dlinfo (void *__restrict __handle,
     int __request, void *__restrict __arg)
     noexcept (true) __attribute__ ((__nonnull__ (1, 3)));


enum
  {

    RTLD_DI_LMID = 1,



    RTLD_DI_LINKMAP = 2,

    RTLD_DI_CONFIGADDR = 3,






    RTLD_DI_SERINFO = 4,
    RTLD_DI_SERINFOSIZE = 5,



    RTLD_DI_ORIGIN = 6,

    RTLD_DI_PROFILENAME = 7,
    RTLD_DI_PROFILEOUT = 8,




    RTLD_DI_TLS_MODID = 9,





    RTLD_DI_TLS_DATA = 10,




    RTLD_DI_PHDR = 11,

    RTLD_DI_MAX = 11
  };




typedef struct
{
  char *dls_name;
  unsigned int dls_flags;
} Dl_serpath;



typedef struct
{
  size_t dls_size;
  unsigned int dls_cnt;





  __extension__ union
  {
    Dl_serpath dls_serpath[0];
    Dl_serpath __dls_serpath_pad[1];
  };



} Dl_serinfo;

struct dl_find_object
{
  __extension__ unsigned long long int dlfo_flags;
  void *dlfo_map_start;
  void *dlfo_map_end;
  struct link_map *dlfo_link_map;
  void *dlfo_eh_frame;
# 220 "/usr/include/dlfcn.h" 3 4
  __extension__ unsigned long long int __dflo_reserved[7];
};



int _dl_find_object (void *__address, struct dl_find_object *__result) noexcept (true);




}
# 278 "../../plugin.h" 2



# 280 "../../plugin.h"
typedef Bit32u (*ioReadHandler_t)(void *, Bit32u, unsigned);
typedef void (*ioWriteHandler_t)(void *, Bit32u, Bit32u, unsigned);

typedef struct _device_t
{
    const char *name;
    plugin_t *plugin;
    Bit16u plugtype;

    class bx_devmodel_c *devmodel;

    struct _device_t *next;
} device_t;


extern device_t *devices;

void plugin_startup(void);
void plugin_cleanup(void);


typedef void (*deviceInitMem_t)(BX_MEM_C *);
typedef void (*deviceInitDev_t)(void);
typedef void (*deviceReset_t)(unsigned);

 void pluginRegisterDeviceDevmodel(plugin_t *plugin, Bit16u type, bx_devmodel_c *dev, const char *name);
 void pluginUnregisterDeviceDevmodel(const char *name, Bit16u type);
 bool pluginDevicePresent(const char *name);


 extern int (*pluginRegisterIOReadHandler)(void *thisPtr, ioReadHandler_t callback,
                                unsigned base, const char *name, Bit8u mask);
 extern int (*pluginRegisterIOWriteHandler)(void *thisPtr, ioWriteHandler_t callback,
                                 unsigned base, const char *name, Bit8u mask);
 extern int (*pluginUnregisterIOReadHandler)(void *thisPtr, ioReadHandler_t callback,
                                unsigned base, Bit8u mask);
 extern int (*pluginUnregisterIOWriteHandler)(void *thisPtr, ioWriteHandler_t callback,
                                 unsigned base, Bit8u mask);
 extern int (*pluginRegisterIOReadHandlerRange)(void *thisPtr, ioReadHandler_t callback,
                                unsigned base, unsigned end, const char *name, Bit8u mask);
 extern int (*pluginRegisterIOWriteHandlerRange)(void *thisPtr, ioWriteHandler_t callback,
                                 unsigned base, unsigned end, const char *name, Bit8u mask);
 extern int (*pluginUnregisterIOReadHandlerRange)(void *thisPtr, ioReadHandler_t callback,
                                unsigned begin, unsigned end, Bit8u mask);
 extern int (*pluginUnregisterIOWriteHandlerRange)(void *thisPtr, ioWriteHandler_t callback,
                                 unsigned begin, unsigned end, Bit8u mask);
 extern int (*pluginRegisterDefaultIOReadHandler)(void *thisPtr, ioReadHandler_t callback,
                                const char *name, Bit8u mask);
 extern int (*pluginRegisterDefaultIOWriteHandler)(void *thisPtr, ioWriteHandler_t callback,
                                 const char *name, Bit8u mask);


 extern void (*pluginRegisterIRQ)(unsigned irq, const char *name);
 extern void (*pluginUnregisterIRQ)(unsigned irq, const char *name);


 extern void (*pluginSetHRQ)(unsigned val);
 extern void (*pluginSetHRQHackCallback)(void (*callback)(void));

void plugin_abort(plugin_t *plugin);






bool bx_load_plugin(const char *name, Bit16u type);
bool bx_unload_plugin(const char *name, bool devflag);
extern void bx_unload_plugin_type(const char *name, Bit16u type);
extern void bx_init_plugins(void);
extern void bx_reset_plugins(unsigned);
extern void bx_unload_plugins(void);
extern void bx_plugins_register_state(void);
extern void bx_plugins_after_restore_state(void);


extern plugin_t bx_builtin_plugins[];

Bit8u bx_get_plugins_count_np(Bit16u type);
const char* bx_get_plugin_name_np(Bit16u type, Bit8u index);
Bit8u bx_get_plugin_flags_np(Bit16u type, Bit8u index);
int bx_load_plugin_np(const char *name, Bit16u type);
int bx_unload_opt_plugin(const char *name, bool devflag);




int plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
# 410 "../../plugin.h"
int libharddrv_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libkeyboard_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libbusmouse_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libserial_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libunmapped_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libbiosdev_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libcmos_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libdma_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libpic_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libpit_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libvga_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libsvga_cirrus_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libgeforce_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libfloppy_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libparallel_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libpci_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libpci2isa_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libpci_ide_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libpcidev_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libusb_uhci_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libusb_ohci_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libusb_ehci_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libusb_xhci_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libsb16_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libes1370_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libnetmod_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libne2k_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libpcipnic_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libe1000_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libextfpuirq_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libgameport_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libspeaker_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libacpi_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libiodebug_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libioapic_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libhpet_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libvoodoo_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libvoodoorush_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);

int libtextconfig_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libwin32config_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);

int libamigaos_gui_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libcarbon_gui_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libmacintosh_gui_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libnogui_gui_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int librfb_gui_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libsdl_gui_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libsdl2_gui_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libterm_gui_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libvncsrv_gui_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libwin32_gui_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libwx_gui_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libx_gui_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);

int libsoundalsa_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libsounddummy_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libsoundfile_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libsoundoss_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libsoundosx_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libsoundpulse_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libsoundsdl_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libsoundwin_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);

int libeth_fbsd_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libeth_linux_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libeth_null_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libeth_slirp_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libeth_socket_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libeth_tap_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libeth_tuntap_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libeth_vde_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libeth_vnet_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libeth_win32_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);

int libusb_floppy_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libusb_hid_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libusb_hub_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libusb_msd_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libusb_printer_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);

int libvmware3_img_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libvmware4_img_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libvbox_img_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libvpc_img_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);
int libvvfat_img_plugin_entry(plugin_t *plugin, Bit16u type, Bit8u mode);




}
# 30 "../iodev.h" 2
# 1 "../../param_names.h" 1
# 31 "../iodev.h" 2
# 1 "../../pc_system.h" 1
# 29 "../../pc_system.h"
typedef void (*bx_timer_handler_t)(void *);

 extern class bx_pc_system_c bx_pc_system;





class bx_pc_system_c : private logfunctions {
private:





  struct {
    bool inUse;
    Bit64u period;
    Bit64u timeToFire;
    bool active;
    bool continuous;
    bx_timer_handler_t funct;

    void *this_ptr;


    char id[32];
    Bit32u param;
  } timer[64];

  unsigned numTimers;
  unsigned triggeredTimer;
  Bit32u currCountdown;
  Bit32u currCountdownPeriod;
  Bit64u ticksTotal;
  Bit64u lastTimeUsec;
  Bit64u usecSinceLast;





  static const Bit64u NullTimerInterval;
  static void nullTimer(void* this_ptr);





  double m_ips;




  void countdownEvent(void);

public:





  void initialize(Bit32u ips);
  int register_timer(void *this_ptr, bx_timer_handler_t, Bit32u useconds,
                         bool continuous, bool active, const char *id);
  bool unregisterTimer(unsigned timerID);
  void setTimerParam(unsigned timerID, Bit32u param);
  void start_timers(void);
  void activate_timer(unsigned timer_index, Bit32u useconds, bool continuous);
  void activate_timer_nsec(unsigned timer_index, Bit64u nseconds, bool continuous);
  void deactivate_timer(unsigned timer_index);
  unsigned triggeredTimerID(void) {
    return triggeredTimer;
  }
  Bit32u triggeredTimerParam(void) {
    return timer[triggeredTimer].param;
  }
  static inline void tick1(void) {
    if (--bx_pc_system.currCountdown == 0) {
      bx_pc_system.countdownEvent();
    }
  }
  static inline void tickn(Bit32u n) {
    while (n >= bx_pc_system.currCountdown) {
      n -= bx_pc_system.currCountdown;
      bx_pc_system.currCountdown = 0;
      bx_pc_system.countdownEvent();

    }


    bx_pc_system.currCountdown -= n;
  }

  int register_timer_ticks(void* this_ptr, bx_timer_handler_t, Bit64u ticks,
                           bool continuous, bool active, const char *id);
  void activate_timer_ticks(unsigned index, Bit64u instructions, bool continuous);

  Bit64u time_usec();
  Bit64u time_nsec();
  Bit64u time_usec_sequential();
  static inline Bit64u time_ticks() {
    return bx_pc_system.ticksTotal +
      Bit64u(bx_pc_system.currCountdownPeriod - bx_pc_system.currCountdown);
  }

  static inline Bit32u getNumCpuTicksLeftNextEvent(void) {
    return bx_pc_system.currCountdown;
  }



  static void benchmarkTimer(void* this_ptr);

  static void dumpStatsTimer(void* this_ptr);

  void isa_bus_delay(void);





  bool HRQ;





  bool enable_a20;
# 166 "../../pc_system.h"
  bx_phy_address a20_mask;

  volatile bool kill_bochs_request;

  void set_HRQ(bool val);

  void raise_INTR(void);
  void clear_INTR(void);


  int Reset(unsigned type);
  Bit8u IAC(void);

  bx_pc_system_c();

  Bit32u inp(Bit16u addr, unsigned io_len) ;
  void outp(Bit16u addr, Bit32u value, unsigned io_len) ;
  void set_enable_a20(bool value);
  bool get_enable_a20(void);
  void MemoryMappingChanged(void);
  void invlpg(bx_address addr);
  void exit(void);
  void register_state(void);
};
# 32 "../iodev.h" 2
# 1 "../../memory/memory-bochs.h" 1
# 37 "../../memory/memory-bochs.h"
class BX_CPU_C;

const Bit32u BIOSROMSZ = (1 << 22);
const Bit32u EXROMSIZE = (0x20000);

const Bit32u BIOS_MASK = BIOSROMSZ-1;
const Bit32u EXROM_MASK = EXROMSIZE-1;

class BX_MEMORY_STUB_C : public logfunctions {
protected:
  Bit64u len, allocated;
  Bit32u block_size;
  Bit8u *actual_vector;
  Bit8u *vector;
  Bit8u **blocks;
  Bit8u *rom;
  Bit8u *bogus;

  Bit32u used_blocks;

  static Bit8u * const swapped_out;
  Bit32u next_swapout_idx;
  FILE *overflow_file;

  static void read_block(Bit32u block);


public:
  BX_MEMORY_STUB_C();
  virtual ~BX_MEMORY_STUB_C();

  static void init_memory(Bit64u guest, Bit64u host, Bit32u block_size);
  static void cleanup_memory(void);
  static Bit8u* get_vector(bx_phy_address addr);

  static Bit8u* getHostMemAddr(BX_CPU_C *cpu, bx_phy_address addr, unsigned rw);


  static void readPhysicalPage(BX_CPU_C *cpu, bx_phy_address addr,
                                      unsigned len, void *data);
  static void writePhysicalPage(BX_CPU_C *cpu, bx_phy_address addr,
                                       unsigned len, void *data);

  static bool dbg_fetch_mem(BX_CPU_C *cpu, bx_phy_address addr, unsigned len, Bit8u *buf);





  static Bit64u get_memory_len(void);
  static void allocate_block(Bit32u index);
  static Bit8u* alloc_vector_aligned(Bit64u bytes, Bit64u alignment);


  static bool is_monitor(bx_phy_address begin_addr, unsigned len);
  static void check_monitor(bx_phy_address addr, unsigned len);

};

typedef bool (*memory_handler_t)(bx_phy_address addr, unsigned len, void *data, void *param);


typedef Bit8u* (*memory_direct_access_handler_t)(bx_phy_address addr, unsigned rw, void *param);

struct memory_handler_struct {
  struct memory_handler_struct *next;
  void *param;
  bx_phy_address begin;
  bx_phy_address end;
  Bit16u bitmap;
  bool overlap;
  memory_handler_t read_handler;
  memory_handler_t write_handler;
  memory_direct_access_handler_t da_handler;
};







enum memory_area_t {
  BX_MEM_AREA_C0000 = 0,
  BX_MEM_AREA_C4000,
  BX_MEM_AREA_C8000,
  BX_MEM_AREA_CC000,
  BX_MEM_AREA_D0000,
  BX_MEM_AREA_D4000,
  BX_MEM_AREA_D8000,
  BX_MEM_AREA_DC000,
  BX_MEM_AREA_E0000,
  BX_MEM_AREA_E4000,
  BX_MEM_AREA_E8000,
  BX_MEM_AREA_EC000,
  BX_MEM_AREA_F0000
};

class BX_MEM_C : public BX_MEMORY_STUB_C {
private:
  struct memory_handler_struct **memory_handlers;
  bool pci_enabled;
  bool bios_write_enabled;

  bool smram_available;
  bool smram_enable;
  bool smram_restricted;

  bool rom_present[65];
  bool memory_type[13][2];
  Bit32u bios_rom_addr;
  Bit8u bios_rom_access;
  Bit8u flash_type;
  Bit8u flash_status;
  Bit8u flash_wsm_state;
  bool flash_modified;

  static Bit8u flash_read(Bit32u addr);
  static void flash_write(Bit32u addr, Bit8u data);

public:
  BX_MEM_C();
  virtual ~BX_MEM_C();

  static void init_memory(Bit64u guest, Bit64u host, Bit32u block_size);
  static void cleanup_memory(void);

  static void enable_smram(bool enable, bool restricted);
  static void disable_smram(void);
  static bool is_smram_accessible(void);

  static void set_bios_write(bool enabled);
  static void set_bios_rom_access(Bit8u region, bool enabled);
  static void set_memory_type(memory_area_t area, bool rw, bool dram);

  static Bit8u* getHostMemAddr(BX_CPU_C *cpu, bx_phy_address addr, unsigned rw);


  static void readPhysicalPage(BX_CPU_C *cpu, bx_phy_address addr,
                                      unsigned len, void *data);
  static void writePhysicalPage(BX_CPU_C *cpu, bx_phy_address addr,
                                       unsigned len, void *data);

  static void dmaReadPhysicalPage(bx_phy_address addr, unsigned len, Bit8u *data);
  static void dmaWritePhysicalPage(bx_phy_address addr, unsigned len, Bit8u *data);

  static bool load_flash_data(const char *path);
  static bool save_flash_data(const char *path);

  static void load_ROM(const char *path, bx_phy_address romaddress, Bit8u type);
  static void load_RAM(const char *path, bx_phy_address romaddress);

  static bool dbg_fetch_mem(BX_CPU_C *cpu, bx_phy_address addr, unsigned len, Bit8u *buf);




  static bool registerMemoryHandlers(void *param, memory_handler_t read_handler,
                  memory_handler_t write_handler, memory_direct_access_handler_t da_handler,
                  bx_phy_address begin_addr, bx_phy_address end_addr);
  static inline bool registerMemoryHandlers(void *param, memory_handler_t read_handler,
                  memory_handler_t write_handler,
                  bx_phy_address begin_addr, bx_phy_address end_addr)
  {
     return registerMemoryHandlers(param, read_handler, write_handler, 
# 201 "../../memory/memory-bochs.h" 3 4
                                                                      __null
# 201 "../../memory/memory-bochs.h"
                                                                          , begin_addr, end_addr);
  }
  static bool unregisterMemoryHandlers(void *param, bx_phy_address begin_addr, bx_phy_address end_addr);

  void register_state(void);


  friend void ramfile_save_handler(void *devptr, FILE *fp);

  friend Bit64s memory_param_save_handler(void *devptr, bx_param_c *param);
  friend void memory_param_restore_handler(void *devptr, bx_param_c *param, Bit64s val);
};

 extern BX_MEM_C bx_mem;
# 33 "../iodev.h" 2
# 1 "../../gui/siminterface.h" 1
# 109 "../../gui/siminterface.h"
typedef enum {
  BX_TOOLBAR_UNDEFINED,
  BX_TOOLBAR_FLOPPYA,
  BX_TOOLBAR_FLOPPYB,
  BX_TOOLBAR_CDROM1,
  BX_TOOLBAR_RESET,
  BX_TOOLBAR_POWER,
  BX_TOOLBAR_SAVE_RESTORE,
  BX_TOOLBAR_COPY,
  BX_TOOLBAR_PASTE,
  BX_TOOLBAR_SNAPSHOT,
  BX_TOOLBAR_CONFIG,
  BX_TOOLBAR_MOUSE_EN,
  BX_TOOLBAR_USER,
  BX_TOOLBAR_USB_DEBUG
} bx_toolbar_buttons;
# 139 "../../gui/siminterface.h"
enum { BX_EJECTED = 0, BX_INSERTED = 1 };


enum {
  BX_BOOT_NONE,
  BX_BOOT_FLOPPYA,
  BX_BOOT_DISKC,
  BX_BOOT_CDROM,
  BX_BOOT_PCMCIA,
  BX_BOOT_USB,
  BX_BOOT_NETWORK
};
# 213 "../../gui/siminterface.h"
# 1 "../../gui/paramtree.h" 1
# 56 "../../gui/paramtree.h"
typedef enum {
  BXT_OBJECT = 201,
  BXT_PARAM,
  BXT_PARAM_NUM,
  BXT_PARAM_BOOL,
  BXT_PARAM_ENUM,
  BXT_PARAM_STRING,
  BXT_PARAM_BYTESTRING,
  BXT_PARAM_DATA,
  BXT_PARAM_FILEDATA,
  BXT_LIST
} bx_objtype;

class bx_object_c;
class bx_param_c;
class bx_param_num_c;
class bx_param_enum_c;
class bx_param_bool_c;
class bx_param_string_c;
class bx_param_filename_c;
class bx_list_c;

class bx_object_c {
private:
  Bit32u id;
  bx_objtype type;
protected:
  void set_type(bx_objtype _type) { type = _type; }
public:
  bx_object_c(Bit32u _id): id(_id), type(BXT_OBJECT) {}
  virtual ~bx_object_c() {}
  Bit32u get_id() const { return id; }
  Bit8u get_type() const { return type; }
};






class bx_param_c : public bx_object_c {
  static const char *default_text_format;
protected:
  bx_list_c *parent;
  char *name;
  char *description;
  char *label;
  char *text_format;
  char *long_text_format;
  char *ask_format;
  char *group_name;
  bool runtime_param;
  bool enabled;
  Bit32u options;




  bx_list_c *dependent_list;
  void *device;
public:
  enum {


    CI_ONLY = (1<<31)
  } bx_param_opt_bits;

  bx_param_c(Bit32u id, const char *name, const char *description);
  bx_param_c(Bit32u id, const char *name, const char *label, const char *description);
  virtual ~bx_param_c();

  virtual void reset() {}

  const char *get_name() const { return name; }
  bx_param_c *get_parent() { return (bx_param_c *) parent; }

  int get_param_path(char *path_out, int maxlen);

  void set_format(const char *format);
  const char *get_format() const {return text_format;}

  void set_long_format(const char *format);
  const char *get_long_format() const {return long_text_format;}

  void set_ask_format(const char *format);
  const char *get_ask_format() const {return ask_format;}

  void set_label(const char *text);
  const char *get_label() const {return label;}

  void set_description(const char *text);
  const char *get_description() const { return description; }

  virtual void set_runtime_param(bool val) { runtime_param = val; }
  bool get_runtime_param() const { return runtime_param; }

  void set_group(const char *group);
  const char *get_group() const {return group_name;}

  bool get_enabled() const { return enabled; }
  virtual void set_enabled(bool _enabled) { enabled = _enabled; }

  static const char* set_default_format(const char *f);
  static const char *get_default_format() { return default_text_format; }

  bx_list_c *get_dependent_list() { return dependent_list; }

  void set_options(Bit32u _options) { options = _options; }
  Bit32u get_options() const { return options; }

  void set_device_param(void *dev) { device = dev; }
  void *get_device_param() { return device; }

  virtual int parse_param(const char *value) { return -1; }

  virtual void dump_param(FILE *fp) {}
  virtual int dump_param(char *buf, int buflen, bool dquotes = false) { return 0; }
};

typedef Bit64s (*param_event_handler)(class bx_param_c *, bool set, Bit64s val);
typedef Bit64s (*param_save_handler)(void *devptr, class bx_param_c *);
typedef void (*param_restore_handler)(void *devptr, class bx_param_c *, Bit64s val);
typedef bool (*param_enable_handler)(class bx_param_c *, bool en);

class bx_param_num_c : public bx_param_c {
  static Bit32u default_base;
  void update_dependents();
protected:
  Bit64s min, max, initial_val;
  union _uval_ {
    Bit64s number;
    Bit64s *p64bit;
    Bit32s *p32bit;
    Bit16s *p16bit;
    Bit8s *p8bit;
    float *pfloat;
    double *pdouble;
    bool *pbool;
  } val;
  param_event_handler handler;
  void *sr_devptr;
  param_save_handler save_handler;
  param_restore_handler restore_handler;
  param_enable_handler enable_handler;
  int base;
  bool is_shadow;
public:
  enum {


    USE_SPIN_CONTROL = (1<<0)
  } bx_numopt_bits;
  bx_param_num_c(bx_param_c *parent,
      const char *name,
      const char *label,
      const char *description,
      Bit64s min, Bit64s max, Bit64s initial_val,
      bool is_shadow = 0);
  virtual void reset() { set(initial_val); }
  void set_handler(param_event_handler handler);
  void set_sr_handlers(void *devptr, param_save_handler save, param_restore_handler restore);
  void set_enable_handler(param_enable_handler handler) { enable_handler = handler; }
  void set_dependent_list(bx_list_c *l);
  virtual void set_enabled(bool enabled);
  virtual Bit32s get() { return (Bit32s) get64(); }
  virtual Bit64s get64();
  virtual void set(Bit64s val);
  void set_base(int _base) { base = _base; }
  void set_initial_val(Bit64s initial_val);
  int get_base() const { return base; }
  void set_range(Bit64u min, Bit64u max);
  Bit64s get_min() const { return min; }
  Bit64s get_max() const { return max; }
  static Bit32u set_default_base(Bit32u val);
  static Bit32u get_default_base() { return default_base; }
  virtual int parse_param(const char *value);
  virtual void dump_param(FILE *fp);
  virtual int dump_param(char *buf, int buflen, bool dquotes = false);
};






class bx_shadow_num_c : public bx_param_num_c {
  Bit8u varsize;
  Bit8u lowbit;
  Bit64u mask;
public:
  bx_shadow_num_c(bx_param_c *parent,
      const char *name,
      Bit64s *ptr_to_real_val,
      int base = 10,
      Bit8u highbit = 63,
      Bit8u lowbit = 0);
  bx_shadow_num_c(bx_param_c *parent,
      const char *name,
      Bit64u *ptr_to_real_val,
      int base = 10,
      Bit8u highbit = 63,
      Bit8u lowbit = 0);
  bx_shadow_num_c(bx_param_c *parent,
      const char *name,
      Bit32s *ptr_to_real_val,
      int base = 10,
      Bit8u highbit = 31,
      Bit8u lowbit = 0);
  bx_shadow_num_c(bx_param_c *parent,
      const char *name,
      Bit32u *ptr_to_real_val,
      int base = 10,
      Bit8u highbit = 31,
      Bit8u lowbit = 0);
  bx_shadow_num_c(bx_param_c *parent,
      const char *name,
      Bit16s *ptr_to_real_val,
      int base = 10,
      Bit8u highbit = 15,
      Bit8u lowbit = 0);
  bx_shadow_num_c(bx_param_c *parent,
      const char *name,
      Bit16u *ptr_to_real_val,
      int base = 10,
      Bit8u highbit = 15,
      Bit8u lowbit = 0);
  bx_shadow_num_c(bx_param_c *parent,
      const char *name,
      Bit8s *ptr_to_real_val,
      int base = 10,
      Bit8u highbit = 7,
      Bit8u lowbit = 0);
  bx_shadow_num_c(bx_param_c *parent,
      const char *name,
      Bit8u *ptr_to_real_val,
      int base = 10,
      Bit8u highbit = 7,
      Bit8u lowbit = 0);
  bx_shadow_num_c(bx_param_c *parent,
      const char *name,
      double *ptr_to_real_val);
  bx_shadow_num_c(bx_param_c *parent,
      const char *name,
      float *ptr_to_real_val);
  virtual Bit64s get64();
  virtual void set(Bit64s val);
  virtual void reset();
};

class bx_param_bool_c : public bx_param_num_c {



public:
  bx_param_bool_c(bx_param_c *parent,
      const char *name,
      const char *label,
      const char *description,
      Bit64s initial_val,
      bool is_shadow = 0);
  virtual int parse_param(const char *value);
  virtual void dump_param(FILE *fp);
  virtual int dump_param(char *buf, int buflen, bool dquotes = false);
};


class bx_shadow_bool_c : public bx_param_bool_c {
public:
  bx_shadow_bool_c(bx_param_c *parent,
      const char *name,
      const char *label,
      bool *ptr_to_real_val);
  bx_shadow_bool_c(bx_param_c *parent,
      const char *name,
      bool *ptr_to_real_val);
  virtual Bit64s get64();
  virtual void set(Bit64s val);
};


class bx_param_enum_c : public bx_param_num_c {
  const char **choices;
  Bit64u *deps_bitmap;
  void update_dependents();
public:
  bx_param_enum_c(bx_param_c *parent,
      const char *name,
      const char *label,
      const char *description,
      const char **choices,
      Bit64s initial_val,
      Bit64s value_base = 0);
  virtual ~bx_param_enum_c();
  const char *get_choice(int n) { return choices[n]; }
  const char **get_choices() { return choices; }
  const char *get_selected() { return choices[val.number - min]; }
  int find_by_name(const char *s);
  virtual void set(Bit64s val);
  bool set_by_name(const char *s);
  void set_dependent_list(bx_list_c *l, bool enable_all);
  void set_dependent_bitmap(Bit64s value, Bit64u bitmap);
  Bit64u get_dependent_bitmap(Bit64s value);
  virtual void set_enabled(bool enabled);
  virtual int parse_param(const char *value);
  virtual void dump_param(FILE *fp);
  virtual int dump_param(char *buf, int buflen, bool dquotes = false);
};

typedef const char* (*param_string_event_handler)(class bx_param_string_c *,
                     bool set, const char *oldval, const char *newval, int maxlen);

class bx_param_string_c : public bx_param_c {
protected:
  int maxsize;
  char *val, *initial_val;
  param_string_event_handler handler;
  param_enable_handler enable_handler;
  void update_dependents();
public:
  enum {
    IS_FILENAME = 1,


    SAVE_FILE_DIALOG = 2,
    SELECT_FOLDER_DLG = 4
  } bx_string_opt_bits;
  bx_param_string_c(bx_param_c *parent,
      const char *name,
      const char *label,
      const char *description,
      const char *initial_val,
      int maxsize=-1);
  virtual ~bx_param_string_c();
  virtual void reset();
  void set_handler(param_string_event_handler handler);
  void set_enable_handler(param_enable_handler handler);
  param_enable_handler get_enable_handler() { return enable_handler; }
  virtual void set_enabled(bool enabled);
  void set_dependent_list(bx_list_c *l);
  Bit32s get(char *buf, int len);
  char *getptr() {return val; }
  const char *getptr() const {return val; }
  virtual void set(const char *buf);
  bool equals(const char *buf) const;
  int get_maxsize() const {return maxsize; }
  virtual void set_initial_val(const char *buf);
  bool isempty() const;
  virtual int parse_param(const char *value);
  virtual void dump_param(FILE *fp);
  virtual int dump_param(char *buf, int buflen, bool dquotes = false);
};

class bx_param_bytestring_c : public bx_param_string_c {
  char separator;
public:
  bx_param_bytestring_c(bx_param_c *parent,
      const char *name,
      const char *label,
      const char *description,
      const char *initial_val,
      int maxsize) : bx_param_string_c(parent, name, label, description, initial_val, maxsize)
  {
    set_type(BXT_PARAM_BYTESTRING);
  }

  void set_separator(char sep) {separator = sep; }
  char get_separator() const {return separator; }

  Bit32s get(char *buf, int len);
  void set(const char *buf);
  bool equals(const char *buf) const;
  void set_initial_val(const char *buf);
  bool isempty() const;

  virtual int parse_param(const char *value);
  virtual int dump_param(char *buf, int buflen, bool dquotes = false);
};




class bx_param_filename_c : public bx_param_string_c {
const char *ext;
public:
  bx_param_filename_c(bx_param_c *parent,
      const char *name,
      const char *label,
      const char *description,
      const char *initial_val,
      int maxsize=-1);
  const char *get_extension() const {return ext;}
  void set_extension(const char *newext) {ext = newext;}

  virtual void set(const char *buf);
  virtual void set_initial_val(const char *buf);
};

class bx_shadow_data_c : public bx_param_c {
  Bit32u data_size;
  Bit8u *data_ptr;
  bool is_text;
public:
  bx_shadow_data_c(bx_param_c *parent,
      const char *name,
      Bit8u *ptr_to_data,
      Bit32u data_size, bool is_text=0);
  Bit8u *getptr() {return data_ptr;}
  const Bit8u *getptr() const {return data_ptr;}
  Bit32u get_size() const {return data_size;}
  bool is_text_format() const {return is_text;}
  Bit8u get(Bit32u index);
  void set(Bit32u index, Bit8u value);
};

typedef void (*filedata_save_handler)(void *devptr, FILE *save_fp);
typedef void (*filedata_restore_handler)(void *devptr, FILE *save_fp);

class bx_shadow_filedata_c : public bx_param_c {
protected:
  FILE **scratch_fpp;
  void *sr_devptr;
  filedata_save_handler save_handler;
  filedata_restore_handler restore_handler;

public:
  bx_shadow_filedata_c(bx_param_c *parent,
      const char *name, FILE **scratch_file_ptr_ptr);
  void set_sr_handlers(void *devptr, filedata_save_handler save, filedata_restore_handler restore);
  FILE **get_fpp() {return scratch_fpp;}
  void save(FILE *save_file);
  void restore(FILE *save_file);
};

typedef struct _bx_listitem_t {
  bx_param_c *param;
  struct _bx_listitem_t *next;
} bx_listitem_t;

typedef void (*list_restore_handler)(void *devptr, class bx_list_c *);

class bx_list_c : public bx_param_c {
protected:

  bx_listitem_t *list;
  int size;



  Bit32u choice;

  char *title;
  void init(const char *list_title);

  void *sr_devptr;
  list_restore_handler restore_handler;
public:
  enum {


    SHOW_PARENT = (1<<0),



    SERIES_ASK = (1<<1),




    USE_TAB_WINDOW = (1<<2),



    USE_BOX_TITLE = (1<<3),



    SHOW_GROUP_NAME = (1<<4),




    USE_SCROLL_WINDOW = (1<<5)
  } bx_listopt_bits;
  bx_list_c(bx_param_c *parent);
  bx_list_c(bx_param_c *parent, const char *name);
  bx_list_c(bx_param_c *parent, const char *name, const char *title);
  bx_list_c(bx_param_c *parent, const char *name, const char *title, bx_param_c **init_list);
  virtual ~bx_list_c();
  bx_list_c *clone();
  void add(bx_param_c *param);
  bx_param_c *get(int index);
  bx_param_c *get_by_name(const char *name);
  int get_size() const { return size; }
  Bit32u get_choice() const { return choice; }
  void set_choice(Bit32u new_choice) { choice = new_choice; }
  char *get_title() { return title; }
  void set_parent(bx_param_c *newparent);
  bx_param_c *get_parent() { return parent; }
  virtual void reset();
  virtual void clear();
  virtual void remove(const char *name);
  virtual void set_runtime_param(bool val);
  void set_restore_handler(void *devptr, list_restore_handler restore);
  void restore();
};
# 214 "../../gui/siminterface.h" 2



typedef enum {
  __ALL_EVENTS_BELOW_ARE_SYNCHRONOUS__ = 2000,
  BX_SYNC_EVT_GET_PARAM,
  BX_SYNC_EVT_ASK_PARAM,
  BX_SYNC_EVT_TICK,
  BX_SYNC_EVT_LOG_DLG,
  BX_SYNC_EVT_GET_DBG_COMMAND,
  BX_SYNC_EVT_MSG_BOX,
  BX_SYNC_EVT_ML_MSG_BOX,
  BX_SYNC_EVT_ML_MSG_BOX_KILL,
  __ALL_EVENTS_BELOW_ARE_ASYNC__,
  BX_ASYNC_EVT_KEY,
  BX_ASYNC_EVT_MOUSE,
  BX_ASYNC_EVT_SET_PARAM,
  BX_ASYNC_EVT_LOG_MSG,
  BX_ASYNC_EVT_DBG_MSG,
  BX_ASYNC_EVT_VALUE_CHANGED,
  BX_ASYNC_EVT_TOOLBAR,
  BX_ASYNC_EVT_STATUSBAR,
  BX_ASYNC_EVT_REFRESH,
  BX_ASYNC_EVT_QUIT_SIM
} BxEventType;

typedef union {
  Bit32s s32;
  char *charptr;
} AnyParamVal;
# 264 "../../gui/siminterface.h"
typedef struct {


  Bit32u bx_key;
  bool raw_scancode;
} BxKeyEvent;





typedef struct {

  Bit16s dx, dy, dz;
  Bit8u buttons;



} BxMouseEvent;
# 297 "../../gui/siminterface.h"
typedef struct {

  class bx_param_c *param;
  AnyParamVal val;
} BxParamEvent;
# 344 "../../gui/siminterface.h"
typedef struct {
  Bit8u level;
  Bit8u mode;
  const char *prefix;
  const char *msg;
} BxLogMsgEvent;
# 364 "../../gui/siminterface.h"
enum {
  BX_LOG_ASK_CHOICE_CONTINUE,
  BX_LOG_ASK_CHOICE_CONTINUE_ALWAYS,
  BX_LOG_ASK_CHOICE_DIE,
  BX_LOG_ASK_CHOICE_DUMP_CORE,
  BX_LOG_ASK_CHOICE_ENTER_DEBUG,
  BX_LOG_ASK_N_CHOICES,
  BX_LOG_NOTIFY_FAILED
};

enum {
  BX_LOG_DLG_ASK,
  BX_LOG_DLG_WARN,
  BX_LOG_DLG_QUIT
};







typedef struct {
  char *command;

} BxDebugCommand;
# 398 "../../gui/siminterface.h"
typedef struct {
  bx_toolbar_buttons button;
  bool on;

} BxToolbarEvent;


typedef struct {
  int element;
  char *text;
  bool active;
  bool w;
} BxStatusbarEvent;



typedef struct {
  BxEventType type;
  Bit32s retcode;
  void *param0;
  union {
    BxKeyEvent key;
    BxMouseEvent mouse;
    BxParamEvent param;
    BxLogMsgEvent logmsg;
    BxToolbarEvent toolbar;
    BxStatusbarEvent statbar;
    BxDebugCommand debugcmd;
  } u;
} BxEvent;


enum {


  BX_QUICK_START = 200,



  BX_LOAD_START,


  BX_EDIT_START,


  BX_RUN_START
};

enum {
  BX_VGA_EXTENSION_NONE,
  BX_VGA_EXTENSION_VBE
};

enum {
  BX_DDC_MODE_DISABLED,
  BX_DDC_MODE_BUILTIN,
  BX_DDC_MODE_BUILTIN_GUI,
  BX_DDC_MODE_FILE
};

enum {
  BX_VBE_MEMSIZE_4MB,
  BX_VBE_MEMSIZE_8MB,
  BX_VBE_MEMSIZE_16MB,
  BX_VBE_MEMSIZE_32MB
};

enum {
  BX_MOUSE_TYPE_NONE,
  BX_MOUSE_TYPE_PS2,
  BX_MOUSE_TYPE_IMPS2,




  BX_MOUSE_TYPE_SERIAL,
  BX_MOUSE_TYPE_SERIAL_WHEEL,
  BX_MOUSE_TYPE_SERIAL_MSYS
};

enum {
  BX_MOUSE_TOGGLE_CTRL_MB,
  BX_MOUSE_TOGGLE_CTRL_F10,
  BX_MOUSE_TOGGLE_CTRL_ALT,
  BX_MOUSE_TOGGLE_CTRL_ALT_G,
  BX_MOUSE_TOGGLE_F12
};
# 515 "../../gui/siminterface.h"
enum {
  BX_SECT_SIZE_512,
  BX_SECT_SIZE_1024,
  BX_SECT_SIZE_4096
};

enum {
  BX_ATA_TRANSLATION_NONE,
  BX_ATA_TRANSLATION_LBA,
  BX_ATA_TRANSLATION_LARGE,
  BX_ATA_TRANSLATION_RECHS,
  BX_ATA_TRANSLATION_AUTO
};


enum {
  BX_CLOCK_SYNC_NONE,
  BX_CLOCK_SYNC_REALTIME,
  BX_CLOCK_SYNC_SLOWDOWN,
  BX_CLOCK_SYNC_BOTH
};


enum {
  BX_PCI_CHIPSET_I430FX,
  BX_PCI_CHIPSET_I440FX,
  BX_PCI_CHIPSET_I440BX
};




 extern const char *floppy_devtype_names[];
 extern const char *floppy_type_names[];
 extern int floppy_type_n_sectors[];
 extern const char *media_status_names[];
 extern const char *bochs_bootdisk_names[];
# 588 "../../gui/siminterface.h"
# 1 "/usr/include/setjmp.h" 1 3 4
# 27 "/usr/include/setjmp.h" 3 4

# 27 "/usr/include/setjmp.h" 3 4
extern "C" {

# 1 "/usr/include/x86_64-linux-gnu/bits/setjmp.h" 1 3 4
# 26 "/usr/include/x86_64-linux-gnu/bits/setjmp.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/wordsize.h" 1 3 4
# 27 "/usr/include/x86_64-linux-gnu/bits/setjmp.h" 2 3 4




typedef long int __jmp_buf[8];
# 30 "/usr/include/setjmp.h" 2 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/types/struct___jmp_buf_tag.h" 1 3 4
# 26 "/usr/include/x86_64-linux-gnu/bits/types/struct___jmp_buf_tag.h" 3 4
struct __jmp_buf_tag
  {




    __jmp_buf __jmpbuf;
    int __mask_was_saved;
    __sigset_t __saved_mask;
  };
# 31 "/usr/include/setjmp.h" 2 3 4

typedef struct __jmp_buf_tag jmp_buf[1];



extern int setjmp (jmp_buf __env) noexcept (true);




extern int __sigsetjmp (struct __jmp_buf_tag __env[1], int __savemask) noexcept (true);



extern int _setjmp (struct __jmp_buf_tag __env[1]) noexcept (true);
# 54 "/usr/include/setjmp.h" 3 4
extern void longjmp (struct __jmp_buf_tag __env[1], int __val)
     noexcept (true) __attribute__ ((__noreturn__));





extern void _longjmp (struct __jmp_buf_tag __env[1], int __val)
     noexcept (true) __attribute__ ((__noreturn__));







typedef struct __jmp_buf_tag sigjmp_buf[1];
# 80 "/usr/include/setjmp.h" 3 4
extern void siglongjmp (sigjmp_buf __env, int __val)
     noexcept (true) __attribute__ ((__noreturn__));





# 1 "/usr/include/x86_64-linux-gnu/bits/setjmp2.h" 1 3 4
# 25 "/usr/include/x86_64-linux-gnu/bits/setjmp2.h" 3 4
extern void longjmp (struct __jmp_buf_tag __env[1], int __val) noexcept (true) __asm__ ("" "__longjmp_chk")

                        __attribute__ ((__noreturn__));
extern void _longjmp (struct __jmp_buf_tag __env[1], int __val) noexcept (true) __asm__ ("" "__longjmp_chk")

                        __attribute__ ((__noreturn__));
extern void siglongjmp (struct __jmp_buf_tag __env[1], int __val) noexcept (true) __asm__ ("" "__longjmp_chk")

                        __attribute__ ((__noreturn__));
# 88 "/usr/include/setjmp.h" 2 3 4


}
# 589 "../../gui/siminterface.h" 2


# 590 "../../gui/siminterface.h"
enum ci_command_t { CI_START, CI_RUNTIME_CONFIG, CI_SHUTDOWN };
enum ci_return_t {
  CI_OK,
  CI_ERR_NO_TEXT_CONSOLE
};
typedef int (*config_interface_callback_t)(void *userdata, ci_command_t command);
typedef BxEvent* (*bxevent_handler)(void *theclass, BxEvent *event);
typedef void (*rt_conf_handler_t)(void *this_ptr);
typedef Bit32s (*addon_option_parser_t)(const char *context, int num_params, char *params[]);
typedef Bit32s (*addon_option_save_t)(FILE *fp);
# 609 "../../gui/siminterface.h"
enum disp_mode_t { DISP_MODE_CONFIG=100, DISP_MODE_SIM };

class bx_simulator_interface_c {
public:
  bx_simulator_interface_c() {}
  virtual ~bx_simulator_interface_c() {}
  virtual void set_quit_context(jmp_buf *context) {}
  virtual bool get_init_done() { return 0; }
  virtual int set_init_done(bool n) {return 0;}
  virtual bool get_ci_started() { return 0; }
  virtual void reset_all_param() {}

  virtual bx_param_c *get_param(const char *pname, bx_param_c *base=
# 621 "../../gui/siminterface.h" 3 4
                                                                   __null
# 621 "../../gui/siminterface.h"
                                                                       ) {return 
# 621 "../../gui/siminterface.h" 3 4
                                                                                 __null
# 621 "../../gui/siminterface.h"
                                                                                     ;}
  virtual bx_param_num_c *get_param_num(const char *pname, bx_param_c *base=
# 622 "../../gui/siminterface.h" 3 4
                                                                           __null
# 622 "../../gui/siminterface.h"
                                                                               ) {return 
# 622 "../../gui/siminterface.h" 3 4
                                                                                         __null
# 622 "../../gui/siminterface.h"
                                                                                             ;}
  virtual bx_param_string_c *get_param_string(const char *pname, bx_param_c *base=
# 623 "../../gui/siminterface.h" 3 4
                                                                                 __null
# 623 "../../gui/siminterface.h"
                                                                                     ) {return 
# 623 "../../gui/siminterface.h" 3 4
                                                                                               __null
# 623 "../../gui/siminterface.h"
                                                                                                   ;}
  virtual bx_param_bool_c *get_param_bool(const char *pname, bx_param_c *base=
# 624 "../../gui/siminterface.h" 3 4
                                                                             __null
# 624 "../../gui/siminterface.h"
                                                                                 ) {return 
# 624 "../../gui/siminterface.h" 3 4
                                                                                           __null
# 624 "../../gui/siminterface.h"
                                                                                               ;}
  virtual bx_param_enum_c *get_param_enum(const char *pname, bx_param_c *base=
# 625 "../../gui/siminterface.h" 3 4
                                                                             __null
# 625 "../../gui/siminterface.h"
                                                                                 ) {return 
# 625 "../../gui/siminterface.h" 3 4
                                                                                           __null
# 625 "../../gui/siminterface.h"
                                                                                               ;}
  virtual unsigned gen_param_id() {return 0;}
  virtual int get_n_log_modules() {return -1;}
  virtual const char *get_logfn_name(int mod) {return 
# 628 "../../gui/siminterface.h" 3 4
                                                     __null
# 628 "../../gui/siminterface.h"
                                                         ;}
  virtual int get_logfn_id(const char *name) {return -1;}
  virtual const char *get_prefix(int mod) {return 
# 630 "../../gui/siminterface.h" 3 4
                                                 __null
# 630 "../../gui/siminterface.h"
                                                     ;}
  virtual int get_log_action(int mod, int level) {return -1;}
  virtual void set_log_action(int mod, int level, int action) {}
  virtual int get_default_log_action(int level) {return -1;}
  virtual void set_default_log_action(int level, int action) {}
  virtual const char *get_action_name(int action) {return 
# 635 "../../gui/siminterface.h" 3 4
                                                         __null
# 635 "../../gui/siminterface.h"
                                                             ;}
  virtual int is_action_name(const char *val) {return -1;}
  virtual const char *get_log_level_name(int level) {return 
# 637 "../../gui/siminterface.h" 3 4
                                                           __null
# 637 "../../gui/siminterface.h"
                                                               ;}
  virtual int get_max_log_level() {return -1;}







  virtual void quit_sim(int code) {}

  virtual int get_exit_code() { return 0; }

  virtual int get_default_rc(char *path, int len) {return -1;}
  virtual int read_rc(const char *path) {return -1;}
  virtual int write_rc(const char *rc, int overwrite) {return -1;}
  virtual int get_log_file(char *path, int len) {return -1;}
  virtual int set_log_file(const char *path) {return -1;}
  virtual int get_log_prefix(char *prefix, int len) {return -1;}
  virtual int set_log_prefix(const char *prefix) {return -1;}
  virtual int get_debugger_log_file(char *path, int len) {return -1;}
  virtual int set_debugger_log_file(const char *path) {return -1;}
# 670 "../../gui/siminterface.h"
  virtual void set_notify_callback(bxevent_handler func, void *arg) {}
  virtual void get_notify_callback(bxevent_handler *func, void **arg) {}


  virtual BxEvent* sim_to_ci_event(BxEvent *event) {return 
# 674 "../../gui/siminterface.h" 3 4
                                                          __null
# 674 "../../gui/siminterface.h"
                                                              ;}





  virtual int log_dlg(const char *prefix, int level, const char *msg, int mode) {return -1;}

  virtual void log_msg(const char *prefix, int level, const char *msg) {}

  virtual void set_log_viewer(bool val) {}
  virtual bool has_log_viewer() const {return 0;}


  virtual int ask_param(bx_param_c *param) {return -1;}
  virtual int ask_param(const char *pname) {return -1;}


  virtual int ask_filename(const char *filename, int maxlen, const char *prompt, const char *the_default, int flags) {return -1;}

  virtual int ask_yes_no(const char *title, const char *prompt, bool the_default) {return -1;}

  virtual void message_box(const char *title, const char *message) {}

  virtual void *ml_message_box(const char *title, const char *message) {return 
# 698 "../../gui/siminterface.h" 3 4
                                                                              __null
# 698 "../../gui/siminterface.h"
                                                                                  ;}

  virtual void ml_message_box_kill(void *ptr) {}

  virtual void periodic() {}
  virtual int create_disk_image(const char *filename, int sectors, bool overwrite) {return -3;}



  virtual void refresh_ci() {}



  virtual void refresh_vga() {}



  virtual void handle_events() {}

  virtual bx_param_c *get_first_hd() {return 
# 717 "../../gui/siminterface.h" 3 4
                                            __null
# 717 "../../gui/siminterface.h"
                                                ;}

  virtual bx_param_c *get_first_cdrom() {return 
# 719 "../../gui/siminterface.h" 3 4
                                               __null
# 719 "../../gui/siminterface.h"
                                                   ;}

  virtual bool is_pci_device(const char *name) {return 0;}

  virtual bool is_agp_device(const char *name) {return 0;}
  virtual bool debugger_active() {return false;}







  virtual void register_configuration_interface(
    const char* name,
    config_interface_callback_t callback,
    void *userdata) {}
  virtual int configuration_interface(const char* name, ci_command_t command) {return -1; }





  virtual int begin_simulation(int argc, char *argv[]) {return -1;}
  virtual int register_runtime_config_handler(void *dev, rt_conf_handler_t handler) {return 0;}
  virtual void unregister_runtime_config_handler(int id) {}
  virtual void update_runtime_options() {}
  typedef bool (*is_sim_thread_func_t)();
  is_sim_thread_func_t is_sim_thread_func;
  virtual void set_sim_thread_func(is_sim_thread_func_t func) {
    is_sim_thread_func = func;
  }
  virtual bool is_sim_thread() {return 1;}
  virtual bool is_wx_selected() const {return 0;}
  virtual void set_debug_gui(bool val) {}
  virtual bool has_debug_gui() const {return 0;}


  virtual void set_display_mode(disp_mode_t newmode) {}
  virtual bool test_for_text_console() {return 1;}


  virtual bool register_addon_option(const char *keyword, addon_option_parser_t parser, addon_option_save_t save_func) {return 0;}
  virtual bool unregister_addon_option(const char *keyword) {return 0;}
  virtual bool is_addon_option(const char *keyword) {return 0;}
  virtual Bit32s parse_addon_option(const char *context, int num_params, char *params []) {return -1;}
  virtual Bit32s save_addon_options(FILE *fp) {return -1;}


  virtual void init_statistics() {}
  virtual void cleanup_statistics() {}
  virtual bx_list_c *get_statistics_root() {return 
# 770 "../../gui/siminterface.h" 3 4
                                                  __null
# 770 "../../gui/siminterface.h"
                                                      ;}


  virtual void init_save_restore() {}
  virtual void cleanup_save_restore() {}
  virtual bool save_state(const char *checkpoint_path) {return 0;}
  virtual bool restore_config() {return 0;}
  virtual bool restore_logopts() {return 0;}
  virtual bool restore_hardware() {return 0;}
  virtual bx_list_c *get_bochs_root() {return 
# 779 "../../gui/siminterface.h" 3 4
                                             __null
# 779 "../../gui/siminterface.h"
                                                 ;}
  virtual bool restore_bochs_param(bx_list_c *root, const char *sr_path, const char *restore_name) { return 0; }


  virtual bool opt_plugin_ctrl(const char *plugname, bool load) {return 0;}
  virtual void init_std_nic_options(const char *name, bx_list_c *menu) {}
  virtual void init_usb_options(const char *usb_name, const char *pname, int maxports, int param0) {}
  virtual int parse_param_from_list(const char *context, const char *param, bx_list_c *base) {return 0;}
  virtual int parse_nic_params(const char *context, const char *param, bx_list_c *base) {return 0;}
  virtual int parse_usb_port_params(const char *context, const char *param,
                                     int maxports, bx_list_c *base) {return -1;}
  virtual int split_option_list(const char *msg, const char *rawopt, char **argv, int max_argv) {return 0;}
  virtual int write_param_list(FILE *fp, bx_list_c *base, const char *optname, bool multiline) {return 0;}
  virtual int write_usb_options(FILE *fp, int maxports, bx_list_c *base) {return 0;}


  virtual int bx_printf(const char *fmt, ...) {return 0;}
  virtual char* bx_gets(char *s, int size, FILE *stream) {return 
# 796 "../../gui/siminterface.h" 3 4
                                                                __null
# 796 "../../gui/siminterface.h"
                                                                    ;}

};

 extern bx_simulator_interface_c *SIM;

extern void bx_init_siminterface();
extern int bx_cleanup_siminterface();
# 812 "../../gui/siminterface.h"
typedef struct {

  int argc;
  char **argv;
# 827 "../../gui/siminterface.h"
} bx_startup_flags_t;

 extern bx_startup_flags_t bx_startup_flags;
 extern bool bx_user_quit;
# 34 "../iodev.h" 2
# 1 "../../gui/gui.h" 1
# 91 "../../gui/gui.h"
typedef struct {
  Bit16u start_address;
  Bit8u cs_start;
  Bit8u cs_end;
  Bit16u line_offset;
  Bit16u line_compare;
  Bit8u h_panning;
  Bit8u v_panning;
  bool line_graphics;
  bool split_hpanning;
  Bit8u blink_flags;
  Bit8u actl_palette[16];
} bx_vga_tminfo_t;

typedef struct {
  Bit16u bpp, pitch;
  Bit8u red_shift, green_shift, blue_shift;
  Bit8u is_indexed, is_little_endian;
  unsigned long red_mask, green_mask, blue_mask;
  bool snapshot_mode;
} bx_svga_tileinfo_t;


 Bit8u reverse_bitorder(Bit8u);

 extern class bx_gui_c *bx_gui;
# 126 "../../gui/gui.h"
class bx_gui_c : public logfunctions {
public:
  bx_gui_c (void);
  virtual ~bx_gui_c ();


  virtual void specific_init(int argc, char **argv, unsigned header_bar_y) = 0;
  virtual void text_update(Bit8u *old_text, Bit8u *new_text,
                          unsigned long cursor_x, unsigned long cursor_y,
                          bx_vga_tminfo_t *tm_info) = 0;
  virtual void graphics_tile_update(Bit8u *tile, unsigned x, unsigned y) = 0;
  virtual void handle_events(void) = 0;
  virtual void flush(void) = 0;
  virtual void clear_screen(void) = 0;
  virtual bool palette_change(Bit8u index, Bit8u red, Bit8u green, Bit8u blue) = 0;
  virtual void dimension_update(unsigned x, unsigned y, unsigned fheight=0, unsigned fwidth=0, unsigned bpp=8) = 0;
  virtual unsigned create_bitmap(const unsigned char *bmap, unsigned xdim, unsigned ydim) = 0;
  virtual unsigned headerbar_bitmap(unsigned bmap_id, unsigned alignment, void (*f)(void)) = 0;
  virtual void replace_bitmap(unsigned hbar_id, unsigned bmap_id) = 0;
  virtual void show_headerbar(void) = 0;
  virtual int get_clipboard_text(Bit8u **bytes, Bit32s *nbytes) = 0;
  virtual int set_clipboard_text(char *snapshot, Bit32u len) = 0;
  virtual void mouse_enabled_changed_specific (bool val) = 0;
  virtual void exit(void) = 0;

  virtual bx_svga_tileinfo_t *graphics_tile_info(bx_svga_tileinfo_t *info);
  virtual Bit8u *graphics_tile_get(unsigned x, unsigned y, unsigned *w, unsigned *h);
  virtual void graphics_tile_update_in_place(unsigned x, unsigned y, unsigned w, unsigned h);

  virtual void set_font(bool lg) {}
  virtual void draw_char(Bit8u ch, Bit8u fc, Bit8u bc, Bit16u xc, Bit16u yc,
                         Bit8u fw, Bit8u fh, Bit8u fx, Bit8u fy,
                         bool gfxcharw9, Bit8u cs, Bit8u ce, bool curs, bool font2) {}

  virtual void statusbar_setitem_specific(int element, bool active, bool w) {}
  virtual void set_tooltip(unsigned hbar_id, const char *tip) {}







  virtual void set_display_mode (disp_mode_t newmode) { }



  virtual Bit32u get_sighandler_mask () {return 0;}

  virtual void sighandler (int sig) {}




  virtual void show_ips(Bit32u ips_count);
  virtual void beep_on(float frequency);
  virtual void beep_off();
  virtual void get_capabilities(Bit16u *xres, Bit16u *yres, Bit16u *bpp);
  virtual void set_mouse_mode_absxy(bool mode) {}

  virtual void set_console_edit_mode(bool mode) {}

  virtual void start_update(void) {}



  static void key_event(Bit32u key);
  static void set_text_charmap(Bit8u map, Bit8u *fbuffer);
  static Bit8u get_mouse_headerbar_id();

  void init(int argc, char **argv, unsigned max_xres, unsigned max_yres,
            unsigned x_tilesize, unsigned y_tilesize);
  void cleanup(void);
  void draw_char_common(Bit8u ch, Bit8u fc, Bit8u bc, Bit16u xc, Bit16u yc,
                        Bit8u fw, Bit8u fh, Bit8u fx, Bit8u fy,
                        bool gfxcharw9, Bit8u cs, Bit8u ce, bool curs, bool font2);
  void text_update_common(Bit8u *old_text, Bit8u *new_text,
                          Bit16u cursor_address, bx_vga_tminfo_t *tm_info);
  void graphics_tile_update_common(Bit8u *tile, unsigned x, unsigned y);
  bx_svga_tileinfo_t *graphics_tile_info_common(bx_svga_tileinfo_t *info);
  Bit8u* get_snapshot_buffer(void) {return snapshot_buffer;}
  bool palette_change_common(Bit8u index, Bit8u red, Bit8u green, Bit8u blue);
  void update_drive_status_buttons(void);
  static void mouse_enabled_changed(bool val);
  int register_statusitem(const char *text, bool auto_off=0);
  void unregister_statusitem(int id);
  void statusbar_setitem(int element, bool active, bool w=0);
  void statusbar_setall(bool active);
  static void init_signal_handlers();
  static void toggle_mouse_enable(void);
  bool mouse_toggle_check(Bit32u key, bool pressed);
  const char* get_toggle_info(void);
  Bit8u get_modifier_keys(void);
  Bit8u set_modifier_keys(Bit8u modifier, bool pressed);
  bool parse_user_shortcut(const char *val);





  bool has_gui_console(void) {return console.present;}
  bool console_running(void) {return console.running;}
  void console_refresh(bool force);
  void console_key_enq(Bit8u key);
  int bx_printf(const char *s);
  char* bx_gets(char *s, int size);






  bool has_command_mode(void) {return command_mode.present;}
  bool command_mode_active(void) {return command_mode.active;}
  void set_command_mode(bool active);
  void set_fullscreen_mode(bool active) {fullscreen_mode = active;}

  static void marklog_handler(void);




protected:


  static void floppyA_handler(void);
  static void floppyB_handler(void);
  static void cdrom1_handler(void);
  static void reset_handler(void);
  static void power_handler(void);
  static void copy_handler(void);
  static void paste_handler(void);
  static void snapshot_handler(void);
  static void config_handler(void);



  static void userbutton_handler(void);
  static void save_restore_handler(void);

  void headerbar_click(int x);

  static void make_text_snapshot(char **snapshot, Bit32u *length);
  static Bit32u set_snapshot_mode(bool mode);

  static void led_timer_handler(void *);
  void led_timer(void);

  void console_init(void);
  void console_cleanup(void);



  bool parse_common_gui_options(const char *arg, Bit8u flags);


  bool floppyA_status;
  bool floppyB_status;
  bool cdrom1_status;
  unsigned floppyA_bmap_id, floppyA_eject_bmap_id, floppyA_hbar_id;
  unsigned floppyB_bmap_id, floppyB_eject_bmap_id, floppyB_hbar_id;
  unsigned cdrom1_bmap_id, cdrom1_eject_bmap_id, cdrom1_hbar_id;
  unsigned power_bmap_id, power_hbar_id;
  unsigned reset_bmap_id, reset_hbar_id;
  unsigned copy_bmap_id, copy_hbar_id;
  unsigned paste_bmap_id, paste_hbar_id;
  unsigned snapshot_bmap_id, snapshot_hbar_id;
  unsigned config_bmap_id, config_hbar_id;
  unsigned mouse_bmap_id, nomouse_bmap_id, mouse_hbar_id;
  unsigned user_bmap_id, user_hbar_id;
  unsigned save_restore_bmap_id, save_restore_hbar_id;




  unsigned bx_headerbar_entries;
  struct {
    unsigned bmap_id;
    unsigned xdim;
    unsigned ydim;
    unsigned xorigin;
    unsigned alignment;
    void (*f)(void);
  } bx_headerbar_entry[12];

  Bit8u vga_charmap[2][0x2000];
  bool charmap_updated;
  bool char_changed[2][256];

  unsigned statusitem_count;
  int led_timer_index;
  struct {
    bool in_use;
    char text[8];
    bool active;
    bool mode;
    bool auto_off;
    Bit8u counter;
  } statusitem[10];

  disp_mode_t disp_mode;

  bool new_gfx_api;
  Bit16u host_xres;
  Bit16u host_yres;
  Bit16u host_pitch;
  Bit8u host_bpp;
  Bit8u *framebuffer;

  bool new_text_api;
  Bit16u cursor_address;
  bx_vga_tminfo_t tm_info;

  unsigned max_xres;
  unsigned max_yres;
  unsigned x_tilesize;
  unsigned y_tilesize;

  bool guest_textmode;
  Bit8u guest_fwidth;
  Bit8u guest_fheight;
  Bit16u guest_xres;
  Bit16u guest_yres;
  Bit8u guest_bpp;

  bool snapshot_mode;
  Bit8u *snapshot_buffer;
  struct {
    Bit8u blue;
    Bit8u green;
    Bit8u red;
    Bit8u reserved;
  } palette[256];

  Bit8u keymodstate;

  Bit8u toggle_method;
  Bit32u toggle_keystate;
  char mouse_toggle_text[20];

  Bit32u user_shortcut[4];
  int user_shortcut_len;
  bool user_shortcut_error;

  Bit32u dialog_caps;

  struct {
    bool present;
    bool running;
    Bit8u *screen;
    Bit8u *oldscreen;
    Bit8u saved_fwidth;
    Bit8u saved_fheight;
    Bit16u saved_xres;
    Bit16u saved_yres;
    Bit8u saved_bpp;
    Bit8u saved_palette[32];
    Bit8u saved_charmap[0x2000];
    unsigned cursor_x;
    unsigned cursor_y;
    Bit16u cursor_addr;
    bx_vga_tminfo_t tminfo;
    Bit8u keys[16];
    Bit8u n_keys;
  } console;


  struct {
    bool present;
    bool active;
  } command_mode;
  bool fullscreen_mode;
  Bit32u marker_count;

  struct {

    bool hide_ips;

    bool nokeyrepeat;
  } gui_opts;
};
# 456 "../../gui/gui.h"
enum {
  BX_KEY_CTRL_L,
  BX_KEY_SHIFT_L,

  BX_KEY_F1,
  BX_KEY_F2,
  BX_KEY_F3,
  BX_KEY_F4,
  BX_KEY_F5,
  BX_KEY_F6,
  BX_KEY_F7,
  BX_KEY_F8,
  BX_KEY_F9,
  BX_KEY_F10,
  BX_KEY_F11,
  BX_KEY_F12,

  BX_KEY_CTRL_R,
  BX_KEY_SHIFT_R,
  BX_KEY_CAPS_LOCK,
  BX_KEY_NUM_LOCK,
  BX_KEY_ALT_L,
  BX_KEY_ALT_R,

  BX_KEY_A,
  BX_KEY_B,
  BX_KEY_C,
  BX_KEY_D,
  BX_KEY_E,
  BX_KEY_F,
  BX_KEY_G,
  BX_KEY_H,
  BX_KEY_I,
  BX_KEY_J,
  BX_KEY_K,
  BX_KEY_L,
  BX_KEY_M,
  BX_KEY_N,
  BX_KEY_O,
  BX_KEY_P,
  BX_KEY_Q,
  BX_KEY_R,
  BX_KEY_S,
  BX_KEY_T,
  BX_KEY_U,
  BX_KEY_V,
  BX_KEY_W,
  BX_KEY_X,
  BX_KEY_Y,
  BX_KEY_Z,

  BX_KEY_0,
  BX_KEY_1,
  BX_KEY_2,
  BX_KEY_3,
  BX_KEY_4,
  BX_KEY_5,
  BX_KEY_6,
  BX_KEY_7,
  BX_KEY_8,
  BX_KEY_9,

  BX_KEY_ESC,

  BX_KEY_SPACE,
  BX_KEY_SINGLE_QUOTE,
  BX_KEY_COMMA,
  BX_KEY_PERIOD,
  BX_KEY_SLASH,

  BX_KEY_SEMICOLON,
  BX_KEY_EQUALS,

  BX_KEY_LEFT_BRACKET,
  BX_KEY_BACKSLASH,
  BX_KEY_RIGHT_BRACKET,
  BX_KEY_MINUS,
  BX_KEY_GRAVE,

  BX_KEY_BACKSPACE,
  BX_KEY_ENTER,
  BX_KEY_TAB,

  BX_KEY_LEFT_BACKSLASH,
  BX_KEY_PRINT,
  BX_KEY_SCRL_LOCK,
  BX_KEY_PAUSE,

  BX_KEY_INSERT,
  BX_KEY_DELETE,
  BX_KEY_HOME,
  BX_KEY_END,
  BX_KEY_PAGE_UP,
  BX_KEY_PAGE_DOWN,

  BX_KEY_KP_ADD,
  BX_KEY_KP_SUBTRACT,
  BX_KEY_KP_END,
  BX_KEY_KP_DOWN,
  BX_KEY_KP_PAGE_DOWN,
  BX_KEY_KP_LEFT,
  BX_KEY_KP_RIGHT,
  BX_KEY_KP_HOME,
  BX_KEY_KP_UP,
  BX_KEY_KP_PAGE_UP,
  BX_KEY_KP_INSERT,
  BX_KEY_KP_DELETE,
  BX_KEY_KP_5,

  BX_KEY_UP,
  BX_KEY_DOWN,
  BX_KEY_LEFT,
  BX_KEY_RIGHT,

  BX_KEY_KP_ENTER,
  BX_KEY_KP_MULTIPLY,
  BX_KEY_KP_DIVIDE,

  BX_KEY_WIN_L,
  BX_KEY_WIN_R,
  BX_KEY_MENU,

  BX_KEY_ALT_SYSREQ,
  BX_KEY_CTRL_BREAK,

  BX_KEY_INT_BACK,
  BX_KEY_INT_FORWARD,
  BX_KEY_INT_STOP,
  BX_KEY_INT_MAIL,
  BX_KEY_INT_SEARCH,
  BX_KEY_INT_FAV,
  BX_KEY_INT_HOME,

  BX_KEY_POWER_MYCOMP,
  BX_KEY_POWER_CALC,
  BX_KEY_POWER_SLEEP,
  BX_KEY_POWER_POWER,
  BX_KEY_POWER_WAKE,

  BX_KEY_NBKEYS
};
# 35 "../iodev.h" 2
# 65 "../iodev.h"
typedef Bit32u (*bx_read_handler_t)(void *, Bit32u, unsigned);
typedef void (*bx_write_handler_t)(void *, Bit32u, Bit32u, unsigned);

typedef bool (*bx_kbd_gen_scancode_t)(void *, Bit32u);
typedef Bit8u (*bx_kbd_get_elements_t)(void *);
typedef void (*bx_mouse_enq_t)(void *, int, int, int, unsigned, bool);
typedef void (*bx_mouse_enabled_changed_t)(void *, bool);
# 89 "../iodev.h"
class bx_devmodel_c : public logfunctions {
  public:
  virtual ~bx_devmodel_c() {}
  virtual void init(void) {}
  virtual void reset(unsigned type) {}
  virtual void register_state(void) {}
  virtual void after_restore_state(void) {}



};


class bx_list_c;
class device_image_t;
class cdrom_base_c;
# 128 "../iodev.h"
enum {
  BX_PCI_BAR_TYPE_NONE = 0,
  BX_PCI_BAR_TYPE_MEM = 1,
  BX_PCI_BAR_TYPE_IO = 2
};






typedef struct {
  Bit8u type;
  Bit32u size;
  Bit32u addr;
  union {
    struct {
      memory_handler_t rh;
      memory_handler_t wh;
      const Bit8u *dummy;
    } mem;
    struct {
      bx_read_handler_t rh;
      bx_write_handler_t wh;
      const Bit8u *mask;
    } io;
  };
} bx_pci_bar_t;

class bx_pci_device_c : public bx_devmodel_c {
public:
  bx_pci_device_c(): pci_rom(
# 159 "../iodev.h" 3 4
                            __null
# 159 "../iodev.h"
                                ), pci_rom_size(0) {
    for (int i = 0; i < 6; i++) memset(&pci_bar[i], 0, sizeof(bx_pci_bar_t));
  }
  virtual ~bx_pci_device_c() {
    if (pci_rom != 
# 163 "../iodev.h" 3 4
                  __null
# 163 "../iodev.h"
                      ) delete [] pci_rom;
  }

  virtual Bit32u pci_read_handler(Bit8u address, unsigned io_len);
  void pci_write_handler_common(Bit8u address, Bit32u value, unsigned io_len);
  virtual void pci_write_handler(Bit8u address, Bit32u value, unsigned io_len) {}
  virtual void pci_bar_change_notify(void) {}

  void init_pci_conf(Bit16u vid, Bit16u did, Bit8u rev, Bit32u classc,
                     Bit8u headt, Bit8u intpin);
  void init_bar_io(Bit8u num, Bit16u size, bx_read_handler_t rh,
                   bx_write_handler_t wh, const Bit8u *mask);
  void init_bar_mem(Bit8u num, Bit32u size, memory_handler_t rh, memory_handler_t wh);
  void register_pci_state(bx_list_c *list);
  void after_restore_pci_state(memory_handler_t mem_read_handler);
  void load_pci_rom(const char *path);

  void set_name(const char *name) {pci_name = name;}
  const char* get_name(void) {return pci_name;}

protected:
  const char *pci_name;
  Bit8u pci_conf[256];
  bx_pci_bar_t pci_bar[6];
  Bit8u *pci_rom;
  Bit32u pci_rom_address;
  Bit32u pci_rom_size;
  memory_handler_t pci_rom_read_handler;
};
# 203 "../iodev.h"
class bx_hard_drive_stub_c : public bx_devmodel_c {
public:
  virtual Bit32u virt_read_handler(Bit32u address, unsigned io_len) { return 0; }
  virtual void virt_write_handler(Bit32u address, Bit32u value, unsigned io_len) {}

  virtual bool bmdma_read_sector(Bit8u channel, Bit8u *buffer, Bit32u *sector_size) {
    pluginlog->panic("%s called in %s stub. you must not have loaded the %s plugin", "HD", "bmdma_read_sector", "HD"); return 0;
  }
  virtual bool bmdma_write_sector(Bit8u channel, Bit8u *buffer) {
    pluginlog->panic("%s called in %s stub. you must not have loaded the %s plugin", "HD", "bmdma_write_sector", "HD"); return 0;
  }
  virtual void bmdma_complete(Bit8u channel) {
    pluginlog->panic("%s called in %s stub. you must not have loaded the %s plugin", "HD", "bmdma_complete", "HD");
  }
};

class bx_cmos_stub_c : public bx_devmodel_c {
public:
  virtual Bit32u get_reg(Bit8u reg) {
    pluginlog->panic("%s called in %s stub. you must not have loaded the %s plugin", "cmos", "get_reg", "cmos"); return 0;
  }
  virtual void set_reg(Bit8u reg, Bit32u val) {
    pluginlog->panic("%s called in %s stub. you must not have loaded the %s plugin", "cmos", "set_reg", "cmos");
  }
  virtual void checksum_cmos(void) {
    pluginlog->panic("%s called in %s stub. you must not have loaded the %s plugin", "cmos", "checksum", "cmos");
  }
  virtual void enable_irq(bool enabled) {
    pluginlog->panic("%s called in %s stub. you must not have loaded the %s plugin", "cmos", "enable_irq", "cmos");
  }
};

class bx_pit_stub_c : public bx_devmodel_c {
public:
  virtual void enable_irq(bool enabled) {
    pluginlog->panic("%s called in %s stub. you must not have loaded the %s plugin", "pit", "enable_irq", "pit");
  }
};

class bx_dma_stub_c : public bx_devmodel_c {
public:
  virtual bool registerDMA8Channel(
    unsigned channel,
    Bit16u (* dmaRead)(Bit8u *data_byte, Bit16u maxlen),
    Bit16u (* dmaWrite)(Bit8u *data_byte, Bit16u maxlen),
    const char *name)
  {
    pluginlog->panic("%s called in %s stub. you must not have loaded the %s plugin", "dma", "registerDMA8Channel", "dma"); return false;
  }
  virtual bool registerDMA16Channel(
    unsigned channel,
    Bit16u (* dmaRead)(Bit16u *data_word, Bit16u maxlen),
    Bit16u (* dmaWrite)(Bit16u *data_word, Bit16u maxlen),
    const char *name)
  {
    pluginlog->panic("%s called in %s stub. you must not have loaded the %s plugin", "dma", "registerDMA16Channel", "dma"); return false;
  }
  virtual bool unregisterDMAChannel(unsigned channel) {
    pluginlog->panic("%s called in %s stub. you must not have loaded the %s plugin", "dma", "unregisterDMAChannel", "dma"); return false;
  }
  virtual bool get_TC(void) {
    pluginlog->panic("%s called in %s stub. you must not have loaded the %s plugin", "dma", "get_TC", "dma"); return false;
  }
  virtual void set_DRQ(unsigned channel, bool val) {
    pluginlog->panic("%s called in %s stub. you must not have loaded the %s plugin", "dma", "set_DRQ", "dma");
  }
  virtual void raise_HLDA(void) {
    pluginlog->panic("%s called in %s stub. you must not have loaded the %s plugin", "dma", "raise_HLDA", "dma");
  }
};

class bx_pic_stub_c : public bx_devmodel_c {
public:
  virtual void raise_irq(unsigned irq_no, Bit8u irq_type) {
    pluginlog->panic("%s called in %s stub. you must not have loaded the %s plugin", "pic", "raise_irq", "pic");
  }
  virtual void lower_irq(unsigned irq_no, Bit8u irq_type) {
    pluginlog->panic("%s called in %s stub. you must not have loaded the %s plugin", "pic", "lower_irq", "pic");
  }
  virtual void set_irq_level(unsigned irq_no, Bit8u irq_type, bool level) {
    pluginlog->panic("%s called in %s stub. you must not have loaded the %s plugin", "pic", "set_irq_level", "pic");
  }
  virtual void set_mode(bool ma_sl, Bit8u mode) {
    pluginlog->panic("%s called in %s stub. you must not have loaded the %s plugin", "pic", "set_mode", "pic");
  }
  virtual Bit8u IAC(void) {
    pluginlog->panic("%s called in %s stub. you must not have loaded the %s plugin", "pic", "IAC", "pic"); return 0;
  }
};

class bx_vga_stub_c

: public bx_pci_device_c



{
public:
  virtual void vga_redraw_area(unsigned x0, unsigned y0, unsigned width,
                               unsigned height) {
    pluginlog->panic("%s called in %s stub. you must not have loaded the %s plugin", "vga", "vga_redraw_area", "vga");
  }
  virtual Bit8u mem_read(bx_phy_address addr) {
    pluginlog->panic("%s called in %s stub. you must not have loaded the %s plugin", "vga", "mem_read", "vga"); return 0;
  }
  virtual void mem_write(bx_phy_address addr, Bit8u value) {
    pluginlog->panic("%s called in %s stub. you must not have loaded the %s plugin", "vga", "mem_write", "vga");
  }
  virtual void get_text_snapshot(Bit8u **text_snapshot,
                                 unsigned *txHeight, unsigned *txWidth) {
    pluginlog->panic("%s called in %s stub. you must not have loaded the %s plugin", "vga", "get_text_snapshot", "vga");
  }
  virtual void set_override(bool enabled, void *dev) {
    pluginlog->panic("%s called in %s stub. you must not have loaded the %s plugin", "vga", "set_override", "vga");
  }
  virtual void refresh_display(bool redraw) {
    pluginlog->panic("%s called in %s stub. you must not have loaded the %s plugin", "vga", "refresh_display", "vga");
  }
};

class bx_speaker_stub_c : public bx_devmodel_c {
public:
  virtual void beep_on(float frequency) {
    bx_gui->beep_on(frequency);
  }
  virtual void beep_off() {
    bx_gui->beep_off();
  }
  virtual void set_line(bool level) {}
};


class bx_pci2isa_stub_c : public bx_pci_device_c {
public:
  virtual void pci_set_irq (Bit8u devfunc, unsigned line, bool level) {
    pluginlog->panic("%s called in %s stub. you must not have loaded the %s plugin", "pci2isa", "pci_set_irq", "pci2isa");
  }
};

class bx_pci_ide_stub_c : public bx_pci_device_c {
public:
  virtual bool bmdma_present(void) {
    return 0;
  }
  virtual void bmdma_start_transfer(Bit8u channel) {}
  virtual void bmdma_set_irq(Bit8u channel) {}
};

class bx_acpi_ctrl_stub_c : public bx_pci_device_c {
public:
  virtual void generate_smi(Bit8u value) {}
};
# 366 "../iodev.h"
class bx_ioapic_stub_c : public bx_devmodel_c {
public:
  virtual void set_enabled(bool enabled, Bit16u base_offset) {}
  virtual void receive_eoi(Bit8u vector) {}
  virtual void set_irq_level(Bit8u int_in, bool level) {}
};
# 383 "../iodev.h"
class bx_devices_c : public logfunctions {
public:
  bx_devices_c();
 ~bx_devices_c();


  void init_stubs(void);



  void init(BX_MEM_C *);



  void reset(unsigned type);

  void exit(void);
  void register_state(void);
  void after_restore_state(void);
  BX_MEM_C *mem;
  bool register_io_read_handler(void *this_ptr, bx_read_handler_t f,
                                Bit32u addr, const char *name, Bit8u mask);
  bool unregister_io_read_handler(void *this_ptr, bx_read_handler_t f,
                                  Bit32u addr, Bit8u mask);
  bool register_io_write_handler(void *this_ptr, bx_write_handler_t f,
                                    Bit32u addr, const char *name, Bit8u mask);
  bool unregister_io_write_handler(void *this_ptr, bx_write_handler_t f,
                                   Bit32u addr, Bit8u mask);
  bool register_io_read_handler_range(void *this_ptr, bx_read_handler_t f,
                                      Bit32u begin_addr, Bit32u end_addr,
                                      const char *name, Bit8u mask);
  bool register_io_write_handler_range(void *this_ptr, bx_write_handler_t f,
                                       Bit32u begin_addr, Bit32u end_addr,
                                       const char *name, Bit8u mask);
  bool unregister_io_read_handler_range(void *this_ptr, bx_read_handler_t f,
                                        Bit32u begin, Bit32u end, Bit8u mask);
  bool unregister_io_write_handler_range(void *this_ptr, bx_write_handler_t f,
                                         Bit32u begin, Bit32u end, Bit8u mask);
  bool register_default_io_read_handler(void *this_ptr, bx_read_handler_t f, const char *name, Bit8u mask);
  bool register_default_io_write_handler(void *this_ptr, bx_write_handler_t f, const char *name, Bit8u mask);
  bool register_irq(unsigned irq, const char *name);
  bool unregister_irq(unsigned irq, const char *name);
  Bit32u inp(Bit16u addr, unsigned io_len) ;
  void outp(Bit16u addr, Bit32u value, unsigned io_len) ;

  void register_default_keyboard(void *dev, bx_kbd_gen_scancode_t kbd_gen_scancode,
                                 bx_kbd_get_elements_t kbd_get_elements);
  void register_removable_keyboard(void *dev, bx_kbd_gen_scancode_t kbd_gen_scancode,
                                   bx_kbd_get_elements_t kbd_get_elements,
                                   Bit8u led_mask);
  void unregister_removable_keyboard(void *dev);
  void register_default_mouse(void *dev, bx_mouse_enq_t mouse_enq, bx_mouse_enabled_changed_t mouse_enabled_changed);
  void register_removable_mouse(void *dev, bx_mouse_enq_t mouse_enq, bx_mouse_enabled_changed_t mouse_enabled_changed);
  void unregister_removable_mouse(void *dev);
  void gen_scancode(Bit32u key);
  Bit8u kbd_get_elements(void);
  void release_keys(void);
  void paste_bytes(Bit8u *data, Bit32s length);
  void kbd_set_indicator(Bit8u devid, Bit8u ledid, bool state);
  void mouse_enabled_changed(bool enabled);
  void mouse_motion(int delta_x, int delta_y, int delta_z, unsigned button_state, bool absxy);
  void add_sound_device(void);
  void remove_sound_device(void);


  Bit32u pci_get_confAddr(void) {return pci.confAddr;}
  int pci_get_slot_from_dev(Bit8u device);
  bool register_pci_handlers(bx_pci_device_c *device, Bit8u *devfunc,
                             const char *name, const char *descr, Bit8u bus = 0);
  bool pci_set_base_mem(void *this_ptr, memory_handler_t f1, memory_handler_t f2,
                        Bit32u *addr, Bit8u *pci_conf, unsigned size);
  bool pci_set_base_io(void *this_ptr, bx_read_handler_t f1, bx_write_handler_t f2,
                       Bit32u *addr, Bit8u *pci_conf, unsigned size,
                       const Bit8u *iomask, const char *name);

  bool is_agp_present();

  static void timer_handler(void *);
  void timer(void);

  bx_cmos_stub_c *pluginCmosDevice;
  bx_dma_stub_c *pluginDmaDevice;
  bx_hard_drive_stub_c *pluginHardDrive;
  bx_pic_stub_c *pluginPicDevice;
  bx_pit_stub_c *pluginPitDevice;
  bx_speaker_stub_c *pluginSpeaker;
  bx_vga_stub_c *pluginVgaDevice;




  bx_ioapic_stub_c *pluginIOAPIC;





  bx_pci2isa_stub_c *pluginPci2IsaBridge;
  bx_pci_ide_stub_c *pluginPciIdeController;
  bx_acpi_ctrl_stub_c *pluginACPIController;




  bx_cmos_stub_c stubCmos;
  bx_dma_stub_c stubDma;
  bx_hard_drive_stub_c stubHardDrive;
  bx_pic_stub_c stubPic;
  bx_pit_stub_c stubPit;
  bx_speaker_stub_c stubSpeaker;
  bx_vga_stub_c stubVga;




  bx_ioapic_stub_c stubIOAPIC;





  bx_pci2isa_stub_c stubPci2Isa;
  bx_pci_ide_stub_c stubPciIde;
  bx_acpi_ctrl_stub_c stubACPIController;
# 515 "../iodev.h"
  Bit8u* bulkIOHostAddr;
  unsigned bulkIOQuantumsRequested;
  unsigned bulkIOQuantumsTransferred;

private:

  struct io_handler_struct {
    struct io_handler_struct *next;
    struct io_handler_struct *prev;
    void *funct;
    void *this_ptr;
    char *handler_name;
    int usage_count;
    Bit8u mask;
  };
  struct io_handler_struct io_read_handlers;
  struct io_handler_struct io_write_handlers;

  struct io_handler_struct **read_port_to_handler;
  struct io_handler_struct **write_port_to_handler;



  char *irq_handler_name[16];

  static Bit32u read_handler(void *this_ptr, Bit32u address, unsigned io_len);
  static void write_handler(void *this_ptr, Bit32u address, Bit32u value, unsigned io_len);
  static Bit32u read(Bit32u address, unsigned io_len);
  static void write(Bit32u address, Bit32u value, unsigned io_len);

  static Bit32u default_read_handler(void *this_ptr, Bit32u address, unsigned io_len);
  static void default_write_handler(void *this_ptr, Bit32u address, Bit32u value, unsigned io_len);


  static Bit64s param_handler(bx_param_c *param, bool set, Bit64s val);
  void paste_delay_changed(Bit32u value);
  void service_paste_buf();

  bool mouse_captured;
  Bit8u mouse_type;
  struct {
    void *dev;
    bx_mouse_enq_t enq_event;
    bx_mouse_enabled_changed_t enabled_changed;
  } bx_mouse[2];

  struct {
    void *dev;
    bx_kbd_gen_scancode_t gen_scancode;
    bx_kbd_get_elements_t get_elements;
    Bit8u led_mask;
    bool bxkey_state[BX_KEY_NBKEYS];
  } bx_keyboard[2];
# 590 "../iodev.h"
  struct {
    Bit8u *buf;
    Bit32u buf_len;
    Bit32u buf_ptr;
    Bit32u delay;
    Bit32u counter;
    bool service;
    bool stop;
  } paste;

  struct {
    bool enabled;

    Bit32u advopts;
    Bit8u handler_id[0x101];
    struct {
      bx_pci_device_c *handler;
    } pci_handler[20];
    unsigned num_pci_handlers;

    const Bit8u *map_slot_to_dev;
    bool slot_used[5];

    Bit32u confAddr;

  } pci;

  int timer_handle;
  int statusbar_id[3];

  Bit8u sound_device_count;

  bool is_harddrv_enabled();
};


inline void DEV_MEM_READ_PHYSICAL(bx_phy_address phy_addr, unsigned len, Bit8u *ptr)
{
  unsigned remainingInPage = 0x1000 - (phy_addr & 0xfff);
  if (len <= remainingInPage) {
    (&bx_mem)->readPhysicalPage(
# 630 "../iodev.h" 3 4
                               __null
# 630 "../iodev.h"
                                   , phy_addr, len, ptr);
  }
  else {
    (&bx_mem)->readPhysicalPage(
# 633 "../iodev.h" 3 4
                               __null
# 633 "../iodev.h"
                                   , phy_addr, remainingInPage, ptr);
    ptr += remainingInPage;
    phy_addr += remainingInPage;
    len -= remainingInPage;
    (&bx_mem)->readPhysicalPage(
# 637 "../iodev.h" 3 4
                               __null
# 637 "../iodev.h"
                                   , phy_addr, len, ptr);
  }
}

inline void DEV_MEM_READ_PHYSICAL_DMA(bx_phy_address phy_addr, unsigned len, Bit8u *ptr)
{
  while(len > 0) {
    unsigned remainingInPage = 0x1000 - (phy_addr & 0xfff);
    if (len < remainingInPage) remainingInPage = len;
    (&bx_mem)->dmaReadPhysicalPage(phy_addr, remainingInPage, ptr);
    ptr += remainingInPage;
    phy_addr += remainingInPage;
    len -= remainingInPage;
  }
}


inline void DEV_MEM_WRITE_PHYSICAL(bx_phy_address phy_addr, unsigned len, Bit8u *ptr)
{
  unsigned remainingInPage = 0x1000 - (phy_addr & 0xfff);
  if (len <= remainingInPage) {
    (&bx_mem)->writePhysicalPage(
# 658 "../iodev.h" 3 4
                                __null
# 658 "../iodev.h"
                                    , phy_addr, len, ptr);
  }
  else {
    (&bx_mem)->writePhysicalPage(
# 661 "../iodev.h" 3 4
                                __null
# 661 "../iodev.h"
                                    , phy_addr, remainingInPage, ptr);
    ptr += remainingInPage;
    phy_addr += remainingInPage;
    len -= remainingInPage;
    (&bx_mem)->writePhysicalPage(
# 665 "../iodev.h" 3 4
                                __null
# 665 "../iodev.h"
                                    , phy_addr, len, ptr);
  }
}

inline void DEV_MEM_WRITE_PHYSICAL_DMA(bx_phy_address phy_addr, unsigned len, Bit8u *ptr)
{
  while(len > 0) {
    unsigned remainingInPage = 0x1000 - (phy_addr & 0xfff);
    if (len < remainingInPage) remainingInPage = len;
    (&bx_mem)->dmaWritePhysicalPage(phy_addr, remainingInPage, ptr);
    ptr += remainingInPage;
    phy_addr += remainingInPage;
    len -= remainingInPage;
  }
}

 extern bx_devices_c bx_devices;
# 29 "voodoorush.cc" 2
# 1 "vgacore.h" 1
# 63 "vgacore.h"
typedef struct {
  Bit16u htotal;
  Bit16u vtotal;
  Bit16u vbstart;
  Bit16u vrstart;
} bx_crtc_params_t;

extern const Bit8u ccdat[16][4];


class bx_nonvga_device_c : public bx_pci_device_c {
public:
  virtual void redraw_area(unsigned x0, unsigned y0,
                           unsigned width, unsigned height) {}
  virtual void refresh_display(bool redraw) {}
  virtual bool update(void) {return false;}
  virtual Bit32u get_vtotal_usec(void) {return 0;}
};


class bx_vgacore_c : public bx_vga_stub_c {
public:
  bx_vgacore_c();
  virtual ~bx_vgacore_c();
  virtual void init(void);
  virtual void reset(unsigned type) {}
  static bool mem_read_handler(bx_phy_address addr, unsigned len, void *data, void *param);
  static bool mem_write_handler(bx_phy_address addr, unsigned len, void *data, void *param);
  virtual Bit8u mem_read(bx_phy_address addr);
  virtual void mem_write(bx_phy_address addr, Bit8u value);
  virtual void set_override(bool enabled, void *dev);
  void vgacore_register_state(bx_list_c *parent);
  virtual void after_restore_state(void);




  virtual void vga_redraw_area(unsigned x0, unsigned y0, unsigned width,
                                 unsigned height);
  virtual void redraw_area(unsigned x0, unsigned y0, unsigned width,
                             unsigned height);
  virtual void refresh_display(bool redraw);
  virtual void get_text_snapshot(Bit8u **text_snapshot, unsigned *txHeight,
                                   unsigned *txWidth);
  virtual bool init_vga_extension(void) {return 0;}
  virtual void get_crtc_params(bx_crtc_params_t *crtcp, Bit32u *vclock);

  virtual bool get_update_mode(void) {return update_mode_vsync;}
  virtual void set_update_timer(Bit32u usec);
  virtual void start_vertical_timer(void);
  static void vga_timer_handler(void *);
  static void vertical_timer_handler(void *);
  virtual void vertical_timer(void);
  static Bit64s vga_param_handler(bx_param_c *param, bool set, Bit64s val);

protected:
  void init_standard_vga(void);
  void init_gui(void);
  void init_iohandlers(bx_read_handler_t f_read, bx_write_handler_t f_write, const char *name);
  void init_systemtimer();

  static Bit32u read_handler(void *this_ptr, Bit32u address, unsigned io_len);
  static void write_handler(void *this_ptr, Bit32u address, Bit32u value, unsigned io_len);

  Bit32u read(Bit32u address, unsigned io_len);
  void write(Bit32u address, Bit32u value, unsigned io_len, bool no_log);

  Bit8u get_vga_pixel(Bit16u x, Bit16u y, Bit32u raddr, Bit16u lc, bool bs, Bit8u *vgamem_ptr);
  virtual void update(void);
  void determine_screen_dimensions(unsigned *piHeight, unsigned *piWidth);
  void calculate_retrace_timing(void);
  bool skip_update(void);
  void update_charmap(void);

  struct {
    struct {
      bool color_emulation;

      bool enable_ram;
      Bit8u clock_select;
      bool select_high_bank;

      bool horiz_sync_pol;
      bool vert_sync_pol;





    } misc_output;

    Bit8u feature_control;

    struct {
      Bit8u address;
      Bit8u reg[0x19];
      bool write_protect;
      Bit16u start_addr;
      Bit8u max_reg;
    } CRTC;

    struct {
      bool flip_flop;
      Bit8u address;
      bool video_enabled;
      Bit8u palette_reg[16];
      Bit8u overscan_color;
      Bit8u color_plane_enable;
      Bit8u horiz_pel_panning;
      Bit8u color_select;
      struct {
        bool graphics_alpha;
        bool display_type;
        bool enable_line_graphics;
        bool blink_intensity;
        bool pixel_panning_compat;
        bool pixel_clock_select;
        bool internal_palette_size;
      } mode_ctrl;
    } attribute_ctrl;

    struct {
      Bit8u write_data_register;
      Bit8u write_data_cycle;
      Bit8u read_data_register;
      Bit8u read_data_cycle;
      Bit8u dac_state;
      struct {
        Bit8u red;
        Bit8u green;
        Bit8u blue;
      } data[256];
      Bit8u mask;
    } pel;

    struct {
      Bit8u index;
      Bit8u set_reset;
      Bit8u enable_set_reset;
      Bit8u color_compare;
      Bit8u data_rotate;
      Bit8u raster_op;
      Bit8u read_map_select;
      Bit8u write_mode;
      Bit32u read_mode;
      bool odd_even;
      bool chain_odd_even;
      Bit8u shift_reg;
      bool graphics_alpha;
      Bit8u memory_mapping;




      Bit8u color_dont_care;
      Bit8u bitmask;
      Bit8u latch[4];
    } graphics_ctrl;

    struct {
      Bit8u index;
      Bit8u map_mask;
      bool reset1;
      bool reset2;
      Bit8u reg1;
      Bit8u char_map_select;
      bool extended_mem;
      bool odd_even_dis;
      bool chain_four;
      bool clear_screen;
    } sequencer;

    bool vga_enabled;
    Bit8u vga_mem_updated;
    Bit16u line_offset;
    Bit16u line_compare;
    Bit16u vertical_display_end;
    unsigned blink_counter;
    bool *vga_tile_updated;
    Bit8u *memory;
    Bit32u memsize;
    Bit32u vgamem_mask;
    bool text_buffer_update;
    Bit8u *text_buffer;
    Bit8u *text_snapshot;
    Bit8u tile[16 * 24 * 4];
    Bit16u charmap_address1;
    Bit16u charmap_address2;
    bool x_dotclockdiv2;
    bool y_doublescan;

    Bit32u vclk[4];
    Bit32u htotal_usec;
    Bit32u hbstart_usec;
    Bit32u hbend_usec;
    Bit32u vtotal_usec;
    Bit32u vblank_usec;
    Bit32u vrstart_usec;
    Bit32u vrend_usec;
    Bit64u display_start_usec;

    Bit8u dac_shift;
    Bit32u ext_offset;
    Bit32u ext_start_addr;
    bool ext_y_dblsize;

    Bit16u last_xres;
    Bit16u last_yres;
    Bit8u last_bpp;
    Bit8u last_fw;
    Bit8u last_fh;

    Bit16u max_xres;
    Bit16u max_yres;
    Bit16u num_x_tiles;
    Bit16u num_y_tiles;

    bool vga_override;

    bx_nonvga_device_c *nvgadev;

  } s;


  int update_timer_id;
  Bit32u vga_update_interval;
  bool update_realtime;
  bool update_mode_vsync;

  int vga_vtimer_id;
  bool vsync_realtime;
  Bit8u vtimer_toggle;
  Bit32u vtimer_interval[2];

  bx_param_enum_c *vga_ext;
  bool pci_enabled;
};
# 30 "voodoorush.cc" 2


# 1 "bitblt.h" 1
# 24 "bitblt.h"
typedef void (*bx_bitblt_rop_t)(
    Bit8u *dst,const Bit8u *src,
    int dstpitch,int srcpitch,
    int bltwidth,int bltheight);
# 70 "bitblt.h"
static void bitblt_rop_fwd_0( Bit8u *dst,const Bit8u *src, int dstpitch,int srcpitch, int bltwidth,int bltheight) { int x,y; dstpitch -= bltwidth; srcpitch -= bltwidth; for (y = 0; y < bltheight; y++) { for (x = 0; x < bltwidth; x++) { *dst = 0; dst++; src++; } dst += dstpitch; src += srcpitch; } }
static void bitblt_rop_fwd_src_and_dst( Bit8u *dst,const Bit8u *src, int dstpitch,int srcpitch, int bltwidth,int bltheight) { int x,y; dstpitch -= bltwidth; srcpitch -= bltwidth; for (y = 0; y < bltheight; y++) { for (x = 0; x < bltwidth; x++) { *dst = (*src) & (*dst); dst++; src++; } dst += dstpitch; src += srcpitch; } }
static void bitblt_rop_fwd_nop( Bit8u *dst,const Bit8u *src, int dstpitch,int srcpitch, int bltwidth,int bltheight) { int x,y; dstpitch -= bltwidth; srcpitch -= bltwidth; for (y = 0; y < bltheight; y++) { for (x = 0; x < bltwidth; x++) { (void)0; dst++; src++; } dst += dstpitch; src += srcpitch; } }
static void bitblt_rop_fwd_src_and_notdst( Bit8u *dst,const Bit8u *src, int dstpitch,int srcpitch, int bltwidth,int bltheight) { int x,y; dstpitch -= bltwidth; srcpitch -= bltwidth; for (y = 0; y < bltheight; y++) { for (x = 0; x < bltwidth; x++) { *dst = (*src) & (~(*dst)); dst++; src++; } dst += dstpitch; src += srcpitch; } }
static void bitblt_rop_fwd_notdst( Bit8u *dst,const Bit8u *src, int dstpitch,int srcpitch, int bltwidth,int bltheight) { int x,y; dstpitch -= bltwidth; srcpitch -= bltwidth; for (y = 0; y < bltheight; y++) { for (x = 0; x < bltwidth; x++) { *dst = ~(*dst); dst++; src++; } dst += dstpitch; src += srcpitch; } }
static void bitblt_rop_fwd_src( Bit8u *dst,const Bit8u *src, int dstpitch,int srcpitch, int bltwidth,int bltheight) { int x,y; dstpitch -= bltwidth; srcpitch -= bltwidth; for (y = 0; y < bltheight; y++) { for (x = 0; x < bltwidth; x++) { *dst = *src; dst++; src++; } dst += dstpitch; src += srcpitch; } }
static void bitblt_rop_fwd_1( Bit8u *dst,const Bit8u *src, int dstpitch,int srcpitch, int bltwidth,int bltheight) { int x,y; dstpitch -= bltwidth; srcpitch -= bltwidth; for (y = 0; y < bltheight; y++) { for (x = 0; x < bltwidth; x++) { *dst = 0xff; dst++; src++; } dst += dstpitch; src += srcpitch; } }
static void bitblt_rop_fwd_notsrc_and_dst( Bit8u *dst,const Bit8u *src, int dstpitch,int srcpitch, int bltwidth,int bltheight) { int x,y; dstpitch -= bltwidth; srcpitch -= bltwidth; for (y = 0; y < bltheight; y++) { for (x = 0; x < bltwidth; x++) { *dst = (~(*src)) & (*dst); dst++; src++; } dst += dstpitch; src += srcpitch; } }
static void bitblt_rop_fwd_src_xor_dst( Bit8u *dst,const Bit8u *src, int dstpitch,int srcpitch, int bltwidth,int bltheight) { int x,y; dstpitch -= bltwidth; srcpitch -= bltwidth; for (y = 0; y < bltheight; y++) { for (x = 0; x < bltwidth; x++) { *dst = (*src) ^ (*dst); dst++; src++; } dst += dstpitch; src += srcpitch; } }
static void bitblt_rop_fwd_src_or_dst( Bit8u *dst,const Bit8u *src, int dstpitch,int srcpitch, int bltwidth,int bltheight) { int x,y; dstpitch -= bltwidth; srcpitch -= bltwidth; for (y = 0; y < bltheight; y++) { for (x = 0; x < bltwidth; x++) { *dst = (*src) | (*dst); dst++; src++; } dst += dstpitch; src += srcpitch; } }
static void bitblt_rop_fwd_notsrc_or_notdst( Bit8u *dst,const Bit8u *src, int dstpitch,int srcpitch, int bltwidth,int bltheight) { int x,y; dstpitch -= bltwidth; srcpitch -= bltwidth; for (y = 0; y < bltheight; y++) { for (x = 0; x < bltwidth; x++) { *dst = (~(*src)) | (~(*dst)); dst++; src++; } dst += dstpitch; src += srcpitch; } }
static void bitblt_rop_fwd_src_notxor_dst( Bit8u *dst,const Bit8u *src, int dstpitch,int srcpitch, int bltwidth,int bltheight) { int x,y; dstpitch -= bltwidth; srcpitch -= bltwidth; for (y = 0; y < bltheight; y++) { for (x = 0; x < bltwidth; x++) { *dst = ~((*src) ^ (*dst)); dst++; src++; } dst += dstpitch; src += srcpitch; } }
static void bitblt_rop_fwd_src_or_notdst( Bit8u *dst,const Bit8u *src, int dstpitch,int srcpitch, int bltwidth,int bltheight) { int x,y; dstpitch -= bltwidth; srcpitch -= bltwidth; for (y = 0; y < bltheight; y++) { for (x = 0; x < bltwidth; x++) { *dst = (*src) | (~(*dst)); dst++; src++; } dst += dstpitch; src += srcpitch; } }
static void bitblt_rop_fwd_notsrc( Bit8u *dst,const Bit8u *src, int dstpitch,int srcpitch, int bltwidth,int bltheight) { int x,y; dstpitch -= bltwidth; srcpitch -= bltwidth; for (y = 0; y < bltheight; y++) { for (x = 0; x < bltwidth; x++) { *dst = (~(*src)); dst++; src++; } dst += dstpitch; src += srcpitch; } }
static void bitblt_rop_fwd_notsrc_or_dst( Bit8u *dst,const Bit8u *src, int dstpitch,int srcpitch, int bltwidth,int bltheight) { int x,y; dstpitch -= bltwidth; srcpitch -= bltwidth; for (y = 0; y < bltheight; y++) { for (x = 0; x < bltwidth; x++) { *dst = (~(*src)) | (*dst); dst++; src++; } dst += dstpitch; src += srcpitch; } }
static void bitblt_rop_fwd_notsrc_and_notdst( Bit8u *dst,const Bit8u *src, int dstpitch,int srcpitch, int bltwidth,int bltheight) { int x,y; dstpitch -= bltwidth; srcpitch -= bltwidth; for (y = 0; y < bltheight; y++) { for (x = 0; x < bltwidth; x++) { *dst = (~(*src)) & (~(*dst)); dst++; src++; } dst += dstpitch; src += srcpitch; } }



static void bitblt_rop_bkwd_0( Bit8u *dst,const Bit8u *src, int dstpitch,int srcpitch, int bltwidth,int bltheight) { int x,y; dstpitch += bltwidth; srcpitch += bltwidth; for (y = 0; y < bltheight; y++) { for (x = 0; x < bltwidth; x++) { *dst = 0; dst--; src--; } dst += dstpitch; src += srcpitch; } }
static void bitblt_rop_bkwd_src_and_dst( Bit8u *dst,const Bit8u *src, int dstpitch,int srcpitch, int bltwidth,int bltheight) { int x,y; dstpitch += bltwidth; srcpitch += bltwidth; for (y = 0; y < bltheight; y++) { for (x = 0; x < bltwidth; x++) { *dst = (*src) & (*dst); dst--; src--; } dst += dstpitch; src += srcpitch; } }
static void bitblt_rop_bkwd_nop( Bit8u *dst,const Bit8u *src, int dstpitch,int srcpitch, int bltwidth,int bltheight) { int x,y; dstpitch += bltwidth; srcpitch += bltwidth; for (y = 0; y < bltheight; y++) { for (x = 0; x < bltwidth; x++) { (void)0; dst--; src--; } dst += dstpitch; src += srcpitch; } }
static void bitblt_rop_bkwd_src_and_notdst( Bit8u *dst,const Bit8u *src, int dstpitch,int srcpitch, int bltwidth,int bltheight) { int x,y; dstpitch += bltwidth; srcpitch += bltwidth; for (y = 0; y < bltheight; y++) { for (x = 0; x < bltwidth; x++) { *dst = (*src) & (~(*dst)); dst--; src--; } dst += dstpitch; src += srcpitch; } }
static void bitblt_rop_bkwd_notdst( Bit8u *dst,const Bit8u *src, int dstpitch,int srcpitch, int bltwidth,int bltheight) { int x,y; dstpitch += bltwidth; srcpitch += bltwidth; for (y = 0; y < bltheight; y++) { for (x = 0; x < bltwidth; x++) { *dst = ~(*dst); dst--; src--; } dst += dstpitch; src += srcpitch; } }
static void bitblt_rop_bkwd_src( Bit8u *dst,const Bit8u *src, int dstpitch,int srcpitch, int bltwidth,int bltheight) { int x,y; dstpitch += bltwidth; srcpitch += bltwidth; for (y = 0; y < bltheight; y++) { for (x = 0; x < bltwidth; x++) { *dst = *src; dst--; src--; } dst += dstpitch; src += srcpitch; } }
static void bitblt_rop_bkwd_1( Bit8u *dst,const Bit8u *src, int dstpitch,int srcpitch, int bltwidth,int bltheight) { int x,y; dstpitch += bltwidth; srcpitch += bltwidth; for (y = 0; y < bltheight; y++) { for (x = 0; x < bltwidth; x++) { *dst = 0xff; dst--; src--; } dst += dstpitch; src += srcpitch; } }
static void bitblt_rop_bkwd_notsrc_and_dst( Bit8u *dst,const Bit8u *src, int dstpitch,int srcpitch, int bltwidth,int bltheight) { int x,y; dstpitch += bltwidth; srcpitch += bltwidth; for (y = 0; y < bltheight; y++) { for (x = 0; x < bltwidth; x++) { *dst = (~(*src)) & (*dst); dst--; src--; } dst += dstpitch; src += srcpitch; } }
static void bitblt_rop_bkwd_src_xor_dst( Bit8u *dst,const Bit8u *src, int dstpitch,int srcpitch, int bltwidth,int bltheight) { int x,y; dstpitch += bltwidth; srcpitch += bltwidth; for (y = 0; y < bltheight; y++) { for (x = 0; x < bltwidth; x++) { *dst = (*src) ^ (*dst); dst--; src--; } dst += dstpitch; src += srcpitch; } }
static void bitblt_rop_bkwd_src_or_dst( Bit8u *dst,const Bit8u *src, int dstpitch,int srcpitch, int bltwidth,int bltheight) { int x,y; dstpitch += bltwidth; srcpitch += bltwidth; for (y = 0; y < bltheight; y++) { for (x = 0; x < bltwidth; x++) { *dst = (*src) | (*dst); dst--; src--; } dst += dstpitch; src += srcpitch; } }
static void bitblt_rop_bkwd_notsrc_or_notdst( Bit8u *dst,const Bit8u *src, int dstpitch,int srcpitch, int bltwidth,int bltheight) { int x,y; dstpitch += bltwidth; srcpitch += bltwidth; for (y = 0; y < bltheight; y++) { for (x = 0; x < bltwidth; x++) { *dst = (~(*src)) | (~(*dst)); dst--; src--; } dst += dstpitch; src += srcpitch; } }
static void bitblt_rop_bkwd_src_notxor_dst( Bit8u *dst,const Bit8u *src, int dstpitch,int srcpitch, int bltwidth,int bltheight) { int x,y; dstpitch += bltwidth; srcpitch += bltwidth; for (y = 0; y < bltheight; y++) { for (x = 0; x < bltwidth; x++) { *dst = ~((*src) ^ (*dst)); dst--; src--; } dst += dstpitch; src += srcpitch; } }
static void bitblt_rop_bkwd_src_or_notdst( Bit8u *dst,const Bit8u *src, int dstpitch,int srcpitch, int bltwidth,int bltheight) { int x,y; dstpitch += bltwidth; srcpitch += bltwidth; for (y = 0; y < bltheight; y++) { for (x = 0; x < bltwidth; x++) { *dst = (*src) | (~(*dst)); dst--; src--; } dst += dstpitch; src += srcpitch; } }
static void bitblt_rop_bkwd_notsrc( Bit8u *dst,const Bit8u *src, int dstpitch,int srcpitch, int bltwidth,int bltheight) { int x,y; dstpitch += bltwidth; srcpitch += bltwidth; for (y = 0; y < bltheight; y++) { for (x = 0; x < bltwidth; x++) { *dst = (~(*src)); dst--; src--; } dst += dstpitch; src += srcpitch; } }
static void bitblt_rop_bkwd_notsrc_or_dst( Bit8u *dst,const Bit8u *src, int dstpitch,int srcpitch, int bltwidth,int bltheight) { int x,y; dstpitch += bltwidth; srcpitch += bltwidth; for (y = 0; y < bltheight; y++) { for (x = 0; x < bltwidth; x++) { *dst = (~(*src)) | (*dst); dst--; src--; } dst += dstpitch; src += srcpitch; } }
static void bitblt_rop_bkwd_notsrc_and_notdst( Bit8u *dst,const Bit8u *src, int dstpitch,int srcpitch, int bltwidth,int bltheight) { int x,y; dstpitch += bltwidth; srcpitch += bltwidth; for (y = 0; y < bltheight; y++) { for (x = 0; x < bltwidth; x++) { *dst = (~(*src)) & (~(*dst)); dst--; src--; } dst += dstpitch; src += srcpitch; } }
# 33 "voodoorush.cc" 2
# 1 "ddc.h" 1
# 25 "ddc.h"
class bx_ddc_c : public logfunctions {
public:
  bx_ddc_c();
  virtual ~bx_ddc_c();

  void init(void);
  Bit8u read(void);
  void write(bool dck, bool dda);

private:

  Bit8u get_edid_byte(void);

  struct {
    bool init_done;
    Bit8u ddc_mode;
    bool DCKhost;
    bool DDAhost;
    bool DDAmon;
    Bit8u ddc_stage;
    Bit8u ddc_bitshift;
    bool ddc_ack;
    bool ddc_rw;
    Bit8u ddc_byte;
    Bit8u edid_index;
    bool edid_extblock;
    Bit8u edid_data[256];
  } s;
};
# 34 "voodoorush.cc" 2
# 1 "pxextract.h" 1
# 35 "voodoorush.cc" 2
# 1 "voodoorush.h" 1
# 36 "voodoorush.cc" 2
# 1 "../virt_timer.h" 1
# 30 "../virt_timer.h"
class bx_virt_timer_c : public logfunctions {
private:

  struct {
    bool inUse;
    Bit64u period;
    Bit64u timeToFire;
    bool active;
    bool continuous;
    bool realtime;
    bx_timer_handler_t funct;


    void *this_ptr;

    char id[32];
  } timer[(32)];

  unsigned numTimers;

  struct {

    Bit64u current_timers_time;
    Bit64u timers_next_event_time;
    Bit64u last_sequential_time;


    Bit64u virtual_next_event_time;
    Bit64u current_virtual_time;

    int system_timer_id;
  } s[2];

  bool in_timer_handler;


  Bit64u ips;

  bool init_done;


  Bit64u last_real_time;
  Bit64u total_real_usec;
  Bit64u last_realtime_delta;
  Bit64u real_time_delay;

  Bit64u last_usec;
  Bit64u usec_per_second;
  Bit64u stored_delta;
  Bit64u last_system_usec;
  Bit64u em_last_realtime;

  Bit64u total_ticks;
  Bit64u last_realtime_ticks;
  Bit64u ticks_per_second;





  static const Bit64u NullTimerInterval;
  static void nullTimer(void* this_ptr);


  void periodic(Bit64u time_passed, bool mode);


  void next_event_time_update(bool mode);



  void advance_virtual_time(Bit64u time_passed, bool mode);

public:

  bx_virt_timer_c();
  virtual ~bx_virt_timer_c() {}



  Bit64u time_usec(bool mode);




  Bit64u time_usec_sequential(bool mode);



  int register_timer(void *this_ptr, bx_timer_handler_t handler,
                         Bit32u useconds, bool continuous,
                         bool active, bool realtime, const char *id);


  bool unregisterTimer(unsigned timerID);

  void start_timers(void);


  void activate_timer(unsigned timer_index, Bit32u useconds, bool continuous);


  void deactivate_timer(unsigned timer_index);



  static void pc_system_timer_handler_0(void* this_ptr);
  static void pc_system_timer_handler_1(void* this_ptr);


  void timer_handler(bool mode);


  void setup(void);


  void init(void);

  void register_state(void);



  void set_realtime_delay(void);
};

 extern bx_virt_timer_c bx_virt_timer;
# 37 "voodoorush.cc" 2
# 1 "../../bxthread.h" 1
# 44 "../../bxthread.h"
# 1 "/usr/include/pthread.h" 1 3 4
# 22 "/usr/include/pthread.h" 3 4
# 1 "/usr/include/sched.h" 1 3 4
# 29 "/usr/include/sched.h" 3 4
# 1 "/usr/lib/gcc/x86_64-linux-gnu/14/include/stddef.h" 1 3 4
# 30 "/usr/include/sched.h" 2 3 4
# 43 "/usr/include/sched.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/sched.h" 1 3 4
# 63 "/usr/include/x86_64-linux-gnu/bits/sched.h" 3 4
# 1 "/usr/include/linux/sched/types.h" 1 3 4
# 98 "/usr/include/linux/sched/types.h" 3 4

# 98 "/usr/include/linux/sched/types.h" 3 4
struct sched_attr {
 __u32 size;

 __u32 sched_policy;
 __u64 sched_flags;


 __s32 sched_nice;


 __u32 sched_priority;


 __u64 sched_runtime;
 __u64 sched_deadline;
 __u64 sched_period;


 __u32 sched_util_min;
 __u32 sched_util_max;

};
# 64 "/usr/include/x86_64-linux-gnu/bits/sched.h" 2 3 4
# 126 "/usr/include/x86_64-linux-gnu/bits/sched.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/types/struct_sched_param.h" 1 3 4
# 23 "/usr/include/x86_64-linux-gnu/bits/types/struct_sched_param.h" 3 4
struct sched_param
{
  int sched_priority;
};
# 127 "/usr/include/x86_64-linux-gnu/bits/sched.h" 2 3 4

extern "C" {



extern int clone (int (*__fn) (void *__arg), void *__child_stack,
    int __flags, void *__arg, ...) noexcept (true);


extern int unshare (int __flags) noexcept (true);


extern int sched_getcpu (void) noexcept (true);


extern int getcpu (unsigned int *, unsigned int *) noexcept (true);


extern int setns (int __fd, int __nstype) noexcept (true);


int sched_setattr (pid_t tid, struct sched_attr *attr, unsigned int flags)
  noexcept (true) __attribute__ ((__nonnull__ (2)));



int sched_getattr (pid_t tid, struct sched_attr *attr, unsigned int size,
     unsigned int flags)
  noexcept (true) __attribute__ ((__nonnull__ (2)));



}
# 44 "/usr/include/sched.h" 2 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/cpu-set.h" 1 3 4
# 32 "/usr/include/x86_64-linux-gnu/bits/cpu-set.h" 3 4
typedef unsigned long int __cpu_mask;






typedef struct
{
  __cpu_mask __bits[1024 / (8 * sizeof (__cpu_mask))];
} cpu_set_t;
# 115 "/usr/include/x86_64-linux-gnu/bits/cpu-set.h" 3 4
extern "C" {

extern int __sched_cpucount (size_t __setsize, const cpu_set_t *__setp)
     noexcept (true);
extern cpu_set_t *__sched_cpualloc (size_t __count) noexcept (true) __attribute__ ((__warn_unused_result__));
extern void __sched_cpufree (cpu_set_t *__set) noexcept (true);

}
# 45 "/usr/include/sched.h" 2 3 4






extern "C" {


extern int sched_setparam (__pid_t __pid, const struct sched_param *__param)
     noexcept (true);


extern int sched_getparam (__pid_t __pid, struct sched_param *__param) noexcept (true);


extern int sched_setscheduler (__pid_t __pid, int __policy,
          const struct sched_param *__param) noexcept (true);


extern int sched_getscheduler (__pid_t __pid) noexcept (true);


extern int sched_yield (void) noexcept (true);


extern int sched_get_priority_max (int __algorithm) noexcept (true);


extern int sched_get_priority_min (int __algorithm) noexcept (true);



extern int sched_rr_get_interval (__pid_t __pid, struct timespec *__t) noexcept (true);
# 130 "/usr/include/sched.h" 3 4
extern int sched_setaffinity (__pid_t __pid, size_t __cpusetsize,
         const cpu_set_t *__cpuset) noexcept (true);


extern int sched_getaffinity (__pid_t __pid, size_t __cpusetsize,
         cpu_set_t *__cpuset) noexcept (true);


}
# 23 "/usr/include/pthread.h" 2 3 4





# 1 "/usr/include/x86_64-linux-gnu/bits/wordsize.h" 1 3 4
# 29 "/usr/include/pthread.h" 2 3 4
# 37 "/usr/include/pthread.h" 3 4
enum
{
  PTHREAD_CREATE_JOINABLE,

  PTHREAD_CREATE_DETACHED

};



enum
{
  PTHREAD_MUTEX_TIMED_NP,
  PTHREAD_MUTEX_RECURSIVE_NP,
  PTHREAD_MUTEX_ERRORCHECK_NP,
  PTHREAD_MUTEX_ADAPTIVE_NP

  ,
  PTHREAD_MUTEX_NORMAL = PTHREAD_MUTEX_TIMED_NP,
  PTHREAD_MUTEX_RECURSIVE = PTHREAD_MUTEX_RECURSIVE_NP,
  PTHREAD_MUTEX_ERRORCHECK = PTHREAD_MUTEX_ERRORCHECK_NP,
  PTHREAD_MUTEX_DEFAULT = PTHREAD_MUTEX_NORMAL



  , PTHREAD_MUTEX_FAST_NP = PTHREAD_MUTEX_TIMED_NP

};




enum
{
  PTHREAD_MUTEX_STALLED,
  PTHREAD_MUTEX_STALLED_NP = PTHREAD_MUTEX_STALLED,
  PTHREAD_MUTEX_ROBUST,
  PTHREAD_MUTEX_ROBUST_NP = PTHREAD_MUTEX_ROBUST
};





enum
{
  PTHREAD_PRIO_NONE,
  PTHREAD_PRIO_INHERIT,
  PTHREAD_PRIO_PROTECT
};
# 104 "/usr/include/pthread.h" 3 4
enum
{
  PTHREAD_RWLOCK_PREFER_READER_NP,
  PTHREAD_RWLOCK_PREFER_WRITER_NP,
  PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP,
  PTHREAD_RWLOCK_DEFAULT_NP = PTHREAD_RWLOCK_PREFER_READER_NP
};
# 124 "/usr/include/pthread.h" 3 4
enum
{
  PTHREAD_INHERIT_SCHED,

  PTHREAD_EXPLICIT_SCHED

};



enum
{
  PTHREAD_SCOPE_SYSTEM,

  PTHREAD_SCOPE_PROCESS

};



enum
{
  PTHREAD_PROCESS_PRIVATE,

  PTHREAD_PROCESS_SHARED

};
# 159 "/usr/include/pthread.h" 3 4
struct _pthread_cleanup_buffer
{
  void (*__routine) (void *);
  void *__arg;
  int __canceltype;
  struct _pthread_cleanup_buffer *__prev;
};


enum
{
  PTHREAD_CANCEL_ENABLE,

  PTHREAD_CANCEL_DISABLE

};
enum
{
  PTHREAD_CANCEL_DEFERRED,

  PTHREAD_CANCEL_ASYNCHRONOUS

};
# 197 "/usr/include/pthread.h" 3 4
extern "C" {




extern int pthread_create (pthread_t *__restrict __newthread,
      const pthread_attr_t *__restrict __attr,
      void *(*__start_routine) (void *),
      void *__restrict __arg) noexcept (true) __attribute__ ((__nonnull__ (1, 3)));





extern void pthread_exit (void *__retval) __attribute__ ((__noreturn__));







extern int pthread_join (pthread_t __th, void **__thread_return);




extern int pthread_tryjoin_np (pthread_t __th, void **__thread_return) noexcept (true);
# 233 "/usr/include/pthread.h" 3 4
extern int pthread_timedjoin_np (pthread_t __th, void **__thread_return,
     const struct timespec *__abstime);
# 243 "/usr/include/pthread.h" 3 4
extern int pthread_clockjoin_np (pthread_t __th, void **__thread_return,
                                 clockid_t __clockid,
     const struct timespec *__abstime);
# 269 "/usr/include/pthread.h" 3 4
extern int pthread_detach (pthread_t __th) noexcept (true);



extern pthread_t pthread_self (void) noexcept (true) __attribute__ ((__const__));


extern int pthread_equal (pthread_t __thread1, pthread_t __thread2)
  noexcept (true) __attribute__ ((__const__));







extern int pthread_attr_init (pthread_attr_t *__attr) noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_attr_destroy (pthread_attr_t *__attr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_attr_getdetachstate (const pthread_attr_t *__attr,
     int *__detachstate)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_attr_setdetachstate (pthread_attr_t *__attr,
     int __detachstate)
     noexcept (true) __attribute__ ((__nonnull__ (1)));



extern int pthread_attr_getguardsize (const pthread_attr_t *__attr,
          size_t *__guardsize)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_attr_setguardsize (pthread_attr_t *__attr,
          size_t __guardsize)
     noexcept (true) __attribute__ ((__nonnull__ (1)));



extern int pthread_attr_getschedparam (const pthread_attr_t *__restrict __attr,
           struct sched_param *__restrict __param)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_attr_setschedparam (pthread_attr_t *__restrict __attr,
           const struct sched_param *__restrict
           __param) noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_attr_getschedpolicy (const pthread_attr_t *__restrict
     __attr, int *__restrict __policy)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_attr_setschedpolicy (pthread_attr_t *__attr, int __policy)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_attr_getinheritsched (const pthread_attr_t *__restrict
      __attr, int *__restrict __inherit)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_attr_setinheritsched (pthread_attr_t *__attr,
      int __inherit)
     noexcept (true) __attribute__ ((__nonnull__ (1)));



extern int pthread_attr_getscope (const pthread_attr_t *__restrict __attr,
      int *__restrict __scope)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_attr_setscope (pthread_attr_t *__attr, int __scope)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_attr_getstackaddr (const pthread_attr_t *__restrict
          __attr, void **__restrict __stackaddr)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2))) __attribute__ ((__deprecated__));





extern int pthread_attr_setstackaddr (pthread_attr_t *__attr,
          void *__stackaddr)
     noexcept (true) __attribute__ ((__nonnull__ (1))) __attribute__ ((__deprecated__));


extern int pthread_attr_getstacksize (const pthread_attr_t *__restrict
          __attr, size_t *__restrict __stacksize)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));




extern int pthread_attr_setstacksize (pthread_attr_t *__attr,
          size_t __stacksize)
     noexcept (true) __attribute__ ((__nonnull__ (1)));



extern int pthread_attr_getstack (const pthread_attr_t *__restrict __attr,
      void **__restrict __stackaddr,
      size_t *__restrict __stacksize)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2, 3)));




extern int pthread_attr_setstack (pthread_attr_t *__attr, void *__stackaddr,
      size_t __stacksize) noexcept (true) __attribute__ ((__nonnull__ (1)));





extern int pthread_attr_setaffinity_np (pthread_attr_t *__attr,
     size_t __cpusetsize,
     const cpu_set_t *__cpuset)
     noexcept (true) __attribute__ ((__nonnull__ (1, 3)));



extern int pthread_attr_getaffinity_np (const pthread_attr_t *__attr,
     size_t __cpusetsize,
     cpu_set_t *__cpuset)
     noexcept (true) __attribute__ ((__nonnull__ (1, 3)));


extern int pthread_getattr_default_np (pthread_attr_t *__attr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_attr_setsigmask_np (pthread_attr_t *__attr,
           const __sigset_t *sigmask);




extern int pthread_attr_getsigmask_np (const pthread_attr_t *__attr,
           __sigset_t *sigmask);







extern int pthread_setattr_default_np (const pthread_attr_t *__attr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));




extern int pthread_getattr_np (pthread_t __th, pthread_attr_t *__attr)
     noexcept (true) __attribute__ ((__nonnull__ (2)));







extern int pthread_setschedparam (pthread_t __target_thread, int __policy,
      const struct sched_param *__param)
     noexcept (true) __attribute__ ((__nonnull__ (3)));


extern int pthread_getschedparam (pthread_t __target_thread,
      int *__restrict __policy,
      struct sched_param *__restrict __param)
     noexcept (true) __attribute__ ((__nonnull__ (2, 3)));


extern int pthread_setschedprio (pthread_t __target_thread, int __prio)
     noexcept (true);




extern int pthread_getname_np (pthread_t __target_thread, char *__buf,
          size_t __buflen)
     noexcept (true) __attribute__ ((__nonnull__ (2)));


extern int pthread_setname_np (pthread_t __target_thread, const char *__name)
     noexcept (true) __attribute__ ((__nonnull__ (2)));





extern int pthread_getconcurrency (void) noexcept (true);


extern int pthread_setconcurrency (int __level) noexcept (true);



extern int pthread_yield (void) noexcept (true);

extern int pthread_yield (void) noexcept (true) __asm__ ("" "sched_yield")
  __attribute__ ((__deprecated__ ("pthread_yield is deprecated, use sched_yield instead")))
                                                      ;







extern int pthread_setaffinity_np (pthread_t __th, size_t __cpusetsize,
       const cpu_set_t *__cpuset)
     noexcept (true) __attribute__ ((__nonnull__ (3)));


extern int pthread_getaffinity_np (pthread_t __th, size_t __cpusetsize,
       cpu_set_t *__cpuset)
     noexcept (true) __attribute__ ((__nonnull__ (3)));
# 509 "/usr/include/pthread.h" 3 4
extern int pthread_once (pthread_once_t *__once_control,
    void (*__init_routine) (void)) __attribute__ ((__nonnull__ (1, 2)));
# 521 "/usr/include/pthread.h" 3 4
extern int pthread_setcancelstate (int __state, int *__oldstate);



extern int pthread_setcanceltype (int __type, int *__oldtype);


extern int pthread_cancel (pthread_t __th);




extern void pthread_testcancel (void);




struct __cancel_jmp_buf_tag
{
  __jmp_buf __cancel_jmp_buf;
  int __mask_was_saved;
};

typedef struct
{
  struct __cancel_jmp_buf_tag __cancel_jmp_buf[1];
  void *__pad[4];
} __pthread_unwind_buf_t __attribute__ ((__aligned__));
# 557 "/usr/include/pthread.h" 3 4
struct __pthread_cleanup_frame
{
  void (*__cancel_routine) (void *);
  void *__cancel_arg;
  int __do_it;
  int __cancel_type;
};




class __pthread_cleanup_class
{
  void (*__cancel_routine) (void *);
  void *__cancel_arg;
  int __do_it;
  int __cancel_type;

 public:
  __pthread_cleanup_class (void (*__fct) (void *), void *__arg)
    : __cancel_routine (__fct), __cancel_arg (__arg), __do_it (1) { }
  ~__pthread_cleanup_class () { if (__do_it) __cancel_routine (__cancel_arg); }
  void __setdoit (int __newval) { __do_it = __newval; }
  void __defer () { pthread_setcanceltype (PTHREAD_CANCEL_DEFERRED,
        &__cancel_type); }
  void __restore () const { pthread_setcanceltype (__cancel_type, 0); }
};
# 766 "/usr/include/pthread.h" 3 4
extern int __sigsetjmp_cancel (struct __cancel_jmp_buf_tag __env[1], int __savemask) noexcept (true) __asm__ ("" "__sigsetjmp")


                     __attribute__ ((__returns_twice__));
# 781 "/usr/include/pthread.h" 3 4
extern int pthread_mutex_init (pthread_mutex_t *__mutex,
          const pthread_mutexattr_t *__mutexattr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_mutex_destroy (pthread_mutex_t *__mutex)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_mutex_trylock (pthread_mutex_t *__mutex)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_mutex_lock (pthread_mutex_t *__mutex)
     noexcept (true) __attribute__ ((__nonnull__ (1)));




extern int pthread_mutex_timedlock (pthread_mutex_t *__restrict __mutex,
        const struct timespec *__restrict
        __abstime) noexcept (true) __attribute__ ((__nonnull__ (1, 2)));
# 817 "/usr/include/pthread.h" 3 4
extern int pthread_mutex_clocklock (pthread_mutex_t *__restrict __mutex,
        clockid_t __clockid,
        const struct timespec *__restrict
        __abstime) noexcept (true) __attribute__ ((__nonnull__ (1, 3)));
# 835 "/usr/include/pthread.h" 3 4
extern int pthread_mutex_unlock (pthread_mutex_t *__mutex)
     noexcept (true) __attribute__ ((__nonnull__ (1)));



extern int pthread_mutex_getprioceiling (const pthread_mutex_t *
      __restrict __mutex,
      int *__restrict __prioceiling)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));



extern int pthread_mutex_setprioceiling (pthread_mutex_t *__restrict __mutex,
      int __prioceiling,
      int *__restrict __old_ceiling)
     noexcept (true) __attribute__ ((__nonnull__ (1, 3)));




extern int pthread_mutex_consistent (pthread_mutex_t *__mutex)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_mutex_consistent_np (pthread_mutex_t *) noexcept (true) __asm__ ("" "pthread_mutex_consistent")
                                __attribute__ ((__nonnull__ (1)))
  __attribute__ ((__deprecated__ ("pthread_mutex_consistent_np is deprecated, use pthread_mutex_consistent")))
                                                                         ;
# 874 "/usr/include/pthread.h" 3 4
extern int pthread_mutexattr_init (pthread_mutexattr_t *__attr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_mutexattr_destroy (pthread_mutexattr_t *__attr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_mutexattr_getpshared (const pthread_mutexattr_t *
      __restrict __attr,
      int *__restrict __pshared)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_mutexattr_setpshared (pthread_mutexattr_t *__attr,
      int __pshared)
     noexcept (true) __attribute__ ((__nonnull__ (1)));



extern int pthread_mutexattr_gettype (const pthread_mutexattr_t *__restrict
          __attr, int *__restrict __kind)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));




extern int pthread_mutexattr_settype (pthread_mutexattr_t *__attr, int __kind)
     noexcept (true) __attribute__ ((__nonnull__ (1)));



extern int pthread_mutexattr_getprotocol (const pthread_mutexattr_t *
       __restrict __attr,
       int *__restrict __protocol)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));



extern int pthread_mutexattr_setprotocol (pthread_mutexattr_t *__attr,
       int __protocol)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_mutexattr_getprioceiling (const pthread_mutexattr_t *
          __restrict __attr,
          int *__restrict __prioceiling)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_mutexattr_setprioceiling (pthread_mutexattr_t *__attr,
          int __prioceiling)
     noexcept (true) __attribute__ ((__nonnull__ (1)));



extern int pthread_mutexattr_getrobust (const pthread_mutexattr_t *__attr,
     int *__robustness)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_mutexattr_getrobust_np (pthread_mutexattr_t *, int *) noexcept (true) __asm__ ("" "pthread_mutexattr_getrobust")

                                   __attribute__ ((__nonnull__ (1)))
  __attribute__ ((__deprecated__ ("pthread_mutexattr_getrobust_np is deprecated, use pthread_mutexattr_getrobust")))
                                                                               ;






extern int pthread_mutexattr_setrobust (pthread_mutexattr_t *__attr,
     int __robustness)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_mutexattr_setrobust_np (pthread_mutexattr_t *, int) noexcept (true) __asm__ ("" "pthread_mutexattr_setrobust")

                                   __attribute__ ((__nonnull__ (1)))
  __attribute__ ((__deprecated__ ("pthread_mutexattr_setrobust_np is deprecated, use pthread_mutexattr_setrobust")))
                                                                               ;
# 967 "/usr/include/pthread.h" 3 4
extern int pthread_rwlock_init (pthread_rwlock_t *__restrict __rwlock,
    const pthread_rwlockattr_t *__restrict
    __attr) noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_rwlock_destroy (pthread_rwlock_t *__rwlock)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_rwlock_rdlock (pthread_rwlock_t *__rwlock)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_rwlock_tryrdlock (pthread_rwlock_t *__rwlock)
  noexcept (true) __attribute__ ((__nonnull__ (1)));




extern int pthread_rwlock_timedrdlock (pthread_rwlock_t *__restrict __rwlock,
           const struct timespec *__restrict
           __abstime) noexcept (true) __attribute__ ((__nonnull__ (1, 2)));
# 1004 "/usr/include/pthread.h" 3 4
extern int pthread_rwlock_clockrdlock (pthread_rwlock_t *__restrict __rwlock,
           clockid_t __clockid,
           const struct timespec *__restrict
           __abstime) noexcept (true) __attribute__ ((__nonnull__ (1, 3)));
# 1023 "/usr/include/pthread.h" 3 4
extern int pthread_rwlock_wrlock (pthread_rwlock_t *__rwlock)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_rwlock_trywrlock (pthread_rwlock_t *__rwlock)
     noexcept (true) __attribute__ ((__nonnull__ (1)));




extern int pthread_rwlock_timedwrlock (pthread_rwlock_t *__restrict __rwlock,
           const struct timespec *__restrict
           __abstime) noexcept (true) __attribute__ ((__nonnull__ (1, 2)));
# 1051 "/usr/include/pthread.h" 3 4
extern int pthread_rwlock_clockwrlock (pthread_rwlock_t *__restrict __rwlock,
           clockid_t __clockid,
           const struct timespec *__restrict
           __abstime) noexcept (true) __attribute__ ((__nonnull__ (1, 3)));
# 1071 "/usr/include/pthread.h" 3 4
extern int pthread_rwlock_unlock (pthread_rwlock_t *__rwlock)
     noexcept (true) __attribute__ ((__nonnull__ (1)));





extern int pthread_rwlockattr_init (pthread_rwlockattr_t *__attr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_rwlockattr_destroy (pthread_rwlockattr_t *__attr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_rwlockattr_getpshared (const pthread_rwlockattr_t *
       __restrict __attr,
       int *__restrict __pshared)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_rwlockattr_setpshared (pthread_rwlockattr_t *__attr,
       int __pshared)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_rwlockattr_getkind_np (const pthread_rwlockattr_t *
       __restrict __attr,
       int *__restrict __pref)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_rwlockattr_setkind_np (pthread_rwlockattr_t *__attr,
       int __pref) noexcept (true) __attribute__ ((__nonnull__ (1)));







extern int pthread_cond_init (pthread_cond_t *__restrict __cond,
         const pthread_condattr_t *__restrict __cond_attr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_cond_destroy (pthread_cond_t *__cond)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_cond_signal (pthread_cond_t *__cond)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_cond_broadcast (pthread_cond_t *__cond)
     noexcept (true) __attribute__ ((__nonnull__ (1)));






extern int pthread_cond_wait (pthread_cond_t *__restrict __cond,
         pthread_mutex_t *__restrict __mutex)
     __attribute__ ((__nonnull__ (1, 2)));
# 1145 "/usr/include/pthread.h" 3 4
extern int pthread_cond_timedwait (pthread_cond_t *__restrict __cond,
       pthread_mutex_t *__restrict __mutex,
       const struct timespec *__restrict __abstime)
     __attribute__ ((__nonnull__ (1, 2, 3)));
# 1171 "/usr/include/pthread.h" 3 4
extern int pthread_cond_clockwait (pthread_cond_t *__restrict __cond,
       pthread_mutex_t *__restrict __mutex,
       __clockid_t __clock_id,
       const struct timespec *__restrict __abstime)
     __attribute__ ((__nonnull__ (1, 2, 4)));
# 1194 "/usr/include/pthread.h" 3 4
extern int pthread_condattr_init (pthread_condattr_t *__attr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_condattr_destroy (pthread_condattr_t *__attr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_condattr_getpshared (const pthread_condattr_t *
     __restrict __attr,
     int *__restrict __pshared)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_condattr_setpshared (pthread_condattr_t *__attr,
     int __pshared) noexcept (true) __attribute__ ((__nonnull__ (1)));



extern int pthread_condattr_getclock (const pthread_condattr_t *
          __restrict __attr,
          __clockid_t *__restrict __clock_id)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_condattr_setclock (pthread_condattr_t *__attr,
          __clockid_t __clock_id)
     noexcept (true) __attribute__ ((__nonnull__ (1)));
# 1230 "/usr/include/pthread.h" 3 4
extern int pthread_spin_init (pthread_spinlock_t *__lock, int __pshared)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_spin_destroy (pthread_spinlock_t *__lock)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_spin_lock (pthread_spinlock_t *__lock)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_spin_trylock (pthread_spinlock_t *__lock)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_spin_unlock (pthread_spinlock_t *__lock)
     noexcept (true) __attribute__ ((__nonnull__ (1)));






extern int pthread_barrier_init (pthread_barrier_t *__restrict __barrier,
     const pthread_barrierattr_t *__restrict
     __attr, unsigned int __count)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_barrier_destroy (pthread_barrier_t *__barrier)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_barrier_wait (pthread_barrier_t *__barrier)
     noexcept (true) __attribute__ ((__nonnull__ (1)));



extern int pthread_barrierattr_init (pthread_barrierattr_t *__attr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_barrierattr_destroy (pthread_barrierattr_t *__attr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_barrierattr_getpshared (const pthread_barrierattr_t *
        __restrict __attr,
        int *__restrict __pshared)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_barrierattr_setpshared (pthread_barrierattr_t *__attr,
        int __pshared)
     noexcept (true) __attribute__ ((__nonnull__ (1)));
# 1297 "/usr/include/pthread.h" 3 4
extern int pthread_key_create (pthread_key_t *__key,
          void (*__destr_function) (void *))
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_key_delete (pthread_key_t __key) noexcept (true);


extern void *pthread_getspecific (pthread_key_t __key) noexcept (true);


extern int pthread_setspecific (pthread_key_t __key,
    const void *__pointer)
  noexcept (true) __attribute__ ((__access__ (__none__, 2)));




extern int pthread_getcpuclockid (pthread_t __thread_id,
      __clockid_t *__clock_id)
     noexcept (true) __attribute__ ((__nonnull__ (2)));
# 1332 "/usr/include/pthread.h" 3 4
extern int pthread_atfork (void (*__prepare) (void),
      void (*__parent) (void),
      void (*__child) (void)) noexcept (true);




extern __inline __attribute__ ((__gnu_inline__)) int
__attribute__ ((__leaf__)) pthread_equal (pthread_t __thread1, pthread_t __thread2) noexcept (true)
{
  return __thread1 == __thread2;
}


}
# 45 "../../bxthread.h" 2
# 1 "/usr/include/semaphore.h" 1 3 4
# 28 "/usr/include/semaphore.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/semaphore.h" 1 3 4
# 23 "/usr/include/x86_64-linux-gnu/bits/semaphore.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/wordsize.h" 1 3 4
# 24 "/usr/include/x86_64-linux-gnu/bits/semaphore.h" 2 3 4
# 35 "/usr/include/x86_64-linux-gnu/bits/semaphore.h" 3 4
typedef union
{
  char __size[32];
  long int __align;
} sem_t;
# 29 "/usr/include/semaphore.h" 2 3 4


extern "C" {



extern int sem_init (sem_t *__sem, int __pshared, unsigned int __value)
  noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int sem_destroy (sem_t *__sem) noexcept (true) __attribute__ ((__nonnull__ (1)));


extern sem_t *sem_open (const char *__name, int __oflag, ...)
  noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int sem_close (sem_t *__sem) noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int sem_unlink (const char *__name) noexcept (true) __attribute__ ((__nonnull__ (1)));





extern int sem_wait (sem_t *__sem) __attribute__ ((__nonnull__ (1)));







extern int sem_timedwait (sem_t *__restrict __sem,
     const struct timespec *__restrict __abstime)
  __attribute__ ((__nonnull__ (1, 2)));
# 81 "/usr/include/semaphore.h" 3 4
extern int sem_clockwait (sem_t *__restrict __sem,
     clockid_t clock,
     const struct timespec *__restrict __abstime)
  __attribute__ ((__nonnull__ (1, 3)));
# 100 "/usr/include/semaphore.h" 3 4
extern int sem_trywait (sem_t *__sem) noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int sem_post (sem_t *__sem) noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int sem_getvalue (sem_t *__restrict __sem, int *__restrict __sval)
  noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


}
# 46 "../../bxthread.h" 2
# 63 "../../bxthread.h"

# 63 "../../bxthread.h"
typedef struct
{



  pthread_cond_t cond;
  pthread_mutex_t lock;

} bx_thread_event_t;

typedef struct
{



  sem_t sem;

} bx_thread_sem_t;

void bx_create_event(bx_thread_event_t *thread_ev);
void bx_destroy_event(bx_thread_event_t *thread_ev);
void bx_set_event(bx_thread_event_t *thread_ev);
bool bx_wait_for_event(bx_thread_event_t *thread_ev);
bool bx_create_sem(bx_thread_sem_t *thread_sem);
void bx_destroy_sem(bx_thread_sem_t *thread_sem);
void bx_wait_sem(bx_thread_sem_t *thread_sem);
void bx_set_sem(bx_thread_sem_t *thread_sem);
# 38 "voodoorush.cc" 2

# 1 "../../bx_debug/debug.h" 1
# 25 "../../bx_debug/debug.h"
# 1 "../../config.h" 1
# 26 "../../bx_debug/debug.h" 2
# 1 "../../osdep.h" 1
# 27 "../../bx_debug/debug.h" 2

Bit32u crc32(const Bit8u *buf, int len);
# 40 "voodoorush.cc" 2
