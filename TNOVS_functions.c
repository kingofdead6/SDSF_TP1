#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <conio.h>
#include <windows.h>
#include <unistd.h> 
#include "TNOVS_functions.h"
#include "Display_functions.h"


// Abstract Machine functions

// Allocates and initializes a new block for the file.
// Increments the number of blocks in the file header.
Block *AllocBlock(File *file) {
    Block *block = (Block*)malloc(sizeof(Block));
    if (!block) {
        fprintf(stderr, "Memory allocation failed.\n");
        return NULL;
    }
    // Initialize the block to default values.
    memset(block->data, 0, BLOCK_SIZE);
    block->Number_of_records = 0;
    block->Byte_Used = 0;
    file->header.Number_of_Blocks++; // Update header metadata.
    return block;
}

// Reads a block from the file based on its block number.
// Seeks to the correct position and reads the block into memory.
int readBlock(FILE *file, int blockNumber, Block *block) {
    fseek(file, sizeof(Header) + blockNumber * sizeof(Block), SEEK_SET);
    return fread(block, sizeof(Block), 1, file);
}

// Writes a block to the file at the specified block number.
// Ensures block data is saved back to the file.
void writeBlock(FILE *file, int blockNumber, Block *block) {
    fseek(file, sizeof(Header) + blockNumber * sizeof(Block), SEEK_SET);
    fwrite(block, sizeof(Block), 1, file);
}

// Updates the file header with new metadata.
void setHeader(FILE *file, Header *header) {
    fseek(file, 0, SEEK_SET);
    fwrite(header, sizeof(Header), 1, file);
}

// Retrieves the current header metadata from the file.
Header getHeader(FILE *file) {
    Header header;
    fseek(file, 0, SEEK_SET);
    fread(&header, sizeof(Header), 1, file);
    return header;
}

// Converts a Record structure into a delimited string format for storage.
void Record_to_String(Record rec, char *s) {
    sprintf(s, "%d,%s,%s,%s,%d%s",
            rec.key, rec.First_Name, rec.Last_Name, rec.Description,
            rec.Eraser ? 1 : 0, DELIMITER);
}

// Parses a delimited string into a Record structure.
void String_to_Record(const char *s, Record *rec) {
    sscanf(s, "%d,%20[^,],%20[^,],%100[^,],%d", 
           &rec->key, rec->First_Name, rec->Last_Name,
           rec->Description, (int*)&rec->Eraser);
}

// Opens a file and initializes the File structure.
// Creates a new file if it doesn't exist (in read mode).
File *Open(const char *filename, const char *mode) {
    FILE *file = fopen(filename, mode);
    if (!file && mode[0] == 'r') {
        // Create and initialize a new file if it doesn't exist.
        file = fopen(filename, "wb+");
        if (file) {
            Header header = {0, 0}; // Initialize header to default values.
            setHeader(file, &header);
            printCentered("File Created Successfully");
        }
        if(!file){
            printCentered("File was not created");
        }
    }

    // Allocate and initialize the File structure.
    File *FIle = (File*)malloc(sizeof(File));
    FIle->file = file;
    FIle->header = getHeader(file);
    printCentered("File is open");
    return FIle;
}

// Closes the file and ensures header metadata is updated.
void Close(File *file) {
    setHeader(file->file, &file->header); // Save updated header.
    if (file->file) fclose(file->file);  // Close file handle.
    free(file); // Free allocated memory for the File structure.
}

// TNOVS FUNCTIONS

