#include "ervp_user_gpio.h"

void user_gpio_set_config(unsigned int index, unsigned int config)
{
  gpio_set_config(get_user_gpio_addr(index),config);
}

void user_gpio_set_output(unsigned int index, int value)
{
	gpio_set_output(get_user_gpio_addr(index),value);
}

void user_gpio_set_input_cfg(unsigned int index, unsigned int config)
{
	gpio_set_input_cfg(get_user_gpio_addr(index),config);
}

int user_gpio_get_input(unsigned int index)
{
	return gpio_get_input(get_user_gpio_addr(index));
}

void user_gpio_enable_interrupt(unsigned int index, unsigned int cond, int compare_value)
{
	gpio_enable_interrupt(get_user_gpio_addr(index), cond, compare_value);
}

void user_gpio_disable_interrupt(unsigned int index)
{
  gpio_disable_interrupt(get_user_gpio_addr(index));
}
