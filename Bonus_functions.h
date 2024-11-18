#ifndef BONUS_FUNCTIONS_H
#define BONUS_FUNCTIONS_H

#include <stdio.h>   
#include <stdlib.h>
#include <string.h>  
#include <stdbool.h> 
#include <conio.h>
#include <windows.h> 
#include <unistd.h>
#include "TNOVS_functions.h"  
#include "TOVS_functions.h"  
#include "Display_functions.h"

// Function to transfer 50% of the records from the source file to the new file
void transfer50PercentRecords(File *sourceFile, File *newFile);

// Function to find the intersection of two files and store it in a result file
void findIntersection(File *file1, File *file2, File *resultFile);

// Function to process a file by reading its records, potentially adding them to a result
void processFile(File *file);

// Function to concatenate two files, avoiding duplicates, and store the result in a third file
void concatenateFilesWithoutDuplicates(File *file1, File *file2, File *resultFile);

// Function that provides a menu and handles bonus operations like transfer, intersection, and concatenation
void Bonus ();

#endif