// Inserts a new record into the file, ensuring no duplicates exist.
void insertRecord_TnOVS(File *file, Record rec) {
    char recordStr[BLOCK_SIZE * 2]; // Buffer for serialized record.
    Record_to_String(rec, recordStr); // Convert record to string.
    int recordLen = strlen(recordStr); // Length of the serialized record.

    if (recordLen > BLOCK_SIZE) {
        printf("Error: Record size exceeds block size.\n");
        return;
    }

    Block block;
    bool isDuplicate = false;

    // Check for duplicate records by scanning all blocks.
    for (int blockNumber = 0; blockNumber < file->header.Number_of_Blocks; blockNumber++) {
        readBlock(file->file, blockNumber, &block);

        char *token = strtok(block.data, DELIMITER);
        while (token != NULL) {
            Record existingRecord;
            String_to_Record(token, &existingRecord);

            // If a matching key is found, mark as duplicate.
            if (!existingRecord.Eraser && existingRecord.key == rec.key) {
                isDuplicate = true;
                break;
            }

            token = strtok(NULL, DELIMITER);
        }
        if (isDuplicate) break;
    }

    if (isDuplicate) {
        printf("Duplicate record with key %d found. Insertion skipped.\n", rec.key);
        return;
    }

    // Find the block to insert the record, or create a new one.
    int blockNumber = file->header.Number_of_Blocks > 0 ? file->header.Number_of_Blocks - 1 : 0;
    readBlock(file->file, blockNumber, &block);

    // If the block is full, prepare a new block for the record.
    if (block.Byte_Used >= BLOCK_SIZE || file->header.Number_of_Blocks == 0) {
        blockNumber = file->header.Number_of_Blocks;
        memset(&block, 0, sizeof(Block));
    }

    int remainingBytes = recordLen;
    char *recordPointer = recordStr;

    // Insert the record across blocks if necessary.
    while (remainingBytes > 0) {
        int availableSpace = BLOCK_SIZE - block.Byte_Used;

        if (remainingBytes > availableSpace) {
            // Fill the current block to its capacity.
            strncat(block.data, recordPointer, availableSpace);
            block.Byte_Used += availableSpace;
            block.Number_of_records++;
            writeBlock(file->file, blockNumber++, &block);

            // Move to the next block and reset it.
            memset(&block, 0, sizeof(Block));
            recordPointer += availableSpace;
            remainingBytes -= availableSpace;
        } else {
            // Insert the remaining part of the record.
            strcat(block.data, recordPointer);
            block.Byte_Used += remainingBytes;
            block.Number_of_records++;
            remainingBytes = 0;
        }
    }

    writeBlock(file->file, blockNumber, &block);

    // Update header metadata.
    file->header.Number_of_Blocks = blockNumber + 1;
    file->header.Number_of_Records++;
    setHeader(file->file, &file->header);
}


// Function to perform initial loading of records into the file
void initialLoad_TnOVS(File *file, int rate) {
    // Loop to create and insert 'rate' number of records
    for (int i = 1; i <= rate; i++) {
        // Create a new record and assign values to its fields
        Record rec;
        rec.key = i;
        snprintf(rec.First_Name, sizeof(rec.First_Name), "First%d", i);
        snprintf(rec.Last_Name, sizeof(rec.Last_Name), "Last%d", i);
        snprintf(rec.Description, sizeof(rec.Description), "Record number %d", i);
        rec.Eraser = false; // Mark the record as not deleted

        // Insert the record into the file using the insertRecord_TnOVS function
        insertRecord_TnOVS(file, rec);
    }
    // Print message indicating the number of records inserted
    printf("Initial load is completed with %d records.\n", rate);
}

// Function to display the header of the file, showing block and record counts
void Display_Header_TnOVS(File *file) {
    // Print the number of blocks and records from the header
    printf("The number of blocks here is  : %d \n", file->header.Number_of_Blocks);
    printf("The number of records here is  : %d \n", file->header.Number_of_Records);
}

// Function to display the entire content of the file, block by block
void display_File_TnOVS(File *file) {
    // First, display the header information
    Display_Header_TnOVS(file);
    Block block;

    // Loop through each block in the file
    for (int i = 0; i < file->header.Number_of_Blocks; i++) {
        // Read the block data from the file
        readBlock(file->file, i, &block);
        // Tokenize the block data based on the delimiter
        char *token = strtok(block.data, DELIMITER);
        
        printf("Block %d:\n", i);

        // Loop through the records in the block and print them if not erased
        while (token != NULL) {
            Record existingRecord;
            String_to_Record(token, &existingRecord);
            if (!existingRecord.Eraser) { // Check if record is not erased
                printf("  %s\n", token); // Print the record
            }
            // Move to the next token (record)
            token = strtok(NULL, DELIMITER);
        }
    }
}

