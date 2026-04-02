#ifndef __ERVP_I2S_MASTER_H__
#define __ERVP_I2S_MASTER_H__

#include "ervp_i2s_master_memorymap.h"
#include "ervp_mmio_util.h"

static inline void i2s_enable_tx()
{
	mmio_write_data(MMAP_I2S_MASTER_CMD, I2S_MASTER_CMD_ENABLE_TX);
}

static inline void i2s_disable_tx()
{
	mmio_write_data(MMAP_I2S_MASTER_CMD, I2S_MASTER_CMD_DISABLE_TX);
}

static inline void i2s_enable_rx()
{
	mmio_write_data(MMAP_I2S_MASTER_CMD, I2S_MASTER_CMD_ENABLE_RX);
}

static inline void i2s_disable_rx()
{
	mmio_write_data(MMAP_I2S_MASTER_CMD, I2S_MASTER_CMD_DISABLE_RX);
}

static inline void i2s_enable_rx_itr(int compare_value)
{
	mmio_write_data(MMAP_I2S_MASTER_RX_ITR_NUM, compare_value);
	mmio_write_data(MMAP_I2S_MASTER_CMD, I2S_MASTER_CMD_ENABLE_RX_ITR);
}

static inline void i2s_disable_rx_itr()
{
	mmio_write_data(MMAP_I2S_MASTER_CMD, I2S_MASTER_CMD_DISABLE_RX_ITR);
}

static inline void i2s_clear_rx_itr()
{
	mmio_write_data(MMAP_I2S_MASTER_CMD, I2S_MASTER_CMD_CLEAR_RX_ITR);
}

static inline void i2s_clear_status()
{
	mmio_write_data(MMAP_I2S_MASTER_CMD, I2S_MASTER_CMD_CLEAR_STATUS);
}

static inline unsigned int i2s_get_status()
{
	return mmio_read_data(MMAP_I2S_MASTER_STATUS);
}

static inline void i2s_write_tx_fifo(void* value_list, int length)
{
	mmfifo_write(MMAP_I2S_MASTER_TX_FIFO, value_list, length);
}

static inline void i2s_read_rx_fifo(void* value_list, int length)
{
	mmfifo_read(MMAP_I2S_MASTER_RX_FIFO, MMAP_I2S_MASTER_RX_FIFO_VALID_NUM, value_list, length);
}

static inline void i2s_read_rx_fifo_force(void* value_list, int length)
{
	mmfifo_read_force(MMAP_I2S_MASTER_RX_FIFO, value_list, length);
}

#endif
