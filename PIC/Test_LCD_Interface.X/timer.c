/**
 * @file
 * @author Nkemjika Okeke
 *
 * Created on February 3, 2019, 12:44 AM
 */

#include "timer.h"
#include "configure.h"

int atohtoi(unsigned char ascii){
   int dec_ascii = (int)ascii; 
   if (dec_ascii<10 && dec_ascii > 0){
     return dec_ascii;
   }
   else if (dec_ascii > 15 && dec_ascii < 26){
     return dec_ascii-6; 
   }
   else if (dec_ascii > 31 && dec_ascii < 42){
     return dec_ascii-12; 
   }
   else if (dec_ascii > 47 && dec_ascii < 58){
     return dec_ascii-18; 
   }
   else if (dec_ascii > 63 && dec_ascii < 74){
     return dec_ascii-24; 
   }
   if (dec_ascii > 79 && dec_ascii < 90){
     return dec_ascii-30; 
   }
   else{
     return -1;
   }
}



int char_to_int_2dig(char first, char second){
    int first_int = (int)first-48; 
    int second_int = (int)second-48;
    return first_int*10+second_int; //Return total entry 
}

float char_to_float_4dig(char first, char second, char third, char fourth){
    int first_int = (int)first-48; 
    int second_int = (int)second-48;
    int third_int = (int)third-48;
    int fourth_int = (int)fourth-48;
    return first_int*100+second_int*10+third_int+fourth_int*0.1; //Return total entry 
}

/**
 * @brief Uses char min/sec times to calculate time elapsed   
 * @param char character of hex number
 */
int time_elapsed(unsigned char begin_min, unsigned char begin_sec,unsigned char end_min,unsigned char end_sec, int*array){
    int b_min_int = atohtoi(begin_min); 
    int b_sec_int = atohtoi(begin_sec);
    int e_min_int = atohtoi(end_min);
    int e_sec_int = atohtoi(end_sec);
    int begin_s = b_min_int*60+b_sec_int;  //Start time in seconds
    int end_s = e_min_int*60+e_sec_int;    //End time in seconds
    int total_s = end_s - begin_s;         //Time elapsed in seconds 
    int minutes = total_s/60;              //Calculate minutes elapsed
    int seconds = total_s%60;              //Calculate seconds elapsed 
    array[0] = seconds;
    array[1] = minutes; 
    return 0; 
}