// Function to display the contents of a specific block in the file
void Display_block_TnOVS(File *file, int num_of_block) {
    Block block;
    // Read the specified block from the file
    readBlock(file->file, num_of_block, &block);
    // Tokenize the block data
    char *token = strtok(block.data, DELIMITER);
    
    printf("Contents of Block %d:\n", num_of_block);

    // Loop through the records in the block and print them if not erased
    while (token != NULL) {
        Record existingRecord;
        String_to_Record(token, &existingRecord);    
        if (!existingRecord.Eraser) { // Check if record is not erased
            printf("  %s\n", token); // Print the record
        }
        // Move to the next token (record)
        token = strtok(NULL, DELIMITER);
    }
}

// Function to display any overlapping records in the file (records that span multiple blocks)
void display_Overlapping_TnOVS(File *file) {
    Block block;
    bool foundOverlap = false;

    // Loop through each block, except the last one, to check for overlapping records
    for (int blockNumber = 0; blockNumber < file->header.Number_of_Blocks - 1; blockNumber++) {
        // Read the block data
        readBlock(file->file, blockNumber, &block);

        // Check if the block has data and the last byte is not the end of a record
        if (block.Byte_Used > 0 && block.data[block.Byte_Used - 1] != '\0') {
            foundOverlap = true;

            // Find the last delimiter in the block, indicating the end of the last record
            char *lastDelimiter = strrchr(block.data, DELIMITER[0]);
            if (lastDelimiter != NULL && (lastDelimiter - block.data) < BLOCK_SIZE - 1) {
                // Extract the partial record that overlaps into the next block
                char partialRecordStr[BLOCK_SIZE + 1] = {0};
                strncpy(partialRecordStr, lastDelimiter + 1, BLOCK_SIZE - (lastDelimiter - block.data + 1));
                partialRecordStr[BLOCK_SIZE] = '\0'; // Null-terminate the partial record string

                // Convert the partial record string into a Record structure
                Record overlappingRecord;
                String_to_Record(partialRecordStr, &overlappingRecord);

                // Print the overlapping record information
                printf("Overlapping Record Detected: Key %d starts in Block %d and continues to Block %d.\n",
                       overlappingRecord.key, blockNumber, blockNumber + 1);
            } else {
                // Error if the delimiter could not be found or the record is not valid
                printf("Error: Could not find a valid starting point for the overlapping record.\n");
            }
        }
    }

    // If no overlapping records were found, print a message
    if (!foundOverlap) {
        printf("No overlapping records across blocks found in the entire file.\n");
    }
}

