#include "soft_fifo.h"

void soft_fifo_init(SoftFifo* fifo)
{
  fifo->wtag = 0;
  fifo->rtag = 0;
}

int soft_fifo_is_empty(const SoftFifo* fifo)
{
  return (fifo->wtag==fifo->rtag);
}

int soft_fifo_is_full(const SoftFifo* fifo)
{
  return ((fifo->wtag^fifo->rtag)==1);
}

int soft_fifo_valid_size(const SoftFifo* fifo)
{
  int rflag = fifo->rtag & 1;
  int rindex = fifo->rtag>>1;
  int wflag = fifo->wtag & 1;
  int windex = fifo->wtag>>1;
  if(rflag!=wflag)
    windex += FIFO_MAX_SIZE;
  return (windex - rindex);
}
void soft_fifo_push(SoftFifo* fifo, int value)
{
  int flag = fifo->wtag & 1;
  int index = fifo->wtag>>1;
  fifo->storage[index++] = value;
  if(index==FIFO_MAX_SIZE)
  {
    index = 0;
    flag = 1 - flag;
  }
  fifo->wtag = (index<<1) | flag;
}

int soft_fifo_pop(SoftFifo* fifo)
{
  int flag = fifo->rtag & 1;
  int index = fifo->rtag>>1;
  int result = fifo->storage[index++];
  if(index==FIFO_MAX_SIZE)
  {
    index = 0;
    flag = 1 - flag;
  }
  fifo->rtag = (index<<1) | flag;
  return result;
}
