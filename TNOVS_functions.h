#ifndef TNOVS_FUNCTIONS_H
#define TNOVS_FUNCTIONS_H

// Standard library includes
#include <stdio.h>      
#include <stdlib.h>     
#include <time.h>  
#include <stdbool.h>
#include <windows.h>   
#include <unistd.h>  
#include <conio.h>      
#include "Display_functions.h" 

// Constants
#define BLOCK_SIZE 256  // Defines the size of a block in bytes
#define DELIMITER "|"   // Delimiter used to separate fields in serialized record strings

// Record structure: Represents a single logical record in the file.
typedef struct {
    int key;                     // Unique identifier for the record
    char First_Name[20];         // First name of the person or entity
    char Last_Name[20];          // Last name of the person or entity
    char Description[100];       // Additional description or details
    bool Eraser;                 // Logical deletion marker (true if deleted, false otherwise)
} Record;

// Block structure: Represents a single block of storage in the file.
typedef struct {
    char data[BLOCK_SIZE];       // Array to store raw block data
    int Byte_Used;               // Number of bytes used in the block
    int Number_of_records;       // Number of records currently stored in the block
} Block;

// Header structure: Metadata for managing the file as a whole.
typedef struct {
    int Number_of_Blocks;        // Total number of blocks in the file
    int Number_of_Records;       // Total number of records in the file
} Header;

// File structure: Encapsulates the file pointer and its associated metadata.
typedef struct {
    FILE *file;                  // Pointer to the open file
    Header header;               // Header metadata associated with the file
} File;

// Abstract machine functions: Provide core functionality for file and block management.

// Allocates a new block for storing data and associates it with the file.
Block *AllocBlock(File *file);

// Reads the contents of a specific block from the file into memory.
int readBlock(FILE *file, int blockNumber, Block *block);

// Writes the contents of a block from memory back into the file.
void writeBlock(FILE *file, int blockNumber, Block *block);

// Sets (updates) the file header in the file.
void setHeader(FILE *file, Header *header);

// Retrieves the file header metadata from the file.
Header getHeader(FILE *file);

// Converts a Record structure into a serialized string format for storage.
void Record_to_String(Record rec, char *s);

// Converts a serialized string back into a Record structure.
void String_to_Record(const char *s, Record *rec);

// Opens a file with the specified name and mode and initializes the File structure.
File *Open(const char *filename, const char *mode);

// Closes an open file and performs any necessary cleanup.
void Close(File *file);

// TnOVS-specific functions: Implement the logic for managing TnOVS records and blocks.

// Inserts a new record into the file using the TnOVS storage mechanism.
void insertRecord_TnOVS(File *file, Record rec);

// Performs an initial load of the file with randomly generated records (for testing purposes).
void initialLoad_TnOVS(File *file, int rate);

// Displays the header metadata for a TnOVS file.
void Display_Header_TnOVS(File *file);

// Displays all records in the file, including their details and storage structure.
void display_File_TnOVS(File *file);

// Displays the contents of a specific block in the file.
void Display_block_TnOVS(File *file, int num_of_block);

// Displays overlapping records (if any) in the TnOVS file.
void display_Overlapping_TnOVS(File *file);

// Searches for a record in the file by its key and displays the result.
void search_TnOVS(File *file, int key);

// Removes duplicate records from the file.
void removeDuplicates_TnOVS(File *file);

// Marks a record as logically deleted (sets Eraser = true).
void logicalDelete_TnOVS(File *file, int key);

// Permanently removes a record from the file (physical deletion).
void physicalDelete_TnOVS(File *file, int key);

// Main function to execute TnOVS-specific operations and testing.
void TnOVS();

#endif
