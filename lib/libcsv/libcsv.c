/*
libcsv - parse and write csv data
Copyright (C) 2008  Robert Gamble

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <assert.h>


/*
This is a portability shim(a small piece of code acting as a translation or
compatibility layer) that ensures 'SIZE_MAX' is defined regardless of whether
the compiler is using an old C standard or a modern one.

=> __STDC_VERSION__ is a predefined macro that tells you which C standard the compiler supports.
=> 199901L means C99 (ISO/IEC 9899:1999, published in 1999).
=> If the compiler is C99 or later, include <stdint.h>, which is a standard header that defines
   SIZE_MAX (the maximum value a size_t can hold).
=> If the compiler is C89/C90 (which predates C99 and doesn't have <stdint.h>), manually define SIZE_MAX.
=> ((size_t)-1) works because size_t is an unsigned integer type. In C, assigning -1 to an unsigned type
   causes it to wrap around to its maximum possible value (all bits set to 1). This is guaranteed by the C standard.
*/

#if __STDC_VERSION__ >= 199901L
#  include <stdint.h>
#else
#  define SIZE_MAX ((size_t)-1) /* C89 doesn't have stdint.h or SIZE_MAX */
#endif


#include "csv.h"

#define VERSION "3.0.3"


/*
  Explanation of states
  ROW_NOT_BEGUN    There have not been any fields encountered for this row
  FIELD_NOT_BEGUN  There have been fields but we are currently not in one
  FIELD_BEGUN      We are in a field
  FIELD_MIGHT_HAVE_ENDED
                   We encountered a double quote inside a quoted field, the
                   field is either ended or the quote is literal
*/
#define ROW_NOT_BEGUN           0
#define FIELD_NOT_BEGUN         1
#define FIELD_BEGUN             2
#define FIELD_MIGHT_HAVE_ENDED  3


/*It's the default increment by which the parser's
internal buffer (entry_buf) grows when it runs out of space*/
#define MEM_BLK_SIZE 128


/*This is a function-like macro that finalizes and delivers a
completed CSV field to the user's callback function*/
#define SUBMIT_FIELD(p) \
  do { \
   if (!quoted) \
     entry_pos -= spaces; \
   if (p->options & CSV_APPEND_NULL) \
     ((p)->entry_buf[entry_pos]) = '\0'; \
   if (cb1 && (p->options & CSV_EMPTY_IS_NULL) && !quoted && entry_pos == 0) \
     cb1(NULL, entry_pos, data); \
   else if (cb1) \
     cb1(p->entry_buf, entry_pos, data); \
   pstate = FIELD_NOT_BEGUN; \
   entry_pos = quoted = spaces = 0; \
 } while (0)


/*This is the companion macro to SUBMIT_FIELD. It finalizes a row
(record/line) and notifies the user that a complete row has been parsed.
*/
#define SUBMIT_ROW(p, c) \
  do { \
    if (cb2) \
      cb2(c, data); \
    pstate = ROW_NOT_BEGUN; \
    entry_pos = quoted = spaces = 0; \
  } while (0)


/*
=> Writes character c into entry_buf at the current position entry_pos
=> Increments entry_pos so the next character goes to the next slot
*/
#define SUBMIT_CHAR(p, c) ((p)->entry_buf[entry_pos++] = (c))

static const char *csv_errors[] = {"success",
                                   "error parsing data while strict checking enabled",
                                   "memory exhausted while increasing buffer size",
                                   "data size too large",
                                   "invalid status code"};


int csv_error(const struct csv_parser *p)
{
  assert(p && "received null csv_parser");

  /* Return the current status of the parser */
  return p->status;
}


const char *csv_strerror(int status)
{
  /* Return a textual description of status */
  if (status >= CSV_EINVALID || status < 0)
    return csv_errors[CSV_EINVALID];
  else
    return csv_errors[status];
}

int csv_get_opts(const struct csv_parser *p)
{
  /* Return the currently set options of parser */
  if (p == NULL)
    return -1;

  return p->options;
}


int csv_set_opts(struct csv_parser *p, unsigned char options)
{
  /* Set the options */
  if (p == NULL)
    return -1;

  p->options = options;
  return 0;
}


