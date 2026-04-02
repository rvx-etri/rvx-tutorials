#include "ervp_timer.h"
#include "ervp_platform_controller_api.h"

#ifdef INCLUDE_FUSE_BOX
#include "ervp_fuse_box.h"
#endif

static inline unsigned int convert_us_to_tick(unsigned int period_us)
{
	unsigned int tick_count;
	tick_count = ((unsigned int)(TICK_HZ/1000000))*period_us;
	return tick_count;
}

void config_timer_interval_us(unsigned int period_us)
{
	_config_timer_interval(convert_us_to_tick(period_us));
}

void config_timer_interval_ms(unsigned int period_ms)
{
	config_timer_interval_us(period_ms*1000);
}

void start_timer_once_ms(unsigned int period_ms)
{
	config_timer_interval_ms(period_ms);
	start_timer_once();
}

static inline void _config_timer_delay(unsigned int delay_tick)
{
	REG32(MMAP_TIMER_DELAY) = delay_tick;
}

static inline void config_timer_delay_us(unsigned int delay_us)
{
	_config_timer_delay(convert_us_to_tick(delay_us));
}

static inline void config_timer_delay_ms(unsigned int delay_ms)
{
	config_timer_delay_us(delay_ms*1000);
}

static inline unsigned int is_delay_working()
{
	return (REG32(MMAP_TIMER_CMD)>>1);
}

void delay_us_by_timer(unsigned int time_in_us)
{
	config_timer_delay_us(time_in_us);
	set_timer_cmd(TIMER_CMD_DELAY);
#ifdef INCLUDE_FUSE_BOX
	if(time_in_us > 50)
		sleep_self();
#endif
	while(is_delay_working());
}

static inline void __delay_by_timer(unsigned int time_in_unit, unsigned int scale, unsigned int bw_lower)
{
	unsigned int upper, lower;
	unsigned int i;
	upper = time_in_unit >> bw_lower;
	lower = ((time_in_unit << bw_lower) >> bw_lower);
	for(i=0; i<upper; i++)
	{
		delay_us_by_timer((1<<bw_lower)*scale);
	}
	delay_us_by_timer(lower*scale);
}

void delay_ms_by_timer(unsigned int time_in_ms)
{
	__delay_by_timer(time_in_ms, 1000, 20);
}

void delay_sec_by_timer(unsigned int time_in_sec)
{
	__delay_by_timer(time_in_sec, 1000000, 10);
}
