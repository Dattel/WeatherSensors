#ifndef helper_h
#define helper_h

//#include <string.h>

char *strrev(char *str);
char * long_to_binary(unsigned long long myulong);

char *strrev(char *str)
{
    if (!str || ! *str)
        return str;

    int i = strlen(str) - 1, j = 0;

    char ch;
    while (i > j)
    {
        ch = str[i];
        str[i] = str[j];
        str[j] = ch;
        i--;
        j++;
    }
    return str;
}

char * long_to_binary(unsigned long long myulong)
{
    unsigned long long tempDecimal;
    char binary[65];
    binary[0] = '\0';

    // Copy decimal value to temperature variable 
    tempDecimal = myulong;
    if (tempDecimal == 0)
        strcat(binary, "0");
        
    while(tempDecimal > 0)
    {
        strcat(binary, ((tempDecimal % 2) == 1)?"1":"0");
        tempDecimal /= 2;
    }
    
    //printf("Binary value of decimal = %s\n", binary);
    return strdup(strrev(binary));
}

#endif