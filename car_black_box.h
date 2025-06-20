/* 
 * File:   car_black_box.h
 * Author: harid
 *
 * Created on 20 December, 2024, 10:05 AM
 */

#ifndef CAR_BLACK_BOX_H
#define	CAR_BLACK_BOX_H

void clear_screen(void);
void display_dashboard(unsigned char[], unsigned char);
void log_event(unsigned char[], unsigned char);
unsigned char  login(unsigned char key, unsigned char reset_flag);
unsigned char menu_screen(unsigned char key, unsigned char reset_flag);
unsigned char view_log(unsigned char key, unsigned char reset_flag);
unsigned char clear_log(void);
unsigned char download_log(void);
unsigned char change_time(unsigned char key, unsigned char reset_flag);
unsigned char change_password(unsigned char key, unsigned char reset_flag);

#endif	/* CAR_BLACK_BOX_H */

