#ifndef __ERVP_IMAGE_FORMAT_DEFINE_H__
#define __ERVP_IMAGE_FORMAT_DEFINE_H__

typedef union
{
  unsigned int value;
  struct
  {
    unsigned int id : 8;
    unsigned int color_format : 8;
    unsigned int num_plane : 8;
    unsigned int is_packed : 8;
  } br;
} ErvpImageFormat;

#define GEN_IMAGE_FORMAT(id, color_format, num_plane, is_packed) (((is_packed&0xFF)<<24)|((num_plane&0xFF)<<16)|((color_format&0xFF)<<8)|((id&0xFF)<<0))

static inline unsigned int image_get_id(unsigned int format)
{
  ErvpImageFormat temp;
  temp.value = format;
  return temp.br.id;
}

static inline unsigned int image_is_packed(unsigned int format)
{
	ErvpImageFormat temp;
  temp.value = format;
  return temp.br.is_packed;
}

static inline unsigned int image_get_num_plane(unsigned int format)
{
  ErvpImageFormat temp;
  temp.value = format;
  return temp.br.num_plane;
}

#endif
