#ifndef __ERVP_MCOM_INPUT_H__
#define __ERVP_MCOM_INPUT_H__

int has_input_from_mcom(void);
int getc_from_mcom(void);
char mcom_gui_template(const char* title, const char* key_list, const char** menu_list);

#endif
