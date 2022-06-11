// Tim Driscoll fastsort.c 2/5/20

//Include neccessary libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char** argv)
{
  //Declare needed local variables
  int wordNum = 1;
  FILE *textFile,*fileSize;

  //Deal with the command line arguments
  if(argc == 2)
    {
      //only two arguments should be just text file
      wordNum = 1;
      textFile = fopen(argv[1],"r+");
      fileSize = fopen(argv[1],"r+");

      //Check to ensure text files can be opened
      if(textFile == NULL)
        {
          fprintf(stderr,"Error: Cannot open file %s with no extra spaces\n",argv[1]);
          return(1);
        }
      if(fileSize == NULL)
        {
          fprintf(stderr,"Error: Cannot open file %s with no extra spaces\n",argv[1]);
          return(1);
        }
    }
  else if(argc > 2)
    {
      // Check for correct command line parameters
      if(argv[1][0] != '-' && abs(atoi(argv[1])) >= 1)
        {
          fprintf(stderr,"Error: Bad command line parameters\n");
          return(1);
        }
      wordNum = abs(atoi(argv[1]));
      textFile = fopen(argv[2],"r");
      fileSize = fopen(argv[2],"r");
      if(textFile == NULL)
        {
          fprintf(stderr,"Error: Cannot open file %s with no extra spaces\n",argv[1]);
          return(1);
        }
      if(fileSize == NULL)
        {
          fprintf(stderr,"Error: Cannot open file %s with no extra spaces\n",argv[1]);
          return(1);
        }
    }

  // Reading in the data from the file
  char lineBuffer[128];
  int maxLineLength = 150;
  int i = 0;
  int ii = 0;

  //determine the number of lines in file
  int numLines = 0;
  while(fgets(lineBuffer,maxLineLength,fileSize))
    {

      numLines++;
    }

  //close first instance of file, no longer needed
  fclose(fileSize);

  //allocate space for two two dimensional arrays one for all text one for words
  char **text = (char **)malloc((numLines)*sizeof(char *));
  char **words = (char **)malloc((numLines)*sizeof(char *));

  //Check to make sure that malloc worked
  if(text == NULL || words == NULL)
    {
      fprintf(stderr,"malloc failed\n");
      return(1);
    }
  //Allocate space for second dimension of arrays
  for(i = 0; i < numLines; i++)
    {
      text[i] = (char *)malloc((maxLineLength)*sizeof(char));
      words[i] = (char *)malloc((maxLineLength)*sizeof(char));

      if(text[i] == NULL || words[i] == NULL)
        {
          fprintf(stderr,"malloc failed\n");
          return(1);
        }
   }

   //Read in the text and determine number of words
  i = 0;
  int *numOfWords = (int *)malloc((numLines)*sizeof(int));
  int lineLength;

  while(fgets(lineBuffer,maxLineLength,textFile))
    {

      lineLength = strlen(lineBuffer);
      //Check the line length
      if(lineLength > 128)
        {
          fprintf(stderr,"Line too long.\n");
          exit(1);
        }

      //copy the line from the buffer array to the text array
      strcpy(text[i],lineBuffer);

      //Determine number of words in each line
      numOfWords[i] = 1;
      for(ii = 0; ii < lineLength-1; ii++)
        {
          if(lineBuffer[ii] == ' ' && lineBuffer[ii+1] != ' ')
            numOfWords[i]++;
        }
      i++;
    }

  //Close the input file, no longer needed (all data has been read in)
  fclose(textFile);

  //Create the array of words to be compared
  int j = 0;
  int length = 0;
  int n = 0;
  int k = 0;
  int wFound = 0;

  for(i=0; i < numLines; i++)
    {
      //If only one word or one word in line add this word to words array
      if(wordNum == 1 || numOfWords[i] == 1)
      {
        j = 0;
        //check for blank line (just the newline character or just spaces)
        if(text[i][j] == '\n')
          {
            words[i][j] = text[i][j];
          }
        else if(text[i][j] == ' ')
          {
            words[i][j] = text[i][j];
          }
        else
          {
            while(text[i][j] != ' ')
              {
                words[i][j] = text[i][j];
                j++;
              }
          }
      }
      else
        {
          //split up the entire string into words then select desired word
          //Looking to add some word other then the first from text array
          length = strlen(text[i]);
          n = 0;
          k = 0;
          wFound = 0;
          //temp two dimensional array to store words from line split up
          char splitStr[numOfWords[i]][128];
          for(j = 0; j < length; j++)
            {
              //Add letters to words while no spaces(and newlines occur)
              if(text[i][j] != ' ' && text[i][j] != '\n')
                {
                  splitStr[n][k++] = text[i][j];
                }
              else
                {
                  splitStr[n][k++] = '\0';
                  k = 0;

                  //Parse over any extra spaces between words
                  while(text[i][j+1] == ' ' )
                    j++;

                  //Obtain desired number given user input or last word if there
                  //isnt enough words
                  if(wordNum < numOfWords[i] && (wordNum-1) == n)
                    {
                      strcpy(words[i],splitStr[n]);
                      wFound = 1;
                    }
                  else if((numOfWords[i]-1) == n && wFound != 1)
                    {
                      strcpy(words[i],splitStr[n]);
                    }
                  n++;
                }
            }
        }
    }


  //Perform the sort
  i = j = 0;
  char lineSwitch[maxLineLength];
  char wordSwitch[maxLineLength];

  //Nested for loops to run through array and compare each word (simple sort)
  for(i = 0; i < numLines-1; i++)
    {
      for(j = 0; j < numLines-1; j++)
        {
          //Make switch based off of word it neccessary
          if(strcmp(words[j],words[j+1]) > 0)
            {
              strcpy(lineSwitch,text[j]);
              strcpy(wordSwitch,words[j]);
              strcpy(text[j],text[j+1]);
              strcpy(words[j],words[j+1]);
              strcpy(text[j+1],lineSwitch);
              strcpy(words[j+1],wordSwitch);
            }
        }
    }

//Print out all the sorted lines from the array
  for(i = 0; i < numLines; i++)
      printf("%s",text[i]);


  return 0;
}

