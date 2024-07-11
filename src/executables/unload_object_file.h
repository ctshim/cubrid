/*
 * Copyright 2008 Search Solution Corporation
 * Copyright 2016 CUBRID Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

/*
 * load_object_threading.h: simplified object definitions
 */
#ifndef _LOAD_OBJECT_THREADING_H_
#define _LOAD_OBJECT_THREADING_H_

#if !defined(WINDOWS) && defined(CS_MODE)
#define SUPPORT_MULTIPLE_UNLOADDB
//#  define MULTI_PROCESSING_UNLOADDB_WITH_FORK // TODO: ctshim  
#endif

class print_output;

#define INVALID_FILE_NO  (-1)


#include <thread>
#define YIELD_THREAD()   std::this_thread::yield ()
//#define YIELD_THREAD()   usleep (10)

#define CHECK_PRINT_ERROR(print_fnc)            \
  do {                                          \
    if ((error = print_fnc) != NO_ERROR)        \
      goto exit_on_error;                       \
  } while (0)

#define CHECK_EXIT_ERROR(e)                     \
  do {                                          \
    if ((e) == NO_ERROR) {                      \
      if (((e) = er_errid()) == NO_ERROR)       \
        (e) = ER_GENERIC_ERROR;                 \
    }                                           \
  } while (0)


#if defined(WINDOWS)
#define TIMER_CLEAR(pwi)
#define TIMER_BEGIN(pwi)
#define TIMER_END(pwi)
#else
typedef struct _waiting_info
{
  struct timespec ts_beging;
  struct timespec ts_wait_sum;
  int64_t cnt;
} S_WAITING_INFO;

#define TIMER_CLEAR(pwi)   memset((pwi), 0x00, sizeof(S_WAITING_INFO))
#define TIMER_BEGIN(pwi)   if(g_time_test_records >= 0) clock_gettime (CLOCK_MONOTONIC /* CLOCK_REALTIME */, &((pwi)->ts_beging))
#define TIMER_END(pwi)   do {                                                   \
    if(g_time_test_records >= 0)  {                                             \
         timespec_accumulate (&((pwi)->ts_wait_sum), &((pwi)->ts_beging));      \
         (pwi)->cnt++;                                                          \
      }                                                                         \
  } while(0);
#define ALIGN_SPACE_FMT "    %-25s  | "
#define NANO_PREC_VAL  (1000000000L)

extern void timespec_diff (struct timespec *start, struct timespec *end, struct timespec *diff);
extern void timespec_accumulate (struct timespec *ts_sum, struct timespec *ts_start);
#endif

typedef struct text_buffer_block TEXT_BUFFER_BLK;
struct text_buffer_block
{
  char *buffer;			/* pointer to the buffer */
  char *ptr;			/* pointer to the next byte to buffer when writing */
  int iosize;			/* optimal I/O pagesize for device */
  int count;			/* number of current buffered bytes */
  TEXT_BUFFER_BLK *next;
};


typedef struct text_output
{
  int io_size;
  int *fd_ref;

  TEXT_BUFFER_BLK *head_ptr;
  TEXT_BUFFER_BLK *tail_ptr;
} TEXT_OUTPUT;

extern bool init_queue_n_list_for_object_file (int q_size, int blk_size);

extern bool flushing_write_blk_queue (int fd);

extern int desc_value_special_fprint (TEXT_OUTPUT * tout, DB_VALUE * value);
extern void desc_value_print (print_output & output_ctx, DB_VALUE * value);
extern int text_print (TEXT_OUTPUT * tout, const char *buf, int buflen, char const *fmt, ...);
extern int text_print_request_flush (TEXT_OUTPUT * tout, bool force);


extern volatile bool error_occurred;

#endif // _LOAD_OBJECT_THREADING_H_