int csv_init(struct csv_parser *p, unsigned char options)
{
  /* Initialize a csv_parser object returns 0 on success, -1 on error */
  if (p == NULL)
    return -1;

  p->entry_buf = NULL;
  p->pstate = ROW_NOT_BEGUN;
  p->quoted = 0;
  p->spaces = 0;
  p->entry_pos = 0;
  p->entry_size = 0;
  p->status = 0;
  p->options = options;
  p->quote_char = CSV_QUOTE;
  p->delim_char = CSV_COMMA;
  p->is_space = NULL;
  p->is_term = NULL;
  p->blk_size = MEM_BLK_SIZE;
  p->malloc_func = NULL;
  p->realloc_func = realloc;
  p->free_func = free;

  return 0;
}


/*
This function deallocates the internal buffer used by the parser
*/
void csv_free(struct csv_parser *p)
{
  /* Free the entry_buffer of csv_parser object */
  if (p == NULL)
    return;

  if (p->entry_buf && p->free_func)
    p->free_func(p->entry_buf);

  p->entry_buf = NULL;
  p->entry_size = 0;

  return;
}

/*
This is the finalization function — it must be called when parsing is
complete to flush any remaining data that wasn't delivered by csv_parse()
(e.g., when a file doesn't end with a newline).

Why csv_fini Exists: csv_parse() processes data in chunks. It only calls
                  SUBMIT_ROW when it sees a line terminator (\r or \n).
                  But what if the input ends without a final newline?
name,age
Alice,30
Bob,25          ← no newline here

Without csv_fini(), the Bob,25 row would never be delivered. csv_fini()
forces delivery of the last pending field and row.
*/

int csv_fini(struct csv_parser *p, void (*cb1)(void *, size_t, void *), void (*cb2)(int c, void *), void *data)
{
  if (p == NULL)
    return -1;

  /* Finalize parsing.  Needed, for example, when file does not end in a newline */
  int quoted = p->quoted;
  int pstate = p->pstate;
  size_t spaces = p->spaces;
  size_t entry_pos = p->entry_pos;

  /*
    This catches the case where the file ends inside a quoted field that never got its closing quote
    Conditions:
    => pstate == FIELD_BEGUN — we're actively inside a field
    => p->quoted — it's a quoted field
    => CSV_STRICT enabled — strict checking is on
    => CSV_STRICT_FINI enabled — strict finalization is on
    If all true, it's an error. Without CSV_STRICT_FINI, the parser would just accept
    the truncated quoted field.
  */
  if ((pstate == FIELD_BEGUN) && p->quoted && (p->options & CSV_STRICT) && (p->options & CSV_STRICT_FINI)) {
    /* Current field is quoted, no end-quote was seen, and CSV_STRICT_FINI is set */
    p->status = CSV_EPARSE;
    return -1;
  }

  /*
    The switch (pstate) — Handling Different End States

    | State                    | Meaning                                                                                  | Action                                                          |
    | ------------------------ | ---------------------------------------------------------------------------------------- | --------------------------------------------------------------- |
    | `FIELD_MIGHT_HAVE_ENDED` | We saw a quote inside a quoted field and don't know if it ended the field or was literal | **Trim** trailing spaces + the quote, then submit field and row |
    | `FIELD_NOT_BEGUN`        | We were between fields (e.g., after a comma)                                             | Submit empty field, then submit row                             |
    | `FIELD_BEGUN`            | We were inside a field                                                                   | Submit the field, then submit row                               |
    | `ROW_NOT_BEGUN`          | Row already properly ended                                                               | Nothing to do                                                   |

  */

  switch (pstate) {
    case FIELD_MIGHT_HAVE_ENDED:
      p->entry_pos -= p->spaces + 1;  /* get rid of spaces and original quote */
      entry_pos = p->entry_pos;
      /*lint -fallthrough */
    case FIELD_NOT_BEGUN:
    case FIELD_BEGUN:
      /* Unnecessary:
      quoted = p->quoted, pstate = p->pstate;
      spaces = p->spaces, entry_pos = p->entry_pos;
      */
      SUBMIT_FIELD(p);
      SUBMIT_ROW(p, -1);
      break;
    case ROW_NOT_BEGUN: /* Already ended properly */
      ;
  }

  /* Reset parser */
  p->spaces = p->quoted = p->entry_pos = p->status = 0;
  p->pstate = ROW_NOT_BEGUN;

  return 0;
}

void csv_set_delim(struct csv_parser *p, unsigned char c)
{
  /* Set the delimiter */
  if (p) p->delim_char = c;
}

void csv_set_quote(struct csv_parser *p, unsigned char c)
{
  /* Set the quote character */
  if (p) p->quote_char = c;
}

unsigned char csv_get_delim(const struct csv_parser *p)
{
  assert(p && "received null csv_parser");

  /* Get the delimiter */
  return p->delim_char;
}

