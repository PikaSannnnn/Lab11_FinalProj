#ifndef __lcd_h__
#define __lcd_h__

void LCD_init();
void LCD_ClearScreen(void);
void LCD_WriteCommand (unsigned char Command);
void LCD_Cursor (unsigned char column);
void LCD_DisplayString(unsigned char column ,const unsigned char *string);	// modified
void LCD_Clean(unsigned char column);	// custom
void delay_ms(int miliSec);
#endif
