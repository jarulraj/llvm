  void heapsort(int arr[], unsigned int N)
  {
      unsigned int n = N, i = n/2, parent, child;
      int t;
  
      for (;;) { /* Loops until arr is sorted */
          if (i > 0) { /* First stage - Sorting the heap */
              i--;           /* Save its index to i */
              t = arr[i];    /* Save parent value to t */
          } else {     /* Second stage - Extracting elements in-place */
              n--;           /* Make the new heap smaller */
              if (n == 0) return; /* When the heap is empty, we are done */
              t = arr[n];    /* Save last value (it will be overwritten) */
              arr[n] = arr[0]; /* Save largest value at the end of arr */
          }
  
          parent = i; /* We will start pushing down t from parent */
          child = i*2 + 1; /* parent's left child */
  
          /* Sift operation - pushing the value of t down the heap */
          while (child < n) {
              if (child + 1 < n  &&  arr[child + 1] > arr[child]) {
                  child++; /* Choose the largest child */
              }
              if (arr[child] > t) { /* If any child is bigger than the parent */
                  arr[parent] = arr[child]; /* Move the largest child up */
                  parent = child; /* Move parent pointer to this child */
                  //child = parent*2-1; /* Find the next child */
                  child = parent*2+1; /* the previous line is wrong*/
              } else {
                  break; /* t's place is found */
              }
          }
          arr[parent] = t; /* We save t in the heap */
      }
  }
