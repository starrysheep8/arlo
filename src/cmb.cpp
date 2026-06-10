#include "Novae.h"

namespace cmb {
  bool arrayContains(int element, int array[], int size) {
    for (int i = 0; i < size; i++) {
        if (array[i] == element) return true;
    }
    return false;
  }

  int getMode(int intArray[], int size) {
    //get list of unique elements in intArray
    int uniqueElementsFound = 0;
    int uniqueElements[size];
    for (int i = 0; i < size; i++) {
        if (!arrayContains(intArray[i], uniqueElements, uniqueElementsFound)) {
            uniqueElements[uniqueElementsFound] = intArray[i];
            uniqueElementsFound++;
        }
    }

    //count occurances of each unique element in intArray
    int foundCounts[uniqueElementsFound];
    for (int i = 0; i < uniqueElementsFound; i++) {foundCounts[i] = 0;}

    for (int i = 0; i < uniqueElementsFound; i++) {
        int currentElement = uniqueElements[i];
        for (int j = 0; j < size; j++) {
            if (intArray[j] == currentElement) {
                foundCounts[i]++;
            }
        }
    }

    //find and return the element that occurred the most
    int biggestIndex = 0;
    int biggestCount = 0;
    for (int i = 0; i < uniqueElementsFound; i++) {
        if (foundCounts[i] > biggestCount) {
            biggestCount = foundCounts[i];
            biggestIndex = i;
        }
    }
    
    return uniqueElements[biggestIndex];
  }

}