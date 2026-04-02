#ifndef __ERVP_BLUETOOTH_H__
#define __ERVP_BLUETOOTH_H__

void bluetooth_putc(char ch);
void bluetooth_puts(char *str);
char bluetooth_getc();
void bluetooth_init();
int bluetooth_recv(char *data, int max_size);
int bluetooth_send(char *data, int size);

#endif
