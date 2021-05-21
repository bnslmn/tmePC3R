/****
 * 
 * @author Amine Benslimane
 * amine.benslimane@etu.sorbonne-universite.fr
 * https://github/bnslmn
 * 
 * Producer-Consumer Model, Synchronization problem
 * 
 * Feel free to use this code, program under GPLv3
 * 
 * Sorbonne Université
 * March 2021,
 * 
 *
 * 
*/


/*
*   Function concat 
        IN :  Two strings str1 & str2
        OUT : String str1.str2  (concatenation)
*
*/
char * concat(char *str1, char *str2)
{
    int cp1 = 0, cp2=0 ;
    char *str = malloc(sizeof(char)* ( strlen(str1) + strlen(str2) +1 ) );
    while (*(str1+cp1) != '\0'){
     *(str+cp1) = *(str1+cp1) ;
     cp1++;
    }
 
    while(*(str2+cp2) != '\0'){
	 *(str+cp1) = *(str2+cp2) ;
     cp2++;
     cp1++;
    }

	return str;  
}

/*
*   Function digits
        IN :  an integer n
        OUT : [log n] (number of digits in a number)
*
*/
int digits(int n){
	int log =0;
	while(n != 0 && log++)
	n /= 10;
	
	return log;
}

char * toString(int i){
	char * str = malloc((digits(i)+1)* sizeof(char));
	sprintf(str, "%d", i);
	return str;
}



// ******************************************************