unsigned char csv_get_quote(const struct csv_parser *p)
{
  assert(p && "received null csv_parser");

  /* Get the quote character */
  return p->quote_char;
}

void csv_set_space_func(struct csv_parser *p, int (*f)(unsigned char))
{
  /* Set the space function */
  if (p) p->is_space = f;
}

void csv_set_term_func(struct csv_parser *p, int (*f)(unsigned char))
{
  /* Set the term function */
  if (p) p->is_term = f;
}

void csv_set_realloc_func(struct csv_parser *p, void *(*f)(void *, size_t))
{
  /* Set the realloc function used to increase buffer size */
  if (p && f) p->realloc_func = f;
}

void csv_set_free_func(struct csv_parser *p, void (*f)(void *))
{
  /* Set the free function used to free the buffer */
  if (p && f) p->free_func = f;
}

void csv_set_blk_size(struct csv_parser *p, size_t size)
{
  /* Set the block size used to increment buffer size */
  if (p) p->blk_size = size;
}

size_t csv_get_buffer_size(const struct csv_parser *p)
{
  /* Get the size of the entry buffer */
  if (p)
    return p->entry_size;
  return 0;
}


/*
Increase the size of the entry buffer.  Attempt to increase size by
=> p->blk_size, if this is larger than SIZE_MAX try to increase current
=> buffer size to SIZE_MAX.  If allocation fails, try to allocate halve
=> the size and try again until successful or increment size is zero.
*/
static int csv_increase_buffer(struct csv_parser *p)
{
  if (p == NULL) return 0;
  if (p->realloc_func == NULL) return 0;


  size_t to_add = p->blk_size;
  void *vp;

  if ( p->entry_size >= SIZE_MAX - to_add )
    to_add = SIZE_MAX - p->entry_size;

  if (!to_add) {
    p->status = CSV_ETOOBIG;
    return -1;
  }

  while ((vp = p->realloc_func(p->entry_buf, p->entry_size + to_add)) == NULL) {
    to_add /= 2;
    if (!to_add) {
      p->status = CSV_ENOMEM;
      return -1;
    }
  }

  /* Update entry buffer pointer and entry_size if successful */
  p->entry_buf = vp;
  p->entry_size += to_add;
  return 0;
}