// Function to search for a record by its key and display its position if found
void search_TnOVS(File *file, int key) {
    Block block;
    bool found = 0;
    
    // Loop through each block to search for the record
    for (int i = 0; i < file->header.Number_of_Blocks; i++) {
        // Read the block data
        readBlock(file->file, i, &block);
        // Tokenize the block data
        char *token = strtok(block.data, DELIMITER);

        int pos = 0;
        // Loop through the records in the block and check for a match by key
        while (token != NULL) {
            Record existingRecord;
            String_to_Record(token, &existingRecord);

            if (!existingRecord.Eraser && existingRecord.key == key) {
                found = 1;
                // Print the record's block number and position
                printf("Record with key %d was found in Block %d at Position %d\n", key, i, pos + 1);
            }

            // Move to the next token (record)
            token = strtok(NULL, DELIMITER);
            pos++;
        }
    }

    // If the record wasn't found, print a message
    if (!found) {
        printf("Record with key %d was not found.\n", key);
    }
}
void removeDuplicates_TnOVS(File *file) {
    Block block;
    int totalRecordsLength = 0;

    // Calculate the total buffer size needed for all blocks in the file
    int totalBufferSize = BLOCK_SIZE * file->header.Number_of_Blocks;
    char *allRecords = (char *)malloc(totalBufferSize * sizeof(char));
    if (!allRecords) {
        printf("Memory allocation failed for allRecords.\n");
        return;
    }
    memset(allRecords, 0, totalBufferSize);  // Initialize memory with zeros

    // Boolean array to track keys that have already been seen
    bool *seenKeys = (bool *)malloc(sizeof(bool) * 1000);  
    memset(seenKeys, 0, sizeof(bool) * 1000);  // Initialize to false (not seen)

    int allRecordsLength = 0;
    // Iterate over all blocks in the file
    for (int i = 0; i < file->header.Number_of_Blocks; i++) {
        readBlock(file->file, i, &block);  // Read each block
        char *token = strtok(block.data, DELIMITER);  // Tokenize the block data

        // Iterate over all records within the block
        while (token != NULL) {
            Record existingRecord;
            String_to_Record(token, &existingRecord);  // Convert the token to a Record

            // Skip the record if the key has already been seen (duplicate)
            if (seenKeys[existingRecord.key]) {
                token = strtok(NULL, DELIMITER);  // Move to next token
                continue;
            }

            // Mark the key as seen
            seenKeys[existingRecord.key] = true;

            // Convert the record back to a string and append it to allRecords
            char recordStr[BLOCK_SIZE];
            Record_to_String(existingRecord, recordStr);
            if (allRecordsLength > 0) {
                strcat(allRecords, DELIMITER);  // Add delimiter between records
                allRecordsLength += strlen(DELIMITER);
            }
            strcat(allRecords, recordStr);  // Append the record string
            allRecordsLength += strlen(recordStr);

            token = strtok(NULL, DELIMITER);  // Move to next token
        }
    }

    Block newBlock;
    memset(&newBlock, 0, sizeof(Block));  // Initialize a new block to store unique records
    char *recordPointer = allRecords;  // Pointer to the string of all records
    int remainingBytes = allRecordsLength;  // Remaining bytes to write to blocks
    int blockNumber = 0;

    // Process records and fit them into blocks
    while (remainingBytes > 0) {
        int availableSpace = BLOCK_SIZE - newBlock.Byte_Used;

        // If there isn't enough space for the remaining bytes in the current block
        if (remainingBytes > availableSpace) {
            strncat(newBlock.data, recordPointer, availableSpace);  // Copy part of the record into the block
            newBlock.Byte_Used += availableSpace;  // Update space used in the block
            newBlock.Number_of_records++;  // Increment record count for the block
            writeBlock(file->file, blockNumber++, &newBlock);  // Write the block to file

            memset(&newBlock, 0, sizeof(Block));  // Reset the block for the next round
            recordPointer += availableSpace;  // Move the pointer to the next part of the record
            remainingBytes -= availableSpace;  // Decrease the remaining bytes to write
        } else {
            strcat(newBlock.data, recordPointer);  // Copy the remaining records to the block
            newBlock.Byte_Used += remainingBytes;  // Update the block's used space
            newBlock.Number_of_records++;  // Increment the number of records in this block
            remainingBytes = 0;  // All records have been written
        }
    }

    // Write the last block if there are any remaining bytes
    if (newBlock.Byte_Used > 0) {
        writeBlock(file->file, blockNumber, &newBlock);
    }

    // Update the file header with the new block count
    file->header.Number_of_Blocks = blockNumber + 1;
    setHeader(file->file, &file->header);

    free(allRecords);  // Free the allocated memory for allRecords
    free(seenKeys);  // Free the memory for seenKeys
}


