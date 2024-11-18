#ifndef TOVS_FUNCTIONS_H
#define TOVS_FUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <windows.h>
#include <unistd.h> 
#include <conio.h>
#include "Display_functions.h"
#include "TNOVS_functions.h"

// TOVS FUNCTION DECLARATIONS

// Function to count the number of records in a block of data
// The function assumes that the data is stored as a string with records separated by a delimiter.
int countRecordsInBlock(const char *data);

// Function to insert a new record into the TOVS file
// Takes a pointer to the file and the record to be inserted as arguments.
// The record will be added to the appropriate block in the file.
void insertRecord_TOVS(File *file, Record rec);

// Function to initially load records into the TOVS file
// Takes a pointer to the file, a minimum index (min), and a rate (ending index).
// This function loads records within the specified range (min to rate) into the file.
void initialLoad_TOVS(File *file, int min, int rate);

// Function to display the header of the TOVS file
// It reads and outputs the header information, such as the number of blocks and records in the file.
void Display_Header_TOVS(File *file);

// Function to display all records stored in the TOVS file
// Iterates through all blocks in the file and outputs the data for each record.
void display_File_TOVS(File *file);

// Function to display a specific block of records from the TOVS file
// Takes the file pointer and the block number as arguments and displays the content of the specified block.
void Display_block_TOVS(File *file, int num_of_block);

// Function to display overlapping records in the TOVS file
// It identifies and displays records that overlap or share keys in the file.
void display_Overlapping_TOVS(File *file);

// Function to search for a record by key in the TOVS file
// Takes the file pointer and the key to search for. If a matching record is found, it displays the record.
void search_TOVS(File *file, int key);

// Function to remove duplicate records in the TOVS file
// It checks for duplicate records (based on the key) and removes them, ensuring that each key appears only once in the file.
void removeDuplicates_TOVS(File *file);

// Function to logically delete a record by key from the TOVS file
// The record is marked as deleted without actually removing it from the file, making it unavailable for further use.
void logicalDelete_TOVS(File *file, int key);

// Function to physically delete a record by key from the TOVS file
// This function completely removes the record from the file and re-adjusts the file's structure accordingly.
void physicalDelete_TOVS(File *file, int key);

// Main function to handle the menu and user interactions for the TOVS file operations
// Displays a menu to the user for performing various operations like inserting, deleting, and searching for records.
// The loop continues until the user decides to exit.
void TOVS();

#endif
