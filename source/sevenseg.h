// Permission to copy is granted provided that this header remains intact. 
// This software is provided with no warranties.

////////////////////////////////////////////////////////////////////////////////

#ifndef SEVEN_SEG_H
#define SEVEN_SEG_H

// Seven Seg Enumeration of characters
enum sevenSeg { 
//  _           _     _           _     _     _     _     _ 
// | |     |    _|    _|   |_|   |_    |_      |   |_|   |_|
// |_|     |   |_     _|     |    _|   |_|     |   |_|    _|
   SS_0, SS_1, SS_2, SS_3, SS_4, SS_5, SS_6, SS_7, SS_8, SS_9};
//  
//  _
// 
// Default
	
////////////////////////////////////////////////////////////////////////////////
//Functionality - Writes output to a 7-segment display
//Parameter: An unsigned character to output
//Returns: None
unsigned char Write7Seg(unsigned char c) {
	
	// Using the following table:
		//		_A_
		//	F |		| B 
		//		_G_
		//	E |		| C
		//		_D_		. DP
	
	// Match your port and pin numbers:
	
	static unsigned char sevenSeg;	// Seven segment port
	static const unsigned char sevenSeg_A		= 2; 		// Pin for segment A
	static const unsigned char sevenSeg_B		= 3; 		// Pin for segment B
	static const unsigned char sevenSeg_C		= 6; 		// Pin for segment C
	static const unsigned char sevenSeg_D		= 5; 		// Pin for segment D
	static const unsigned char sevenSeg_E		= 4; 		// Pin for segment E
	static const unsigned char sevenSeg_F		= 1; 		// Pin for segment F
	static const unsigned char sevenSeg_G		= 0; 		// Pin for segment G
	// static const unsigned char sevenSeg_DP		= 7; 		// Pin for segment DP (UNUSED)
	
	// Turns on all segments
	// *sevenSeg_Port =	(1<<sevenSeg_A)	|	(1<<sevenSeg_B) |	(1<<sevenSeg_C) |	(1<<sevenSeg_D) |
	//					(1<<sevenSeg_E)	|	(1<<sevenSeg_F) |	(1<<sevenSeg_G) |	(1<<sevenSeg_DP);	
	
	// Seven Seg character defintions
	switch (c) {
		case SS_0:
			sevenSeg =	(1<<sevenSeg_A)	|	(1<<sevenSeg_B) |	(1<<sevenSeg_C) |	(1<<sevenSeg_D) |
								(1<<sevenSeg_E)	|	(1<<sevenSeg_F)	;
			break;
		case SS_1:
			sevenSeg =	(1<<sevenSeg_B) |	(1<<sevenSeg_C) ;
			break;
		case SS_2:
			sevenSeg =	(1<<sevenSeg_A)	|	(1<<sevenSeg_B) |	(1<<sevenSeg_D) |	
								(1<<sevenSeg_E)	|	(1<<sevenSeg_G) ;
			break;
		case SS_3:
			sevenSeg =	(1<<sevenSeg_A)	|	(1<<sevenSeg_B) |	(1<<sevenSeg_C) |	(1<<sevenSeg_D) |
								(1<<sevenSeg_G) ;
			break;
		case SS_4:
			sevenSeg =	(1<<sevenSeg_B) |	(1<<sevenSeg_C)	|	
								(1<<sevenSeg_F) |	(1<<sevenSeg_G) ;
			break;
		case SS_5:
			sevenSeg =	(1<<sevenSeg_A)	|	(1<<sevenSeg_C) |	(1<<sevenSeg_D) |
								(1<<sevenSeg_F) |	(1<<sevenSeg_G) ;
			break;
		case SS_6:
			sevenSeg =	(1<<sevenSeg_A)	|	(1<<sevenSeg_C) |	(1<<sevenSeg_D) |
								(1<<sevenSeg_E)	|	(1<<sevenSeg_F) |	(1<<sevenSeg_G) ;
			break;
		case SS_7:
			sevenSeg =	(1<<sevenSeg_A)	|	(1<<sevenSeg_B) |	(1<<sevenSeg_C) ;
			break;
		case SS_8:
			sevenSeg =	(1<<sevenSeg_A)	|	(1<<sevenSeg_B) |	(1<<sevenSeg_C) |	(1<<sevenSeg_D) |
								(1<<sevenSeg_E)	|	(1<<sevenSeg_F) |	(1<<sevenSeg_G) ;
			break;
		case SS_9:
			sevenSeg =	(1<<sevenSeg_A)	|	(1<<sevenSeg_B) |	(1<<sevenSeg_C) |	(1<<sevenSeg_D) |
								(1<<sevenSeg_F) |	(1<<sevenSeg_G) ;
			break;
		default:
			sevenSeg =	(1<<sevenSeg_G) ;
		break;
	}

	return sevenSeg;
}

#endif // SEVEN_SEG_H