void logicalDelete_TnOVS(File *file, int key) {
    Block block;
    bool found = false;

    // Calculate the maximum size required for storing all records
    int maxSize = BLOCK_SIZE * file->header.Number_of_Blocks;

    // Allocate memory to store all records from the file
    char *allRecords = (char *)malloc(maxSize * sizeof(char));
    if (allRecords == NULL) {
        printf("Memory allocation failed.\n");
        return;
    }
    
    memset(allRecords, 0, maxSize);  // Initialize memory to zero
    int totalRecordsLength = 0;

    // Iterate over all blocks in the file
    for (int i = 0; i < file->header.Number_of_Blocks; i++) {
        readBlock(file->file, i, &block);  // Read each block
        char *token = strtok(block.data, DELIMITER);  // Tokenize the block data
        
        // Iterate over all records within the block
        while (token != NULL) {
            Record existingRecord;
            String_to_Record(token, &existingRecord);  // Convert the token to a Record

            // If the record matches the key and is not already erased, mark it for logical deletion
            if (existingRecord.key == key && !existingRecord.Eraser) {
                existingRecord.Eraser = true;
                found = true;
            }

            // If the record is not erased, append it to allRecords
            if (!existingRecord.Eraser) {
                char recordStr[BLOCK_SIZE];
                Record_to_String(existingRecord, recordStr);
                
                if (totalRecordsLength > 0) {
                    strcat(allRecords, DELIMITER);  // Add delimiter between records
                    totalRecordsLength += strlen(DELIMITER);
                }
                strcat(allRecords, recordStr);  // Append the record string
                totalRecordsLength += strlen(recordStr);
            }

            token = strtok(NULL, DELIMITER);  // Move to the next token
        }
    }

    // If the record wasn't found for logical deletion, exit early
    if (!found) {
        printf("Record with key %d not found for logical deletion.\n", key);
        free(allRecords);  
        return;
    }

    // Write the modified records back to blocks
    int blockNumber = 0;
    memset(&block, 0, sizeof(Block));  // Initialize a new block

    char *recordPointer = allRecords;  // Pointer to all records
    int remainingBytes = totalRecordsLength;  // Remaining bytes to write

    // Write the records to blocks, splitting them if necessary
    while (remainingBytes > 0) {
        int availableSpace = BLOCK_SIZE - block.Byte_Used;

        // If the current block doesn't have enough space for the remaining bytes
        if (remainingBytes > availableSpace) {
            strncat(block.data, recordPointer, availableSpace);  // Copy the part of the record into the block
            block.Byte_Used += availableSpace;  // Update block usage
            block.Number_of_records++;  // Increment the number of records in the block
            writeBlock(file->file, blockNumber++, &block);  // Write the current block to the file

            memset(&block, 0, sizeof(Block));  // Reset the block for the next set of records
            recordPointer += availableSpace;  // Move to the next part of the records
            remainingBytes -= availableSpace;  // Update remaining bytes to write
        } else {
            strcat(block.data, recordPointer);  // Copy the rest of the records into the block
            block.Byte_Used += remainingBytes;  // Update block usage
            block.Number_of_records++;  // Increment the number of records
            remainingBytes = 0;  // All records have been written
        }
    }

    // Write the last block if necessary
    if (block.Byte_Used > 0) {
        writeBlock(file->file, blockNumber, &block);
    }

    // Update the file's block count and header
    file->header.Number_of_Blocks = blockNumber + 1;
    file->header.Number_of_Records -= 1;  // Decrease the total record count by 1
    setHeader(file->file, &file->header);

    printf("Record with key %d logically deleted and blocks re-inserted.\n", key);

    free(allRecords);  // Free allocated memory
}


