///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2019, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// \brief Tiny printf, sprintf and snprintf implementation, optimized for speed on
//        embedded systems with a very limited resources.
//        Use this instead of bloated standard/newlib printf.
//        These routines are thread safe and reentrant.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __ERVP_PRINTF_H__
#define __ERVP_PRINTF_H__

#include <stdarg.h>
#include <stddef.h>
#include "platform_info.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int ready_to_print;

void set_uart_index_for_printf(int uart_index);

/**
 * Output a character to a custom device like UART, used by the printf() function
 * This function is declared here only. You have to write your custom implementation somewhere
 * \param character Character to output
 */
void _putchar(char character);


/**
 * Tiny printf implementation
 * You have to implement _putchar if you use printf()
 * To avoid conflicts with the regular printf() API it is overridden by macro defines
 * and internal underscore-appended functions like printf_rvx() are used
 * \param format A string that specifies the format of the output
 * \return The number of characters that are written into the array, not counting the terminating null character
 */
#ifdef SILENT_FOR_PROFILING
static inline int printf(const char* format, ...){ return 0; };
#else
#define printf printf_rvx
#endif

#define printf_must printf_rvx

int printf_rvx(const char* format, ...);

/**
 * Tiny sprintf implementation
 * Due to security reasons (buffer overflow) YOU SHOULD CONSIDER USING (V)SNPRINTF INSTEAD!
 * \param buffer A pointer to the buffer where to store the formatted string. MUST be big enough to store the output!
 * \param format A string that specifies the format of the output
 * \return The number of characters that are WRITTEN into the buffer, not counting the terminating null character
 */
#define sprintf sprintf_rvx
int sprintf_rvx(char* buffer, const char* format, ...);


/**
 * Tiny snprintf/vsnprintf implementation
 * \param buffer A pointer to the buffer where to store the formatted string
 * \param count The maximum number of characters to store in the buffer, including a terminating null character
 * \param format A string that specifies the format of the output
 * \param va A value identifying a variable arguments list
 * \return The number of characters that COULD have been written into the buffer, not counting the terminating
 *         null character. A value equal or larger than count indicates truncation. Only when the returned value
 *         is non-negative and less than count, the string has been completely written.
 */
#define snprintf  snprintf_rvx
#define vsnprintf vsnprintf_rvx
int  snprintf_rvx(char* buffer, size_t count, const char* format, ...);
int vsnprintf_rvx(char* buffer, size_t count, const char* format, va_list va);


/**
 * Tiny vprintf implementation
 * \param format A string that specifies the format of the output
 * \param va A value identifying a variable arguments list
 * \return The number of characters that are WRITTEN into the buffer, not counting the terminating null character
 */
#define vprintf vprintf_rvx
int vprintf_rvx(const char* format, va_list va);


/**
 * printf with output function
 * You may use this as dynamic alternative to printf() with its fixed _putchar() output
 * \param out An output function which takes one character and an argument pointer
 * \param arg An argument pointer for user data passed to output function
 * \param format A string that specifies the format of the output
 * \return The number of characters that are sent to the output function, not counting the terminating null character
 */
int fctprintf(void (*out)(char character, void* arg), void* arg, const char* format, ...);

static inline const char* debug_get_file_name_only(const char* s)
{
  const char* p = s;
  const char* last = p;
  while(1)
  {
    char c = *p;
    if(c==0)
      break;
    p++;
    if(c=='/')
      last = p;
  }
  return last;
}

#define printf_function() do{ printf("\n[%s]\n", __func__); } while(0)
#define print_line() do{ printf("\n[%s @%d]", debug_get_file_name_only(__FILE__), __LINE__); } while(0)

#define debug_printx(var) do{ if(ready_to_print) printf_must("\n[%s @%d] %s 0x%08x", debug_get_file_name_only(__FILE__), __LINE__, #var, var); } while(0)
#define debug_printd(var) do{ if(ready_to_print) printf_must("\n[%s @%d] %s %d", debug_get_file_name_only(__FILE__), __LINE__, #var, var); } while(0)
#define debug_printu(var) do{ if(ready_to_print) printf_must("\n[%s @%d] %s %u", debug_get_file_name_only(__FILE__), __LINE__, #var, var); } while(0)
#define debug_printf(var) do{ if(ready_to_print) printf_must("\n[%s @%d] %s %f", debug_get_file_name_only(__FILE__), __LINE__, #var, var); } while(0)
#define debug_printc(var) do{ if(ready_to_print) printf_must("\n[%s @%d] %s %c", debug_get_file_name_only(__FILE__), __LINE__, #var, var); } while(0)
#define debug_prints(var) do{ if(ready_to_print) printf_must("\n[%s @%d] %s %s", debug_get_file_name_only(__FILE__), __LINE__, #var, var); } while(0)

#define debug_printn() do{ if(ready_to_print) printf_must("\n"); } while(0)
#define debug_print_line() do{ if(ready_to_print) printf_must("\n[%s @%d]", debug_get_file_name_only(__FILE__), __LINE__); } while(0)

#ifdef __cplusplus
}
#endif


#endif  // _PRINTF_H_
