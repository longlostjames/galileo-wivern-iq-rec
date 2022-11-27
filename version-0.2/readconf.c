#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include"readconf.h"


double getconf_double(char *filename, char *keyword)
{
  // Uses getconf to read a double from a space-delimited config file.
  // Looks for the value preceded by the given keyword in the given filename.
  char valuestr[80];
  double value;
  char *end;

  if( getconf(filename,keyword,valuestr) == 1) {
    printf("** getconf_double: Keyword %s not found.\n",keyword);
    return(0);
  }
  else {
    value=strtod(valuestr,&end);
    return value;
  }
}

float getconf_float(char *filename, char *keyword)
{
  // Uses getconf to read a double from a space-delimited config file.
  // Looks for the value preceded by the given keyword in the given filename.
  char valuestr[80];
  float value;
  char *end;

  if( getconf(filename,keyword,valuestr) == 1) {
    printf("** getconf_float: Keyword %s not found.\n",keyword);
    return(0);
  }
  else {
    value=strtod(valuestr,&end);
    return value;
  }
}

  
int getconf(char *filename, char *keyword, char *value)
{
  // Reads a string from a space-delimited config file.
  // Reads the string preceded by the given keyword in the given filename.
  char buffer[255];
  char *p;
  FILE *fptr;
  int status=1;

  fptr=fopen(filename,"r");
  if(fptr==NULL) {
    printf("** getconf: Unable to open %s\n",filename);
    return(status);
  }
  do {
    if(fgets(buffer, 255, fptr)==(char *)NULL)
      {
	printf("** getconf: Error reading file.\n");
	return(status);
      }
    
    p = strstr(buffer,keyword);
    if(p != NULL && buffer[0] != '#') {
      p = buffer + strlen(keyword) + 1;
      strcpy(value,p);
      value[strlen(value)-1]='\0';
      status=0;
      break;
    }
  } while( feof(fptr) != EOF || p != NULL );
  
  fclose(fptr);
  return(status);
}