/*
This is the heart of the library — the main CSV parsing function.
It's a state machine that processes input character by character.


Function Signature:
| Parameter | Meaning                                                                                       |
| --------- | --------------------------------------------------------------------------------------------- |
| `p`       | Parser state object                                                                           |
| `s`       | Pointer to input data (can be binary — cast to `unsigned char`)                               |
| `len`     | Number of bytes to process                                                                    |
| `cb1`     | **Field callback**: called when a field is complete (`void *data, size_t len, void *context`) |
| `cb2`     | **Row callback**: called when a row ends (`int terminator, void *context`)                    |
| `data`    | User context pointer passed through to callbacks                                              |
Returns: Number of bytes successfully processed. If less than len, an error occurred.




*/
size_t csv_parse(struct csv_parser *p, const void *s, size_t len, void (*cb1)(void *, size_t, void *), void (*cb2)(int c, void *), void *data)
{
  assert(p && "received null csv_parser");

  if (s == NULL) return 0;

  unsigned const char *us = s;  /* Access input data as array of unsigned char */
  unsigned char c;              /* The character we are currently processing */
  size_t pos = 0;               /* The number of characters we have processed in this call */

  /* Store key fields into local variables for performance */
  unsigned char delim = p->delim_char;
  unsigned char quote = p->quote_char;
  int (*is_space)(unsigned char) = p->is_space;
  int (*is_term)(unsigned char) = p->is_term;
  int quoted = p->quoted;
  int pstate = p->pstate;
  size_t spaces = p->spaces;
  size_t entry_pos = p->entry_pos;


  if (!p->entry_buf && pos < len) {
    /* Buffer hasn't been allocated yet and len > 0 */
    if (csv_increase_buffer(p) != 0) {
      p->quoted = quoted, p->pstate = pstate, p->spaces = spaces, p->entry_pos = entry_pos;
      return pos;
    }
  }

  while (pos < len) {
    /* Check memory usage, increase buffer if necessary */
    if (entry_pos == ((p->options & CSV_APPEND_NULL) ? p->entry_size - 1 : p->entry_size) ) {
      if (csv_increase_buffer(p) != 0) {
        p->quoted = quoted, p->pstate = pstate, p->spaces = spaces, p->entry_pos = entry_pos;
        return pos;
      }
    }

    c = us[pos++];

    switch (pstate) {
      case ROW_NOT_BEGUN:
      case FIELD_NOT_BEGUN:
        if ((is_space ? is_space(c) : c == CSV_SPACE || c == CSV_TAB) && c!=delim) { /* Space or Tab */
          continue;
        } else if (is_term ? is_term(c) : c == CSV_CR || c == CSV_LF) { /* Carriage Return or Line Feed */
          if (pstate == FIELD_NOT_BEGUN) {
            SUBMIT_FIELD(p);
            SUBMIT_ROW(p, c);
          } else {  /* ROW_NOT_BEGUN */
            /* Don't submit empty rows by default */
            if (p->options & CSV_REPALL_NL) {
              SUBMIT_ROW(p, c);
            }
          }
          continue;
        } else if (c == delim) { /* Comma */
          SUBMIT_FIELD(p);
          break;
        } else if (c == quote) { /* Quote */
          pstate = FIELD_BEGUN;
          quoted = 1;
        } else {               /* Anything else */
          pstate = FIELD_BEGUN;
          quoted = 0;
          SUBMIT_CHAR(p, c);
        }
        break;
      case FIELD_BEGUN:
        if (c == quote) {         /* Quote */
          if (quoted) {
            SUBMIT_CHAR(p, c);
            pstate = FIELD_MIGHT_HAVE_ENDED;
          } else {
            /* STRICT ERROR - double quote inside non-quoted field */
            if (p->options & CSV_STRICT) {
              p->status = CSV_EPARSE;
              p->quoted = quoted, p->pstate = pstate, p->spaces = spaces, p->entry_pos = entry_pos;
              return pos-1;
            }
            SUBMIT_CHAR(p, c);
            spaces = 0;
          }
        } else if (c == delim) {  /* Comma */
          if (quoted) {
            SUBMIT_CHAR(p, c);
          } else {
            SUBMIT_FIELD(p);
          }
        } else if (is_term ? is_term(c) : c == CSV_CR || c == CSV_LF) {  /* Carriage Return or Line Feed */
          if (!quoted) {
            SUBMIT_FIELD(p);
            SUBMIT_ROW(p, c);
          } else {
            SUBMIT_CHAR(p, c);
          }
        } else if (!quoted && (is_space? is_space(c) : c == CSV_SPACE || c == CSV_TAB)) { /* Tab or space for non-quoted field */
            SUBMIT_CHAR(p, c);
            spaces++;
        } else {  /* Anything else */
          SUBMIT_CHAR(p, c);
          spaces = 0;
        }
        break;
      case FIELD_MIGHT_HAVE_ENDED:
        /* This only happens when a quote character is encountered in a quoted field */
        if (c == delim) {  /* Comma */
          entry_pos -= spaces + 1;  /* get rid of spaces and original quote */
          SUBMIT_FIELD(p);
        } else if (is_term ? is_term(c) : c == CSV_CR || c == CSV_LF) {  /* Carriage Return or Line Feed */
          entry_pos -= spaces + 1;  /* get rid of spaces and original quote */
          SUBMIT_FIELD(p);
          SUBMIT_ROW(p, c);
        } else if (is_space ? is_space(c) : c == CSV_SPACE || c == CSV_TAB) {  /* Space or Tab */
          SUBMIT_CHAR(p, c);
          spaces++;
        } else if (c == quote) {  /* Quote */
          if (spaces) {
            /* STRICT ERROR - unescaped double quote */
            if (p->options & CSV_STRICT) {
              p->status = CSV_EPARSE;
              p->quoted = quoted, p->pstate = pstate, p->spaces = spaces, p->entry_pos = entry_pos;
              return pos-1;
            }
            spaces = 0;
            SUBMIT_CHAR(p, c);
          } else {
            /* Two quotes in a row */
            pstate = FIELD_BEGUN;
          }
        } else {  /* Anything else */
          /* STRICT ERROR - unescaped double quote */
          if (p->options & CSV_STRICT) {
            p->status = CSV_EPARSE;
            p->quoted = quoted, p->pstate = pstate, p->spaces = spaces, p->entry_pos = entry_pos;
            return pos-1;
          }
          pstate = FIELD_BEGUN;
          spaces = 0;
          SUBMIT_CHAR(p, c);
        }
        break;
     default:
       break;
    }
  }
  p->quoted = quoted, p->pstate = pstate, p->spaces = spaces, p->entry_pos = entry_pos;
  return pos;
}