void physicalDelete_TnOVS(File *file, int key) {
    Block block;  // Declare a block to hold data read from the file
    bool found = false;  // Flag to track whether the record is found
    int totalRecordsLength = 0;  // Variable to track the total length of all records combined
    
    // Calculate the total size required to hold all records from all blocks
    int totalBufferSize = BLOCK_SIZE * file->header.Number_of_Blocks;
    // Allocate memory to store all records from the file
    char *allRecords = (char *)malloc(totalBufferSize * sizeof(char));
    
    // Check if memory allocation was successful
    if (!allRecords) {
        printf("Memory allocation failed for allRecords.\n");
        return;
    }
    
    // Initialize the memory to 0
    memset(allRecords, 0, totalBufferSize);

    // Loop through all blocks in the file
    for (int i = 0; i < file->header.Number_of_Blocks; i++) {
        // Read the current block
        readBlock(file->file, i, &block);
        // Tokenize the block's data using the delimiter
        char *token = strtok(block.data, DELIMITER);

        // Loop through all records in the current block
        while (token != NULL) {
            Record existingRecord;  // Temporary variable to store the record
            // Convert the token to a Record structure
            String_to_Record(token, &existingRecord);

            // If the current record's key matches the key to be deleted, skip it
            if (existingRecord.key == key) {
                found = true;  // Set the flag to true as the record is found
                token = strtok(NULL, DELIMITER);  // Move to the next token
                continue;  // Skip adding this record to the allRecords buffer
            }

            // Convert the current record to a string representation
            char recordStr[BLOCK_SIZE];
            Record_to_String(existingRecord, recordStr);

            // If this is not the first record, add the delimiter between records
            if (totalRecordsLength > 0) {
                strcat(allRecords, DELIMITER);
                totalRecordsLength += strlen(DELIMITER);
            }
            // Add the current record string to the allRecords buffer
            strcat(allRecords, recordStr);
            totalRecordsLength += strlen(recordStr);  // Update the total length of records

            // Move to the next token (next record)
            token = strtok(NULL, DELIMITER);
        }
    }

    // If the record was found, proceed to delete it
    if (found) {
        Block newBlock;  // Block to hold the updated data after deletion
        memset(&newBlock, 0, sizeof(Block));  // Initialize the new block

        // Pointer to traverse through allRecords buffer
        char *recordPointer = allRecords;
        int remainingBytes = totalRecordsLength;  // Number of remaining bytes to be written
        int blockNumber = 0;  // Block number to write data to
        int lastBlockByteUsed = 0;  // Byte usage in the last block

        // Loop to write records back to the blocks
        while (remainingBytes > 0) {
            int availableSpace = BLOCK_SIZE - newBlock.Byte_Used;  // Calculate available space in the block

            // If the current block cannot hold all the remaining records, fill it partially
            if (remainingBytes > availableSpace) {
                strncat(newBlock.data, recordPointer, availableSpace);  // Copy part of the data to the block
                newBlock.Byte_Used += availableSpace;  // Update the byte usage in the block
                newBlock.Number_of_records++;  // Update the record count in the block
                // Write the block to the file
                writeBlock(file->file, blockNumber++, &newBlock);

                // Reset the block for the next set of records
                memset(&newBlock, 0, sizeof(Block));
                // Move the record pointer forward and decrease the remaining bytes
                recordPointer += availableSpace;
                remainingBytes -= availableSpace;
            } else {
                // If the remaining records fit into the current block, write them all
                strcat(newBlock.data, recordPointer);
                newBlock.Byte_Used += remainingBytes;  // Update byte usage
                newBlock.Number_of_records++;  // Update record count
                remainingBytes = 0;  // All remaining bytes have been written
            }
        }

        // If there are remaining records in the last block, write it
        if (newBlock.Byte_Used > 0) {
            writeBlock(file->file, blockNumber, &newBlock);
        }

        // Update the file header to reflect the new number of blocks and records
        file->header.Number_of_Blocks = blockNumber + 1;  // Increment the block count
        file->header.Number_of_Records--;  // Decrease the record count as one was deleted
        // Update the file header on disk
        setHeader(file->file, &file->header);

        // Inform the user that the record has been deleted
        printf("Record with key %d successfully deleted.\n", key);
    } else {
        // If the record wasn't found, inform the user
        printf("Record with key %d not found.\n", key);
    }

    // Free the memory allocated for allRecords
    free(allRecords);
}


