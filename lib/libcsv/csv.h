/*
libcsv - parse and write csv data
Copyright (C) 2008-2021  Robert Gamble

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



#ifndef LIBCSV_H__ /* can also write LIBCSV_H, LIBCSV_H_ */
#define LIBCSV_H__ /* can also write LIBCSV_H, LIBCSV_H_ */


/* Including Necessary Header Files*/
#include <stdlib.h>
#include <stdio.h>


/* The following code allows C++ program to
include and use a C header file without causing
linker errors.

=> __cpluplus is a special macro that is automatically
defined only by C++ compilers. If you are compiling with
a C compiler then this macro does not exist.

=> extern "C" is a C++ specific keyword. It tell the
C++ compiler to not apply C++ name mangling to the
function declared inside these braces; treat them as
C functions.

=> #endif closes the preprocessor conditional.
*/
#ifdef __cplusplus
extern "C" {
#endif


/* The followin three lines define the version
number of the libcsv library.

This follows the common MAJOR.MINOR.RELEASE versioning convention:
=> MAJOR: incompatible API changes
=> MINOR: added functionality (backward compatible)
=> RELEASE/PATCH: bug fixes (backward compatible)

*/
#define CSV_MAJOR 3
#define CSV_MINOR 0
#define CSV_RELEASE 3


/* Error Codes */
#define CSV_SUCCESS 0  /* Operation completed successfully (no error) */
#define CSV_EPARSE 1   /* Parse error — occurs in strict mode when
						  the CSV data doesn't conform
						  to expected formatting rules */
#define CSV_ENOMEM 2   /* No memory — a memory allocation failed while
						  trying to grow the internal buffer */
#define CSV_ETOOBIG 3  /* Too big — the buffer needed would
						  exceed SIZE_MAX (the maximum representable
						  size_t value on the system) */
#define CSV_EINVALID 4 /* Invalid — an internal error that should never
						  happen; indicates a bug if returned by csv_error() */




/* parser options */
#define CSV_STRICT 1    	     /* Enable strict mode — the parser enforces stricter
																	CSV formatting rules. If violations are found, it
																	returns CSV_EPARSE. */
#define CSV_REPALL_NL 2 	     /* Report all newlines — normally, newlines inside
																	quoted fields are silently accepted. With this flag,
																	unquoted carriage returns (\r) and line feeds (\n)
																	are reported to the callback function cb2. */
#define CSV_STRICT_FINI 4   	 /* Strict finalization — when csv_fini() is called to
																	finish parsing, if the last field is quoted but missing
																	its closing quote, return CSV_EPARSE instead of silently
																	accepting it. */
#define CSV_APPEND_NULL 8   	 /* Null-terminate fields — ensures every field buffer passed
																	to the callback (cb1) ends with a \0 byte, making it a
																	valid C string. */
#define CSV_EMPTY_IS_NULL 16 	/* Empty fields become NULL — when an empty, unquoted field
																	is encountered (e.g., ,, or , ,), pass a NULL pointer
																	(instead of an empty string "") to the cb1 callback. */


/* Character values */
#define CSV_TAB    0x09 /* 0x09 is the hexadecimal representation of the horizontal tab */
#define CSV_SPACE  0x20 /* 0x20 is the hexadecimal representation of the standard space */
#define CSV_CR     0x0d /* 0x0d is the hexadecimal representation of the Carriage Return (/r) */
#define CSV_LF     0x0a /* 0x0a is the hexadecimal representation of the Line Feed(LF) */
#define CSV_COMMA  0x2c /* 0x2c is the hexadecimal representation of the comma character */
#define CSV_QUOTE  0x22 /* 0x22 is the hexadecimal representation of the quotation mark (") */



struct csv_parser {
  /*========================================================*/
  /*			State Tracking (Parsing Progress)           */
  /*========================================================*/
  int pstate;         /*Parser state — tracks where the parser is
											in the state machine (e.g., start of field,
											inside quoted field, after quote, end of row, etc.).
											This is the heart of the parsing logic. */
  int quoted;         /*Quoted field flag — true (non-zero) if the
											current field began with a quote character,
											false (0) otherwise. Helps distinguish "hello"
											from hello. */
  size_t spaces;      /*Space counter — counts consecutive spaces after
											a closing quote or within a non-quoted field.
											Used to handle spaces before delimiters properly
											(e.g., "field" ,next). */


  /*========================================================*/
  /*		  Buffer Management (Building Field Values)     */
  /*========================================================*/
  unsigned char *entry_buf;   /*Entry buffer — dynamically allocated memory
								that holds the characters of the current field
								being parsed. As characters are read, they're
								appended here. */
  size_t entry_pos;   		/*Current position in buffer — also represents
								the current length of the field data. Points
								to where the next character will be written. */
  size_t entry_size;  		/*Total buffer size — the allocated capacity of
								entry_buf. If entry_pos reaches this, the
								buffer needs to grow via realloc_func. */


  /*========================================================*/
  /*		  	Status & Configuration     			   */
  /*========================================================*/
  int status;         	  	/*Operation status — stores error codes
								  (like CSV_EPARSE, CSV_ENOMEM) so csv_error()
								  can retrieve them later.*/
  unsigned char options;		/*Parser options — bitmask of flags (CSV_STRICT,
								  CSV_APPEND_NULL, etc.) set during csv_init().*/
  unsigned char quote_char; 	/*Quote character — usually " (0x22), but customizable
								  via csv_set_quote().*/
  unsigned char delim_char; 	/*Delimiter character — usually , (0x2c), but customizable
								  via csv_set_delim().*/


  /*========================================================*/
  /*		 Customizable Behavior (Function Pointers)      */
  /*========================================================*/
  int (*is_space)(unsigned char); /*Space checker — function that determines
									if a character counts as "space" (default: tab and space).
									Used to skip whitespace. Customizable via
									csv_set_space_func().*/
  int (*is_term)(unsigned char);  /*Terminator checker — function that determines if
									a character is a line terminator (default: \r and \n)
									Customizable via csv_set_term_func().*/


  /*========================================================*/
  /*		 			Block Size     					*/
  /*========================================================*/
  size_t blk_size;	/*Block size — the increment by which entry_buf
						grows when it needs more space. Instead of
						reallocating byte-by-byte (expensive), it grows
						in chunks. Customizable via csv_set_blk_size().*/


  /*========================================================*/
  /*		 Memory Management (Function Pointers)          */
  /*========================================================*/
  void *(*malloc_func)(size_t);           /*not used*/
  void *(*realloc_func)(void *, size_t);  /*Reallocator — function used to grow
											entry_buf when it fills up. Default is realloc().
											Customizable for specialized memory management
											(e.g., custom allocators, memory pools). */
  void (*free_func)(void *);              /*Deallocator — function to free entry_buf when
											csv_free() is called. Default is free(). */
};

/*
How It All Works Together: When you call csv_init(), this struct is zeroed/initialized.
Then csv_parse() processes input character by character:
=> State machine (pstate) decides what each character means in context
=> Characters are appended to entry_buf (growing via realloc_func if needed)
=> When a field ends (delimiter or newline found), entry_buf (length entry_pos) is passed to your cb1 callback
=> When a row ends (newline), your cb2 callback is invoked
=> Any errors are stored in status

When done, csv_fini() finalizes any remaining data, and csv_free() releases entry_buf using free_func.
*/


/* Function Prototypes */
int csv_init(struct csv_parser *p, unsigned char options);
int csv_fini(struct csv_parser *p, void (*cb1)(void *, size_t, void *), void (*cb2)(int, void *), void *data);
void csv_free(struct csv_parser *p);
int csv_error(const struct csv_parser *p);
const char * csv_strerror(int error);
size_t csv_parse(struct csv_parser *p, const void *s, size_t len, void (*cb1)(void *, size_t, void *), void (*cb2)(int, void *), void *data);
size_t csv_write(void *dest, size_t dest_size, const void *src, size_t src_size);
int csv_fwrite(FILE *fp, const void *src, size_t src_size);
size_t csv_write2(void *dest, size_t dest_size, const void *src, size_t src_size, unsigned char quote);
int csv_fwrite2(FILE *fp, const void *src, size_t src_size, unsigned char quote);
int csv_get_opts(const struct csv_parser *p);
int csv_set_opts(struct csv_parser *p, unsigned char options);
void csv_set_delim(struct csv_parser *p, unsigned char c);
void csv_set_quote(struct csv_parser *p, unsigned char c);
unsigned char csv_get_delim(const struct csv_parser *p);
unsigned char csv_get_quote(const struct csv_parser *p);
void csv_set_space_func(struct csv_parser *p, int (*f)(unsigned char));
void csv_set_term_func(struct csv_parser *p, int (*f)(unsigned char));
void csv_set_realloc_func(struct csv_parser *p, void *(*)(void *, size_t));
void csv_set_free_func(struct csv_parser *p, void (*)(void *));
void csv_set_blk_size(struct csv_parser *p, size_t);
size_t csv_get_buffer_size(const struct csv_parser *p);


#ifdef __cplusplus
}
#endif

#endif