/*
This is a convenience wrapper around csv_write2 that uses the default double-quote character (").

It simply forwards all arguments to csv_write2, hardcoding the quote character as CSV_QUOTE
(which is 0x22 = " from the header).

Why It Exists: Ease of use. Most CSV files use standard double quotes, so this saves you from
typing the quote parameter every time:
*/
size_t csv_write(void *dest, size_t dest_size, const void *src, size_t src_size)
{
  return csv_write2(dest, dest_size, src, src_size, CSV_QUOTE);
}

/*
It forwards all arguments to csv_fwrite2, hardcoding the quote character
as the standard double-quote (CSV_QUOTE = 0x22 = ").

| Parameter   | Passed Through? | Meaning                  |
| ----------- | --------------- | ------------------------ |
| `fp`        | 1               | File pointer to write to |
| `src`       | 1               | Input data to wrap       |
| `src_size`  | 1               | Length of input data     |
| `CSV_QUOTE` | 0 (hardcoded)   | Quote character = `"`    |

Why It Exists: Same rationale as csv_write — most CSV files use standard
               double quotes, so this saves typing

*/
int csv_fwrite(FILE *fp, const void *src, size_t src_size)
{
  return csv_fwrite2(fp, src, src_size, CSV_QUOTE);
}

/*
This is the core CSV serialization function — it takes raw data and wraps it in CSV quotes,
escaping any quote characters inside by doubling them. It can either write to a buffer or
just count how many bytes would be needed.

| Parameter   | Purpose                                     |
| ----------- | ------------------------------------------- |
| `dest`      | Output buffer (can be `NULL` to count only) |
| `dest_size` | Size of output buffer                       |
| `src`       | Input data to quote-wrap                    |
| `src_size`  | Length of input data                        |
| `quote`     | Quote character to use (e.g., `"` or `'` )  |
Returns: Total number of characters needed for the fully quoted output.

*/
size_t csv_write2(void *dest, size_t dest_size, const void *src, size_t src_size, unsigned char quote)
{
  unsigned char *cdest = dest;
  const unsigned char *csrc = src;
  size_t chars = 0;

  if (src == NULL)
    return 0;

  if (dest == NULL)
    dest_size = 0;

  if (dest_size > 0)
    *cdest++ = quote;
  chars++;

  while (src_size) {
    if (*csrc == quote) {
      if (dest_size > chars)
        *cdest++ = quote;
      if (chars < SIZE_MAX) chars++;
    }
    if (dest_size > chars)
      *cdest++ = *csrc;
    if (chars < SIZE_MAX) chars++;
    src_size--;
    csrc++;
  }

  if (dest_size > chars)
    *cdest = quote;
  if (chars < SIZE_MAX) chars++;

  return chars;
}


/*
This is the file-stream version of CSV serialization — it writes quoted data directly to a
FILE * instead of a memory buffer. It follows the same escaping rules as csv_write2 but with
a simpler API since file streams handle their own memory.

| Parameter  | Purpose                                   |
| ---------- | ----------------------------------------- |
| `fp`       | File pointer to write to (`fopen` result) |
| `src`      | Input data to quote-wrap                  |
| `src_size` | Length of input data                      |
| `quote`    | Quote character to use                    |

Returns: 0 on success, EOF (typically -1) on any write error.

Example walkthrough:
Input: say "hi" (8 bytes)
Quote: "
File: already open

| Step          | `fputc` writes | Stream contents |
| ------------- | -------------- | --------------- |
| Opening quote | `"`            | `"`             |
| `s`           | `s`            | `"s`            |
| `a`           | `a`            | `"sa`           |
| `y`           | `y`            | `"say`          |
| ` ` (space)   | ` `            | `"say `         |
| `"` (escape)  | `"`            | `"say "`        |
| `"` (actual)  | `"`            | `"say """`      |
| `h`           | `h`            | `"say ""h`      |
| `i`           | `i`            | `"say ""hi`     |
| Closing quote | `"`            | `"say ""hi"`    |

*/
int csv_fwrite2 (FILE *fp, const void *src, size_t src_size, unsigned char quote)
{
  const unsigned char *csrc = src;

  if (fp == NULL || src == NULL)
    return 0;

  if (fputc(quote, fp) == EOF)
    return EOF;

  while (src_size) {
    if (*csrc == quote) {
      if (fputc(quote, fp) == EOF)
        return EOF;
    }
    if (fputc(*csrc, fp) == EOF)
      return EOF;
    src_size--;
    csrc++;
  }

  if (fputc(quote, fp) == EOF) {
    return EOF;
  }

  return 0;
}


