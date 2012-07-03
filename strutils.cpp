#include "strutils.h"

unsigned int power(int base, int exponent)
{
   if (exponent == 0)
      return 1;
   else if (exponent%2 == 0)
      return power(base, exponent/2) * power(base, exponent/2);
   else
      return base * power(base, exponent - 1);
}

void chomp(char* string) {
   char* p = string;

   while (*(p++) != 0);

   while (*p == '\n' || *p == '\r')
      *(p--) = 0;
}

bool isNumber(char c) {
   if (c >= '0' && c <= '9')
      return true;
   else
      return false;
}

bool isNumber(char* string) {
   char* p = string;

   while (*p != 0) {
      if (!isNumber(*p))
         return false;

      p++;
   }

   return true;
}

unsigned int charToInt(char c) {
   return c - '0';
}

int stringToInt(char* string) {
   int n = 0;
   int sum = 0;

   if (!isNumber(string))
      return 0;

   char* p = string;

   while (*p != 0)
      p++;

   p--;

   while (p != string - 1) {
      sum += charToInt(*p) * power(10, n);

      p--;
      n++;
   }

   return sum;
}

int stringLength(char* string) {
   char* p = string;

   while (*(p++) != 0);
   
   return p - string;
}

void stringCopy(char* dst, char* src) {
   char* p1 = dst;
   char* p2 = src;
   
   while (*p2 != 0) {
      *p1 = *p2;
      p1++;
      p2++;
   }

   *p1 = 0;
}

bool stringEquals(char* s1, char* s2) {
   char* p1 = s1;
   char* p2 = s2;
   if (stringLength(s1) != stringLength(s2))
      return false;
      
   while(*p1 != 0 && *p2 != 0) {
      if (*(p1++) != *(p2++))
         return false;
   }
   
   return true;
}

bool stringContains(char* string, char c) {
   int len = stringLength(string);

   for (int i = 0; i < len; i++) {
      if (string[i] == c) {
         return true;
      }
   }

   return false;
}

bool isDelimiter(char c, char* delimiters) {
   int nDelimiters = stringLength(delimiters);

   for (int i = 0; i < nDelimiters; i++) {
      if (delimiters[i] == c)
         return true;
   }

   return false;
}

char** stringSplit(char* string, char* delimiters, int* nTokens) {
   char* p = string;
   char* token;
   char** tokens;
   int count = 0;
   bool delimiterFound = false;
   
   while(*p != 0) {
      if (isDelimiter(*p, delimiters) && !delimiterFound) {
         count++;
         delimiterFound = true;
      }
      else {
         delimiterFound = false;
      }
      
      p++;
   }
   
   count++;
   
   tokens = new char*[count];
   *nTokens = count;
   delimiterFound = false;
   token = p = string;
   count = 0;
   
   while(*p != 0) {
      if (isDelimiter(*p, delimiters) && !delimiterFound) {
         *p = 0;
         tokens[count++] = token;
         token = p + 1;
         delimiterFound = true;
      }
      else if (isDelimiter(*p, delimiters) && delimiterFound) {
         token++;
      }
      else {
         delimiterFound = false;
      }
      
      p++;
   }

   tokens[count] = token;
   
   return tokens;
}