// Main TNOVS function that provides a menu-driven interface to interact with the file system
void TnOVS() {
    int key;        // Variable to hold key input from the user
    int choice = 0; // Variable to track the selected menu option
    char filename[24];  // String to hold the filename for the TNOVS file
    
    Before_tnovs(filename);  // Initialize the filename for TNOVS file (e.g., append .tnovs extension)

    strcat(filename, ".tnovs");  // Add the .tnovs extension to the filename

    // Open the file for reading and writing, if it exists, otherwise create it
    File *tnovsFile = Open(filename, "rb+"); 
    sleep(3);  // Pause for 3 seconds before the main loop

    // Main infinite loop for the menu system
    while (1) {
        display_TnOVS_Menu(choice);  // Display the menu with the current choice highlighted
        int reckey ,reckey2,reckey3 ,Blocknum;
        key = getch();  // Get a key press from the user

        if (key == 224) {  // Handle special keys (arrows for navigation)
            key = getch();  // Get the second byte of the key press (special keys are two-byte sequences)
            switch (key) {
                case 72:  // Up arrow key
                    choice = (choice - 1 + 11) % 11;  // Move selection up, wrapping around to the last option
                    break;
                case 80:  // Down arrow key
                    choice = (choice + 1) % 11;  // Move selection down, wrapping around to the first option
                    break;
            }
        } else if (key == 13) {  // Enter key (to select the highlighted option)
            switch (choice) {
                case 0:  // Initial load of records into the file
                    system("cls");  // Clear the screen
                    int rate;  // Variable to hold the number of records to load
                    printCentered("Enter the number of records that you want to insert as an initial load : \n");
                    printf("\033[1A\033[63C");  // Adjust the cursor position
                    scanf("%d", &rate);  // Input the number of records
                    initialLoad_TnOVS(tnovsFile, rate);  // Load the records into the file
                    break;
                case 1:  // Insert a new record into the file
                    system("cls");  // Clear the screen
                    Record rec;  // Declare a record to store the input data
                    printCentered("Enter the informations of the record that you want to insert : \n");
                    printCentered("key : ");
                    printf("\033[1A\033[65C");
                    scanf("%d", &rec.key);  // Input the record key
                    printCentered("  First Name : ");
                    printf("\033[0A\033[66C");
                    scanf("%20s", &rec.First_Name);  // Input the first name
                    printCentered("  Last Name : ");
                    printf("\033[0A\033[66C");
                    scanf("%20s", &rec.Last_Name);  // Input the last name
                    printCentered("  Description : ");
                    printf("\033[0A\033[68C");
                    scanf("%100s", &rec.Description);  // Input the description
                    rec.Eraser = false;  // Set the Eraser flag to false, indicating that the record is not deleted
                    insertRecord_TnOVS(tnovsFile, rec);  // Insert the record into the file
                    break;
                case 2:  // Logically delete a record from the file
                    
                    system("cls");  // Clear the screen
                    printCentered("Enter the key of the record that you want to logically delete : \n");
                    printf("\033[1A\033[63C");
                    scanf("%d", &reckey);  // Input the record key to logically delete
                    logicalDelete_TnOVS(tnovsFile, reckey);  // Perform logical deletion
                    removeDuplicates_TnOVS(tnovsFile);  // Remove duplicates after deletion
                    break;
                case 3:  // Physically delete a record from the file
                    system("cls");  // Clear the screen
                    printCentered("Enter the key of the record that you want to physically delete : \n");
                    printf("\033[1A\033[63C");
                    scanf("%d", &reckey2);  // Input the record key to physically delete
                    physicalDelete_TnOVS(tnovsFile, reckey2);  // Perform physical deletion
                    removeDuplicates_TnOVS(tnovsFile);  // Remove duplicates after deletion
                    break;
                case 4:  // Search for a record by key
                    system("cls");  // Clear the screen
                    printCentered("Enter the key of the record that you want to Search about : \n");
                    printf("\033[1A\033[63C");
                    scanf("%d", &reckey3);  // Input the record key to search for
                    search_TnOVS(tnovsFile, reckey3);  // Perform the search operation
                    break;
                case 5:  // Display the header information of the file
                    system("cls");  // Clear the screen
                    printCentered("These are the informations of your header : \n");
                    Display_Header_TnOVS(tnovsFile);  // Display header information
                    break;
                case 6:  // Display the contents of a specific block
                    system("cls");  // Clear the screen
                    printCentered("Enter the Number of the block that you want to display its content : \n");
                    scanf("%d", &Blocknum);  // Input the block number to display
                    Display_block_TnOVS(tnovsFile, Blocknum);  // Display the block content
                    break;
                case 7:  // Display overlapping records
                    system("cls");  // Clear the screen
                    display_Overlapping_TnOVS(tnovsFile);  // Display overlapping records (if any)
                    break;
                case 8:  // Display the entire file content
                    system("cls");  // Clear the screen
                    display_File_TnOVS(tnovsFile);  // Display the entire file content
                    break;
                case 9:  // Exit the program
                    return;
                case 10:  // Exit the program with exit status 0
                    exit(0);
                    break;
                default:  // Handle invalid options
                    printf("Invalid option\n");
                    break;
            }
            printf("Press any key to continue...\n");
            getch();  // Wait for the user to press a key before returning to the menu
        }
    }
}
