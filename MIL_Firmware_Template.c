/*
*	Board/Project Name: Template Board
*	Author: Marquez Jones
*	Desc: Standardized template example created to promote readability of code
*		  Noboby will probably use this but it exists anyway
*/

/*********************************************************************INCLUDES*********************************************************************/

/* put includes here*/
#include "Yeet.h"

/*********************************************************************DEFINES**********************************************************************/

/* put defines here*/
#define THIRTY_SEVEN 37

/*********************************************************************GLOBAL DATA******************************************************************/

/*put global data here( isr flags, etc)*/
uint8_t yeet_flag = 0;

/*********************************************************************FUNCTION PROTOTYPES**********************************************************/

/*by declaration of Marquez, you will use function prototypes*/

//adds A and B
uint8_t  sumAB(uint8_t A, uint8_t B);

//initializes Yeet peripheral
void init_Yeet(void);

/*********************************************************************MAIN************************************************************************/

int main(void){
	
	init_Yeet();
	
	while(1){
		
		uint8_t A = 1;
		uint8_t B = 1;
		
		C = sumAB(A, B);
		
		
	}
	
}

/*********************************************************************INTERRUPT SERVICE ROUTINES****************************************************/


/* define ISRs here */


/*********************************************************************FUNCTION DEFINITIONS**********************************************************/

//adds A and B
uint8_t  sumAB(uint8_t A, uint8_t B){
	
	return A + B;
	
}

//initializes Yeet peripheral
void init_Yeet(void);{
	
    yeet_flag = THIRTY_SEVEN;
	
}



















