#ifndef _H_SIL1136_H_
#define _H_SIL1136_H_

#define SIL1136_ID	(0x72)


void sil1136_init(void);
void sil1136_init_sg(int w, int h, int vsw, int vfp, int vbp, int hsw, int hfp, int hbp, int freqHz);



#endif