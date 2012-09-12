#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <lab01.h>


//Sean Dalrymple
//Cosc 301
//Lab01 Function File

void removewhitespace( char *mystr){
     if (mystr == NULL){ 
          return;
     }
     int len = strlen(mystr);
     int nowhite = 0;
     int tempdex;
     char *rmvwht;
     int i = 0;
     for(; i<len; i++){
          if (!(isspace(mystr[i]))){
               nowhite++;
          }
     }      
     char tempstr [nowhite+1];//create array of size (nowhite +1)
     rmvwht = &tempstr[0];
     rmvwht[nowhite] = '\0';//last char is '\0'
     int j = 0;
     for (; j<len; j++){//copy all non white space to temporary array
           if (!(isspace(mystr[j]))){
               rmvwht[tempdex] = mystr[j];
               tempdex++;
           }
     }
     int k = 0;
     for (; k<nowhite; k++){//copy shortened string back to original
          mystr[k] = rmvwht[k];
          }
     mystr[nowhite] = '\0';
     return;

}

char *c2pascal( char *mystr){
     if (mystr == NULL){
          return NULL;
     }
     int len = strlen(mystr);
     char *pstr;
     pstr = (char * ) malloc((sizeof(char)*(len+1)));
     pstr[0] = len;
     int i = 0;
     for(; i<len;i++){
          pstr[i+1] = mystr[i];
     }
     return pstr; 
}

char *pascal2c( char *pstr){
     if (pstr == NULL){
          return NULL;
     }
     char *cstr = malloc(sizeof(char)*(pstr[0]+1));
     int len = pstr[0];
     int i = 0;
     for(; i<len;i++){//copy values of pascal string to c string
          cstr[i] = pstr[i+1];
     }
     cstr[i] = '\0';
     return cstr;
}
char **tokenify (char *s){
     if (s == NULL){
          return NULL;
     }
     int letstate = 0;
     int wordcount = 0;
     int tdex = 0;
     int len = strlen(s);
     int begin, end;
     int i = 0;
     for(; i<len; i++){//go through and count the number of words
          if ( (!(isspace(s[i]))) && (!letstate) ){//start of a word
               wordcount++;
               letstate++; 
          }
          if ( isspace(s[i]) && letstate){//end of a word
               letstate = 0;
          }
     }
     char **tkfy  = (char **) malloc(sizeof(char*)*(wordcount+1));
     letstate = 0;
     int j = 0;
     for(; j<len; j++){
          if ( (!(isspace(s[j]))) && (!letstate) ){//start of a word
               begin = j;
               letstate++; 
          }
          if (isspace(s[j]) && letstate){//end of a word
               end = j;
               char holder = s[j];
               s[end] = '\0';
               char *tempstr;
               tempstr = strdup(&s[begin]);
               s[end] = holder;
               tkfy[tdex] = tempstr;
               tdex++; 
               letstate = 0;
          }
     }
     tkfy[tdex] = NULL;
     return tkfy;
}

 
         



