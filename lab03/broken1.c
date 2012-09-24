#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void
removewhitespace(char *s1)
{
    // remove whitespace from a string, in place
    if (!s1)
        return;
    int len = strlen(s1);
    char buffer[len];
    int i = 0;
    int j = 0;
    for (; i < strlen(s1); i++) {
        if (!isspace(s1[i])) {
            buffer[j] = s1[i];
            j++;
        }
    }
    buffer[j] = '\0';    
    strncpy(s1, buffer, (j+1));
}


char** tokenify(char *s)
{
    
    const char *sep=" \t\n";
    char *word = NULL;
    int len = strlen(s);
    char *s2 = (char *)malloc(sizeof(char)*(len+1));
    strncpy(s2,s,len+1);
    // find out exactly how many tokens we have
    int words = 0;
    word = strtok(s,sep);
    while (word){    
        printf("words: %d\n", words);
        printf("s: %s\n", word);
        word = strtok(NULL,sep);
        words++;
        }
   

    // allocate the array of char *'s, with one additional
    char **array = (char **)malloc(sizeof(char*)*(words+1));
    int i = 0;
    word = strtok(s2, sep);
    for (; i<words; i++) {
        printf("adding word %s to array pos %d\n", word, i);
        //printf("%s",word);
        array[i] = strdup(word);
        word = strtok(NULL, sep);
    }
    array[i] = NULL;
    free(s2);
    return array;
}

void
printtokens(char **tokenlist) {
    if (tokenlist == NULL){
        return;
    }
    //char *tmp = tokenlist[0];
    int toknum = 0;
    printf("Printing tokens:\n");
    while(tokenlist[toknum] != NULL) {
        printf("\t%d: <%s>\n", toknum, tokenlist[toknum]);
	    free(tokenlist[toknum]);
    	toknum++;
        }
    free(tokenlist);
}

int
main(int argc, char **argv) {
    char s1[] = "  the \tinternet is a series of tubes  ";
    char s2[] = "   \t\n";
    char s3[] = "  the \tinternet is not a series of tubes  ";
    char s4[] = "   \t\n";
    char s5[] = "8";
    
    removewhitespace(s1);
    removewhitespace(s2);
    printf("Remove whitespace - s1: %s\n", s1);
    printf("Remove whitespace - s2: %s\n", s2);
    

    printtokens(tokenify(s3));
    printtokens(tokenify(s4));
    printtokens(tokenify(s5));

    return 0;
}

