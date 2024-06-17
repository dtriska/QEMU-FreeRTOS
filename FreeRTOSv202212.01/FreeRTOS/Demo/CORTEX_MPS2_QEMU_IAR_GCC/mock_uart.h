#ifndef MOCK_UART_H
#define MOCK_UART_H

/* Function prototypes */
void uart_init(void);
void uart_set_interrupt_handler(void (*handler)(void));
int uart_read(char *buffer, int length);
void uart_write(const char *buffer, int length);

#endif /* MOCK_UART_H */
