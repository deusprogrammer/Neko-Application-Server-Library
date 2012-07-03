#ifndef STRUTILS_H
#define STRUTILS_H
unsigned int power(int base, int exponent);
void chomp(char* string);
bool isNumber(char c);
bool isNumber(char* string);
unsigned int charToInt(char c);
int stringToInt(char* string);
int stringLength(char* string);
void stringCopy(char* dst, char* src);
bool stringEquals(char* s1, char* s2);
bool stringContains(char* string, char c);
bool isDelimiter(char c, char* delimiters);
char** stringSplit(char* string, char* delimiters, int* nTokens);
#endif
