#ifndef MOCK_GPIO_H
#define MOCK_GPIO_H

/* GPIO pin definitions */
#define GPIO_LED1 1

/* Function prototypes */
void gpio_init(int pin);
void gpio_toggle(int pin);

#endif /* MOCK_GPIO_H */
