#include "platform_info.h"
#include "ervp_uart.h"
#include "ervp_printf.h"
#include "ervp_bluetooth.h"

#define UART_ID_OF_BLUETOOTH		(UART_INDEX_FOR_BLUETOOTH)
#define UART_RATE_BLUETOOTH			(115200)
#define BUFFER_SIZE_OF_BLUETOOTH 	(1024*2)

void _bluetooth_send(char *ch, int size);
void _bluetooth_recv(char *ch, int size);
void _bluetooth_wait_ack();
void _bluetooth_send_ack();

void bluetooth_putc(char ch)
{
	uart_putc(UART_ID_OF_BLUETOOTH, ch);
}

void bluetooth_puts(char *str)
{
	int n = 0;
	while(str[n] != '\0') // '\0' marks the end of a string
	{
		uart_putc(UART_ID_OF_BLUETOOTH, str[n]);
		n += 1;
	}
}

char bluetooth_getc()
{
	return uart_getc(UART_ID_OF_BLUETOOTH); //receive ascii value from PmodBT2
}

void bluetooth_init()
{
    uart_config(UART_ID_OF_BLUETOOTH, UART_RATE_BLUETOOTH);
}

void _bluetooth_send(char *ch, int size)
{
	int i;
	for(i=0; i<size; i++)
	{
		uart_putc(UART_ID_OF_BLUETOOTH, ch[i]);
	}
}

void _bluetooth_recv(char *ch, int size)
{
	int i;
	for(i=0; i<size; i++)
	{
		ch[i] = uart_getc(UART_ID_OF_BLUETOOTH);
	}
}

void _bluetooth_wait_ack()
{
    char data;
    while(1)
    {   
        data = bluetooth_getc();
        if(data == 'A') break;
    }
}

int bluetooth_send(char *data, int size)
{
    int cnt = 0;
	int remaining_size;

    _bluetooth_send(&size, sizeof(size));
    _bluetooth_wait_ack();

	remaining_size = size;
    while(1)
    {
		if(remaining_size > BUFFER_SIZE_OF_BLUETOOTH) 
		{
			_bluetooth_send(data, BUFFER_SIZE_OF_BLUETOOTH);
			_bluetooth_wait_ack();
		}
		else
		{
			_bluetooth_send(data, remaining_size);
			_bluetooth_wait_ack();
			break;
		}
		data += BUFFER_SIZE_OF_BLUETOOTH;
		cnt += BUFFER_SIZE_OF_BLUETOOTH;
		remaining_size -= BUFFER_SIZE_OF_BLUETOOTH;
    }
	cnt += remaining_size;

    return cnt;
}

void _bluetooth_send_ack()
{
	bluetooth_putc('A');
}

int bluetooth_recv(char *data, int max_size)
{
	int received_data_size;
	int remaining_size;

	_bluetooth_recv(&received_data_size, sizeof(received_data_size));
	_bluetooth_send_ack();
	if(received_data_size > max_size)
	{
		printf("the rx size (%d) is larger than the max size (%d)\n", received_data_size, max_size);
		return 0;
	}

	remaining_size = received_data_size;
    while(1)
    {
		if(remaining_size > BUFFER_SIZE_OF_BLUETOOTH) 
		{
			_bluetooth_recv(data, BUFFER_SIZE_OF_BLUETOOTH);
			_bluetooth_send_ack();
		}
		else
		{
			_bluetooth_recv(data, remaining_size);
			_bluetooth_send_ack();
			break;
		}
		data += BUFFER_SIZE_OF_BLUETOOTH;
		remaining_size -= BUFFER_SIZE_OF_BLUETOOTH;
    }

    return received_data_size;
}
