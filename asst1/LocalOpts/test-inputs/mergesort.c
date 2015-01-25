#include <stdlib.h>
#include <string.h>

// Mix two sorted tables in one and split the result into these two tables.
void Mix(int *tab1,int *tab2,int count1,int count2)
{
  int i,i1,i2;
  i = i1 = i2 = 0;
  int * temp = (int *)malloc(sizeof(int)*(count1+count2));

  while((i1<count1) && (i2<count2))
  {
    while((i1<count1) && (*(tab1+i1)<=*(tab2+i2)))
    {
      *(temp+i++) = *(tab1+i1);
      i1++;
    }
    if (i1<count1)
    {
      while((i2<count2) && (*(tab2+i2)<=*(tab1+i1)))
      {
        *(temp+i++) = *(tab2+i2);
        i2++;
      }
    }
  }

  memcpy(temp+i,tab1+i1,(count1-i1)*sizeof(int));
  memcpy(tab1,temp,count1*sizeof(int));

  memcpy(temp+i,tab2+i2,(count2-i2)*sizeof(int));
  memcpy(tab2,temp+count1,count2*sizeof(int));
  // These two lines can be:
  // memcpy(tab2,temp+count1,i2*sizeof(int));
  free(temp);
  return;
}

// MergeSort a table of integer of size count.
// Never tested.
void MergeSort(int *tab,int count)
{
  if (count==1) return;

  MergeSort(tab,count/2);
  MergeSort(tab+count/2,(count+1)/2);
  Mix(tab,tab+count/2,count/2,(count+1)/2);
}
