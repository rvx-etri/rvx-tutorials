// See LICENSE file for licence details

#ifndef __TOUCH_H__
#define __TOUCH_H__

typedef void (*function_ptr_t) (void);

void touch_init(function_ptr_t *touch_callback_func);
void read_touch(unsigned int *dx, unsigned int *dy);

#endif

