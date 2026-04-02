#ifndef __ERVP_COLOR_FORMAT_H__
#define __ERVP_COLOR_FORMAT_H__

#include <stdint.h>

#include "ervp_printf.h"
#include "ervp_misc_util.h"
#include "ervp_assert.h"

#define COLOR_FORMAT_SPECIAL 0
#define COLOR_FORMAT_RGB 1
#define COLOR_FORMAT_YUV 2

// variables which has specified 
typedef union
{
	uint16_t packed;
	struct
	{
		unsigned int b : 5;
		unsigned int g : 6;
		unsigned int r : 5;
		unsigned int unused;
	} br;
} rgb565_t;

typedef union
{
	uint32_t packed;
	struct
	{
		uint8_t x;
		uint8_t b;
		uint8_t g;
		uint8_t r;
	} br;
} xbgr8888_t;

inline static uint8_t extract_r_from_yuv(uint8_t y, uint8_t u, uint8_t v)
{
	return (uint8_t)(CLIP((1192 * (y - 16) + 1634 * (v - 128)) >> 10, 0, 0xFF));
}

inline static uint8_t extract_g_from_yuv(uint8_t y, uint8_t u, uint8_t v)
{
	return (uint8_t)(CLIP((1192 * (y - 16) - 400 * (u - 128) - 833 * (v - 128)) >> 10, 0, 0xFF));
}

inline static uint8_t extract_b_from_yuv(uint8_t y, uint8_t u, uint8_t v)
{
	return (uint8_t)(CLIP((1192 * (y - 16) + 2066 * (u - 128)) >> 10, 0, 0xFF));
}

inline static uint16_t gen_rgb565(uint8_t r, uint8_t g, uint8_t b)
{
	rgb565_t value;
	value.br.b = (b>>3);
	value.br.g = (g>>2);
	value.br.r = (r>>3);
  	return value.packed;
}

inline static uint8_t extract_r_from_rgb565(uint16_t rgb565)
{
	rgb565_t value;
	value.packed = rgb565;
	return value.br.r << 3;
}

inline static uint8_t extract_g_from_rgb565(uint16_t rgb565)
{
	rgb565_t value;
	value.packed = rgb565;
	return value.br.g << 2;
}

inline static uint8_t extract_b_from_rgb565(uint16_t rgb565)
{
	rgb565_t value;
	value.packed = rgb565;
	return value.br.b << 3;
}

inline static uint32_t gen_xbgr8888(uint8_t x, uint8_t r, uint8_t g, uint8_t b)
{
	xbgr8888_t value;
	value.br.x = x;
	value.br.b = b;
	value.br.g = g;
	value.br.r = r;
	return value.packed;
}

inline static uint8_t extract_r_from_xbgr8888(uint32_t xbgr8888)
{
	xbgr8888_t value;
	value.packed = xbgr8888;
	return value.br.r;
}

inline static uint8_t extract_g_from_xbgr8888(uint32_t xbgr8888)
{
	xbgr8888_t value;
	value.packed = xbgr8888;
	return value.br.g;
}

inline static uint8_t extract_b_from_xbgr8888(uint32_t xbgr8888)
{
	xbgr8888_t value;
	value.packed = xbgr8888;
	return value.br.b;
}

static inline uint32_t gen_xbgr8888_from_rgb565(uint16_t rgb565)
{
	uint8_t r, g, b;
	r = extract_r_from_rgb565(rgb565);
	g = extract_g_from_rgb565(rgb565);
	b = extract_b_from_rgb565(rgb565);
	return gen_xbgr8888(0, r, g, b);
}

static inline uint16_t gen_rgb565_from_xbgr8888(uint32_t xbgr8888)
{
	uint8_t r, g, b;
	r = extract_r_from_xbgr8888(xbgr8888);
	g = extract_g_from_xbgr8888(xbgr8888);
	b = extract_b_from_xbgr8888(xbgr8888);
	return gen_rgb565(r, g, b);
}

static inline uint32_t gen_xbgr8888_from_y8(uint8_t y8)
{
	return gen_xbgr8888(0, y8, y8, y8);
}

static inline uint8_t rgb_to_luma(uint8_t r, uint8_t g, uint8_t b)
{
	//return 0.299*r + 0.587*g + 0.114*b;		// YCrCb, ITU-R BT.601
	//return 0.2126*r + 0.7152*g + 0.0722*b;	// YCrCb, ITU-R BT.709
	return (13933*r + 46871*g + 4732*b) >> 16;
}

static inline uint8_t rgb565_to_luma(uint16_t rgb565)
{
	uint8_t r,g,b;
	r = extract_r_from_rgb565(rgb565);
	g = extract_g_from_rgb565(rgb565);
	b = extract_b_from_rgb565(rgb565);
	return rgb_to_luma(r, g, b);
}

static inline uint8_t xbgr8888_to_luma(uint32_t xbgr8888)
{
	uint8_t r, g, b;
	r = extract_r_from_xbgr8888(xbgr8888);
	g = extract_g_from_xbgr8888(xbgr8888);
	b = extract_b_from_xbgr8888(xbgr8888);
	return rgb_to_luma(r, g, b);
}

#endif
