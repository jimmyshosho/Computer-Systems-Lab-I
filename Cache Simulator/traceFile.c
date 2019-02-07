// Group Members: Dhimiter Shosho, Alexander Donadio, Sam Fick
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

int size =8; // biggest hexidecimal number will be 8 characters
FILE *trace;// pointer for trace file
unsigned int *address;// pointer used for malloc

int C; // total cache size
int K; // number of lines per set in cache (set assocativity)
int L; // block length in bytes
int i; // declaration for for loop variable
int j; // declaration for for loop variable
int negativeOne = -1;

double total= 0;
double missCount=0;

int numSets;    // number of sets in cache
int **tagArray; // 2D array for the tags of the addresses
int **lruArray; // 2D array for LRU status of the addresses

// DECLERATION OF FUNCTIONS
int whichSet();      //Outputs the cache set in which the address falls
int setIndexLength();//Outputs the number of bits in the set index field of address
int offsetLength();  //Outputs the number of bits in the line offset field of address
int tagBits();       //Outputs the tag bits associated with the address
int hitWay(int** tagarr, int currentIndex, int currentTag);//If there is a hit, this outputs the cache way in which the accessed line can be found; it returns −1 if there is a cache miss.
void updateOnHit(int** tagarr, int** lruarr, int currentIndex, int currentTag, int block);  //Updates the tagArray and lruArray upon a hit. This function is only called on a cache hit
void updateOnMiss(int** tagarr, int** lruarr, int currentIndex, int currentTag); //Updates the tagArray and lruArray upon a miss. This functionis only called on a cache miss.

// HELPER FUNCTIONS
int logB2(int a); // helper method used for getting index bits, offset bits, etc
int tagBitsLength(unsigned int* currentAddress); // helper method used for getting number of tag bits
int numBits(unsigned int* currentAddress);// helper method used for counting total number of bits in given address

int main(int argc, char *argv[])
{
  K = atoi(argv[1]);
  L = atoi(argv[2]);
  C = atoi(argv[3]) * 1024;

  int numRows = C/(K*L);
  int numCols = K;

  tagArray = (int **) malloc(numRows*sizeof(int*)); // creates rows in array with C/K*L rows
  lruArray = (int **) malloc(numRows*sizeof(int*)); // creates rows in array with C/K*L rows

  for(i = 0; i<numRows;i++)
    {
      *(tagArray + i) = (int*) malloc(numCols*sizeof(int)); // fills each row with K columns
      *(lruArray + i) = (int*) malloc(numCols*sizeof(int)); // fills each row with K columns
    }

  for(i = 0; i<numRows; i++)
    for(j = 0; j<numCols; j++)
      {
        tagArray[i][j] = negativeOne;
        lruArray[i][j] = negativeOne;
      }

  trace = fopen(argv[4],"r"); // we use the file pointer to point to traces.txt
  address = (int*) malloc(size*sizeof(int));// makes address an "array"

    while( fscanf(trace, "%x", address) != EOF)// continues to scan file until there is nothing left
  {  // address now contains the newest address in the file
    //address = (int*) malloc(size*sizeof(int));// makes address an "array"
    total = total+1;
    int index = whichSet(address); // which row
    int tag = tagBits(address); // used to find hit/miss

    int hit = hitWay(tagArray,index,tag);
    if(hit == -1)
    {
      updateOnMiss(tagArray,lruArray,index,tag);
      missCount = missCount+1;
    }
    else
      updateOnHit(tagArray,lruArray,index, tag,hit);

    address = (int*) realloc(address,8); // clears address array
  }

  double  missRate = missCount/total;
  printf("%s %i %i %i %.4f\n",argv[4], C/1024, K, L, missRate);
  free(address);
  free(lruArray);
  free(tagArray);
  return 1;
}


int whichSet(unsigned int* currentAddress) //Outputs the cache set in which the address falls (set index)
{
  unsigned int tempAddress = *currentAddress;
  int shiftLeft = tagBitsLength(currentAddress);
  tempAddress = tempAddress << shiftLeft;
  int shiftRight = shiftLeft + offsetLength();
  tempAddress = tempAddress >> shiftRight;
  return tempAddress;
}

int tagBits(unsigned int* currentAddress) //Outputs the tag bits associated with the address
{
  unsigned int tempAddress = *currentAddress;
  int shift = offsetLength()+setIndexLength();
  return tempAddress >> shift;
}

int setIndexLength() //Outputs the number of bits in the set index field of address
{
  return logB2(C/(K*L));
}

int offsetLength() //Outputs the number of bits in the line offset field of address
{
  return logB2(L*8);
}

int hitWay(int** tagarr, int currentIndex, int currentTag)
//If there is a hit, this outputs the cache way in which the accessed line can be found; it returns −1 if there is a cache miss.
{
  int hit = -1;
  for(j=0; j<K; j++)
    if(tagarr[currentIndex][j] == currentTag)
      hit=j;

  return hit;
}

void updateOnHit(int** tagarr, int** lruarr, int currentIndex, int currentTag, int block)
{
  tagarr[currentIndex][block] = currentTag;
  for(j =0; j<K;j++)
    if( (-1<lruarr[currentIndex][j]) && (lruarr[currentIndex][j]<lruarr[currentIndex][block]))
        lruarr[currentIndex][j] = lruarr[currentIndex][j]+1;
    lruarr[currentIndex][block] = 0;
}

void updateOnMiss(int** tagarr, int** lruarr, int currentIndex, int tag)
{
  // case for when row is not full
  int count = 0;
  while (lruarr[currentIndex][count] != -1)
  {
    lruarr[currentIndex][count] = lruarr[currentIndex][count] + 1;
    count++;
  }
// case for when row is full
if( count <= K-1)
{
  lruarr[currentIndex][count]= 0;
  tagarr[currentIndex][count] = tag;
}
else
{
  for(j=0; j<K; j++)
  {
    if( lruarr[currentIndex][j] == (K-1) )
    {
      lruarr[currentIndex][j] = 0;
      tagarr[currentIndex][j] = tag;
    }
    else
        lruarr[currentIndex][j] = lruarr[currentIndex][j]+1;
  }
}



}

int logB2(int a)
{
  int count=0;
    while( a%2 == 0)
    {
      count++;
      a = a/2;
    }
  return count;
}

int numBits(unsigned int* currentAddress) // returns total number of bits in address
{
  unsigned int tempAddress = *currentAddress;
  int count = 0;
  while (tempAddress != 0)
  {
    tempAddress = tempAddress >> 1;
    count = count+1;
  }

  int total = count/4;
  if(count%4 != 0)
    total=total + 1;

  return total*4;
}

int tagBitsLength(unsigned int* currentAddress)// helper method used for getting number of tag bits
{
    int temp;
    unsigned int tempAddress = *currentAddress;
    if (numBits(currentAddress) != 32)
      temp = numBits(currentAddress) + (32-numBits(currentAddress));
    else
      temp = numBits(currentAddress);

   return temp-setIndexLength()-offsetLength();
}