/*

┌─────────────────────────────────────────────────────────────────────────────┐
│                           LIBCSV ARCHITECTURE                               │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                     USER APPLICATION                                │    │
│  │                                                                     │    │
│  │   ┌─────────────┐    ┌─────────────┐    ┌─────────────────────┐     │    │
│  │   │  CSV FILE   │    │  CSV DATA   │    │   OUTPUT (write)    │     │    │
│  │   │   (read)    │    │  (in memory)│    │   FILE / BUFFER     │     │    │
│  │   └──────┬──────┘    └──────┬──────┘    └─────────────────────┘     │    │
│  │          │                  │                                       │    │
│  │          │  csv_parse()   │                                         │    │
│  │          │  (chunked)     │                                         │    │
│  │          ▼                  ▼                                       │    │
│  │   ┌──────────────────────────────────────────────────────────┐      │    │
│  │   │              struct csv_parser (STATE)                   │      │    │
│  │   │  ┌────────────────────────────────────────────────────┐  │      │    │
│  │   │  │  entry_buf ──► [growing buffer for current         │  │      │    │
│  │   │  │               │  field data]                       │  │      │    │
│  │   │  │  entry_pos   │  current write position             │  │      │    │
│  │   │  │  entry_size  │  allocated capacity                 │  │      │    │
│  │   │  │  pstate      │  ROW_NOT_BEGUN / FIELD_NOT_BEGUN    │  │      │    │
│  │   │  │              │  FIELD_BEGUN / FIELD_MIGHT_HAVE_    │  │      │    │
│  │   │  │              │  ENDED                              │  │      │    │
│  │   │  │  quoted      │  0=non-quoted, 1=quoted field       │  │      │    │
│  │   │  │  spaces      │  trailing space counter             │  │      │    │
│  │   │  │  status      │  error code (CSV_EPARSE, etc.)      │  │      │    │
│  │   │  │  options     │  CSV_STRICT | CSV_APPEND_NULL...    │  │      │    │
│  │   │  │  quote_char  │  '"' (customizable)                 │  │      │    │
│  │   │  │  delim_char  │  ',' (customizable)                 │  │      │    │
│  │   │  │  blk_size    │  128 bytes (allocation chunk)       │  │      │    │
│  │   │  │  realloc_func│  realloc() (customizable)           │  │      │    │
│  │   │  │  free_func   │  free() (customizable)              │  │      │    │
│  │   │  └────────────────────────────────────────────────────┘  │      │    │
│  │   └──────────────────────────────────────────────────────────┘      │    │
│  │          │                  │                                       │    │
│  │          │  cb1 callback    │  cb2 callback                         │    │
│  │          ▼                  ▼                                       │    │
│  │   ┌─────────────┐    ┌─────────────┐                                │    │
│  │   │  FIELD DATA │    │  ROW END    │                                │    │
│  │   │  (void*,    │    │  (int term, │                                │    │
│  │   │   size_t)   │    │   void*)    │                                │    │
│  │   └─────────────┘    └─────────────┘                                │    │
│  │                                                                     │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
│                                                                             │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│   PARSER LIFECYCLE (Initialization → Parsing → Finalization → Cleanup)      │
│                                                                             │
│        ┌──────────┐                                                         │
│        │  START   │                                                         │
│        └────┬─────┘                                                         │
│             │                                                               │
│             ▼                                                               │
│   ┌─────────────────┐                                                       │
│   │  csv_init(&p,   │   ◄─── Allocate struct csv_parser on stack/heap       │
│   │  CSV_STRICT |   │        Set defaults: quote='"', delim=',',            │
│   │  CSV_APPEND_NULL│        blk_size=128, realloc_func=realloc             │
│   └────────┬────────┘                                                       │
│            │                                                                │
│            ▼                                                                │
│   ┌─────────────────┐     ┌────────────────────────┐                        │
│   │ csv_set_delim() │     │ csv_set_quote()        │  ◄── Optional config   │
│   │ csv_set_blk_size│     │ csv_set_space_func()   │                        │
│   │ csv_set_opts()  │     │ csv_set_realloc_func() │                        │
│   └─────────────────┘     └────────────────────────┘                        │
│            │                                                                │
│            ▼                                                                │
│   ┌─────────────────────────────────────────────────────────────────┐       │
│   │                         PARSING LOOP                            │       │
│   │                                                                 │       │
│   │   ┌─────────────┐      ┌─────────────┐      ┌─────────────┐     │       │
│   │   │ Read chunk  │─────►│csv_parse()  │─────►│ More data?  │───┐ │       │
│   │   │ from file   │      │             │      │             │   │ │       │
│   │   └─────────────┘      └──────┬──────┘      └─────────────┘   │ │       │
│   │                               │                               │ │       │
│   │                    ┌──────────┴──────────┐                    │ │       │
│   │                    │                     │                    │ │       │
│   │                    ▼                     ▼                    │ │       │
│   │           ┌─────────────┐      ┌─────────────┐                │ │       │
│   │           │   SUCCESS   │      │    ERROR    │                │ │       │
│   │           │ (returned   │      │ (returned   │                │ │       │
│   │           │  len bytes) │      │  < len)     │                │ │       │
│   │           └─────────────┘      └──────┬──────┘                │ │       │
│   │                                         │                     │ │       │
│   │                              ┌──────────┘                     │ │       │
│   │                              ▼                                │ │       │
│   │                     ┌─────────────┐                           │ │       │
│   │                     │csv_error()  │  ◄── Get error code       │ │       │
│   │                     │csv_strerror │  ◄── Get error message    │ │       │
│   │                     └─────────────┘                           │ │       │
│   │                                                                 │       │
│   └─────────────────────────────────────────────────────────────────┘       │
│            │                                                                │
│            ▼                                                                │
│   ┌─────────────────┐  ◄── Flush any remaining field/row (no newline at EOF)│
│   │  csv_fini(&p,   │      Also checks CSV_STRICT_FINI for unclosed quotes  │
│   │  cb1, cb2, data)│                                                       │
│   └────────┬────────┘                                                       │
│            │                                                                │
│            ▼                                                                │
│   ┌─────────────────┐  ◄── Free entry_buf memory                            │
│   │  csv_free(&p)   │      (uses custom free_func if set)                   │
│   └─────────────────┘                                                       │
│            │                                                                │
│            ▼                                                                │
│   ┌─────────────────┐  ◄── Can call csv_init() again to reuse the struct    │
│   │  REUSE or END   │                                                       │
│   └─────────────────┘                                                       │
│                                                                             │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│   STATE MACHINE (Inside csv_parse)                                          │
│                                                                             │
│                                                                             │
│                    ┌─────────────────┐                                      │
│                    │  ROW_NOT_BEGUN  │◄──────────────────────────┐          │
│                    │  (start of row) │                           │          │
│                    └────────┬────────┘                           │          │
│                             │                                    │          │
│           ┌─────────────────┼─────────────────┐                  │          │
│           │                 │                 │                  │          │
│        newline          comma/EOF          non-space             │          │
│           │                 │                 │                  │          │
│           │                 ▼                 ▼                  │          │
│           │        ┌─────────────┐    ┌─────────────┐            │          │
│           │        │FIELD_NOT_   │    │  FIELD_BEGUN│            │          │
│           │        │  BEGUN      │    │  quoted=0/1 │            │          │
│           │        │(after comma)│    │             │            │          │
│           │        └──────┬──────┘    └──────┬──────┘            │          │
│           │               │                  │                   │          │
│           │         ┌─────┴─────┐            │                   │          │
│           │         │           │            │                   │          │
│           │      newline     comma        quote                  │          │
│           │         │         │            │                     │          │
│           │         ▼         ▼            ▼                     │          │
│           │    ┌────────┐ ┌────────┐  ┌─────────────────┐        │          │
│           │    │SUBMIT_ │ │SUBMIT_ │  │ SUBMIT_CHAR(q)  │        │          │
│           │    │ FIELD  │ │ FIELD  │  │ pstate=FIELD_   │        │          │
│           │    │SUBMIT_ │ │ (empty)│  │ MIGHT_HAVE_ENDED│        │          │
│           │    │ ROW    │ └────────┘  └────────┬────────┘        │          │
│           │    └───┬────┘                      │                 │          │
│           │        │                            │                │          │
│           │        └────────────────────────────┘                │          │
│           │                                     │                │          │
│           │                              ┌──────┴──────┐         │          │
│           │                              │             │         │          │
│           │                           comma/newline  quote       │          │
│           │                              │             │         │          │
│           │                              ▼             ▼         │          │
│           │                         ┌─────────┐  ┌─────────┐     │          │
│           │                         │SUBMIT_  │  │SUBMIT_  │     │          │
│           │                         │ FIELD   │  │ CHAR(q) │     │          │
│           │                         │SUBMIT_  │  │pstate=  │     │          │
│           │                         │ ROW     │  │FIELD_   │     │          │
│           │                         └────┬────┘  │BEGUN    │     │          │
│           │                              │       └────┬────┘     │          │
│           │                              │            │          │          │
│           └──────────────────────────────┴────────────┘          │          │
│                                                      │           │          │
│                                                      └───────────|          │
│                                                                             │
│   Legend:                                                                   │
│   ───────►  Normal transition                                               │
│   ─ ─ ─ ►  Return/loop back                                                 │
│   SUBMIT_FIELD  → calls cb1 with field data                                 │
│   SUBMIT_ROW    → calls cb2 with terminator char                            │
│                                                                             │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│   WRITE FUNCTIONS (Serialization - Opposite of Parsing)                     │
│                                                                             │
│                                                                             │
│   ┌─────────────────────────────────────────────────────────────────────┐   │
│   │                        USER DATA                                    │   │
│   │                    (raw, unquoted)                                  │   │
│   └───────────────────────────┬─────────────────────────────────────────┘   │
│                               │                                             │
│              ┌────────────────┼────────────────┐                            │
│              │                │                │                            │
│              ▼                ▼                ▼                            │
│   ┌─────────────────┐ ┌─────────────┐ ┌─────────────────┐                   │
│   │   csv_write()   │ │csv_write2() │ │  csv_fwrite()   │                   │
│   │  (quote=")      │ │(custom quote│ │  (quote=")      │                   │
│   │                 │ │             │ │                 │                   │
│   │  ──► csv_write2 │ │  Count mode │ │  ──► csv_fwrite2│                   │
│   │     (dest,      │ │  (dest=NULL)│ │     (fp,        │                   │
│   │      dest_size, │ │  or Write   │ │      src,       │                   │
│   │      src,       │ │  mode       │ │      src_size)  │                   │
│   │      src_size,  │ │             │ │                 │                   │
│   │      '"')       │ │  Returns:   │ │  Returns:       │                   │
│   │                 │ │ bytes needed│ │  0 or EOF       │                   │
│   │  Returns:       │ │             │ │                 │                   │
│   │  bytes needed   │ │  Escapes:   │ │  Escapes:       │                   │
│   │                 │ │  "" → """"  │ │  "" → """"      │                   │
│   │                 │ │             │ │                 │                   │
│   └─────────────────┘ └─────────────┘ └─────────────────┘                   │
│                              │                                              │
│                              ▼                                              │
│   ┌─────────────────────────────────────────────────────────────────────┐   │
│   │                    OUTPUT: Quoted CSV Field                         │   │
│   │                                                                     │   │
│   │   Input:  hello "world"    ──►    Output: "hello ""world"""         │   │
│   │   Input:  a,b,c            ──►    Output: "a,b,c"                   │   │
│   │   Input:  line1\nline2     ──►    Output: "line1                    │   │
│   │                                          line2"                     │   │
│   └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│   MEMORY MANAGEMENT (Buffer Growth)                                         │
│                                                                             │
│                                                                             │
│   csv_parse() ──► entry_buf full? ──Yes──► csv_increase_buffer()            │
│        │                                      │                             │
│        │                                      ▼                             │
│        │                              ┌─────────────┐                       │
│        │                              │  Try to add │                       │
│        │                              │  blk_size   │                       │
│        │                              │  (default   │                       │
│        │                              │   128 bytes)│                       │
│        │                              └──────┬──────┘                       │
│        │                                     │                              │
│        │                              ┌─────┴─────┐                         │
│        │                              │           │                         │
│        │                         Success      Failure                       │
│        │                              │           │                         │
│        │                              ▼           ▼                         │
│        │                         ┌────────┐  ┌────────┐                     │
│        │                         │Continue│  │Try half│                     │
│        │                         │parsing │  │size    │                     │
│        │                         │        │  │Repeat  │                     │
│        │                         └────────┘  │until 0 │                     │
│        │                                    └────┬───┘                      │
│        │                                         │                          │
│        │                                    ┌────┴────┐                     │
│        │                                    │         │                     │
│        │                               Success    Still fails               │
│        │                                    │         │                     │
│        │                                    ▼         ▼                     │
│        │                               ┌────────┐ ┌───────────┐             │
│        │                               │Continue│ │CSV_ETOOBIG│             │
│        │                               │parsing │ │or CSV_    │             │
│        │                               │        │ │ ENOMEM    │             │
│        │                               └────────┘ └───────────┘             │
│        │                                                                    │
│        └────────────────────────────────────────────────────────────────────┘
│                                                                             │
│   csv_free() ──► calls free_func(entry_buf) ──► entry_buf = NULL            │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘

*/
