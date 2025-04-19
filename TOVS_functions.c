#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <conio.h>
#include <windows.h>
#include <unistd.h> 
#include "TNOVS_functions.h"
#include "Display_functions.h"




//                          TOVS FUNCTIONS : 
// Function to count the number of records in a block
int countRecordsInBlock(const char *data) {
    int count = 0; // Initialize the record count
    char tempData[BLOCK_SIZE];
    strcpy(tempData, data); // Copy block data into a temporary buffer

    // Tokenize the block's data using the delimiter
    char *token = strtok(tempData, DELIMITER);
    while (token != NULL) {
        count++; // Increment the count for each token (record)
        token = strtok(NULL, DELIMITER); // Move to the next token
    }
    return count; // Return the total count of records in the block
}

// Function to insert a new record into the TOVS file structure
void insertRecord_TOVS(File *file, Record rec) {
    // Convert the record into a string representation
    char recordStr[BLOCK_SIZE * 2];
    Record_to_String(rec, recordStr);
    int recordLen = strlen(recordStr); // Get the length of the record string

    // Check if the record's length exceeds the block size
    if (recordLen > BLOCK_SIZE) {
        printf("Error: Record size exceeds block size.\n");
        return; // If the record is too large, return without inserting
    }

    Block block;
    bool isDuplicate = false; // Flag to check if a duplicate record exists
    bool inserted = false; // Flag to check if the record was inserted
    int blockNumberToInsert = -1; // Variable to store the block number for insertion
    int positionToInsert = -1; // Variable to store the position for insertion within the block

    // Iterate over all blocks to find a suitable place for the new record
    for (int blockNumber = 0; blockNumber < file->header.Number_of_Blocks; blockNumber++) {
        readBlock(file->file, blockNumber, &block); // Read the block from the file

        char tempBlockData[BLOCK_SIZE];
        strcpy(tempBlockData, block.data); // Copy the block data into a temporary buffer

        char *token = strtok(tempBlockData, DELIMITER);
        int position = 0; // Position counter within the block data

        // Iterate through all records in the block
        while (token != NULL) {
            Record existingRecord;
            // Parse the token into an existing record
            sscanf(token, "%d,%20[^,],%20[^,],%100[^,],%d",
                   &existingRecord.key, existingRecord.First_Name,
                   existingRecord.Last_Name, existingRecord.Description,
                   (int *)&existingRecord.Eraser);

            // If the record is not erased, check if it's a duplicate or if it's a suitable spot for insertion
            if (!existingRecord.Eraser) {
                // If the current record has the same key, it's a duplicate
                if (existingRecord.key == rec.key) {
                    isDuplicate = true;
                    break; // Exit the loop since we found a duplicate
                }

                // If the current record's key is greater and we haven't inserted yet, mark this position for insertion
                if (existingRecord.key > rec.key && !inserted) {
                    blockNumberToInsert = blockNumber;
                    positionToInsert = position;
                    inserted = true;
                }
            }

            // Move to the next token (next record in the block)
            position += strlen(token) + strlen(DELIMITER);
            token = strtok(NULL, DELIMITER);
        }

        // If a duplicate is found, stop the search
        if (isDuplicate) break;
    }

    // If a duplicate is found, print a message and stop the insertion
    if (isDuplicate) {
        printf("Duplicate record with key %d found. Insertion skipped.\n", rec.key);
        return;
    }

    char overflowData[BLOCK_SIZE * 2] = {0}; // Buffer to hold overflow data if the record doesn't fit in the current block

    // If the record is inserted in the middle of a block
    if (inserted) {
        // Read the selected block and prepare for insertion
        readBlock(file->file, blockNumberToInsert, &block);

        char newData[BLOCK_SIZE * 2];
        memset(newData, 0, sizeof(newData)); // Clear the new data buffer

        // Copy data before the insertion point
        strncpy(newData, block.data, positionToInsert);
        // Append the new record string and delimiter
        strcat(newData, recordStr);
        strcat(newData, DELIMITER);
        // Append the remaining data from the block
        strcat(newData, block.data + positionToInsert);

        // If the new data exceeds the block size, split it into overflow data
        if (strlen(newData) > BLOCK_SIZE) {
            strncpy(overflowData, newData + BLOCK_SIZE, strlen(newData) - BLOCK_SIZE);
            overflowData[strlen(newData) - BLOCK_SIZE] = '\0';
            newData[BLOCK_SIZE] = '\0'; // Truncate the new data to fit within the block
        }

        // Update the block data with the new content
        strncpy(block.data, newData, BLOCK_SIZE);
        block.Byte_Used = strlen(block.data); // Update the number of bytes used in the block
        block.Number_of_records = countRecordsInBlock(block.data); // Update the number of records in the block
        writeBlock(file->file, blockNumberToInsert, &block); // Write the updated block back to the file

    } else {
        // If no insertion point was found, insert the record into the last block or create a new block if necessary
        int blockNumber = file->header.Number_of_Blocks > 0 ? file->header.Number_of_Blocks - 1 : 0;
        readBlock(file->file, blockNumber, &block);

        // If the block is full or no blocks exist, create a new block
        if (block.Byte_Used >= BLOCK_SIZE || file->header.Number_of_Blocks == 0) {
            blockNumber = file->header.Number_of_Blocks;
            memset(&block, 0, sizeof(Block)); // Initialize a new block
            file->header.Number_of_Blocks++; // Increment the number of blocks in the file
        }

        // Append the new record and delimiter to the block data
        strcat(block.data, recordStr);
        strcat(block.data, DELIMITER);
        block.Byte_Used += recordLen + strlen(DELIMITER); // Update the bytes used in the block
        block.Number_of_records++; // Increment the number of records in the block
        writeBlock(file->file, blockNumber, &block); // Write the updated block back to the file
    }

    bool finalWrite = false; // Flag to indicate whether the final overflow data is written
    int remainingBytes = strlen(overflowData); // Remaining overflow data size
    char *recordPointer = overflowData; // Pointer to the overflow data

    int currentBlock = blockNumberToInsert + 1; // Start from the next block after insertion
    while (strlen(overflowData) > 0 || !finalWrite) {
        Block nextBlock;

        // Read the next block or create a new block if necessary
        if (currentBlock < file->header.Number_of_Blocks) {
            readBlock(file->file, currentBlock, &nextBlock);
        } else {
            memset(&nextBlock, 0, sizeof(Block));
            file->header.Number_of_Blocks++; // Increment block count if new block is created
        }

        char tempOverflow[BLOCK_SIZE * 2];
        memset(tempOverflow, 0, sizeof(tempOverflow)); // Clear the temporary overflow buffer

        // Copy the overflow data into the temporary buffer
        strncpy(tempOverflow, overflowData, BLOCK_SIZE);
        // If the next block already contains data, concatenate it
        if (strlen(nextBlock.data) > 0) {
            strcat(tempOverflow, nextBlock.data);
        }

        // If the combined overflow data exceeds the block size, split it
        if (strlen(tempOverflow) > BLOCK_SIZE) {
            strncpy(overflowData, tempOverflow + BLOCK_SIZE, strlen(tempOverflow) - BLOCK_SIZE);
            overflowData[strlen(tempOverflow) - BLOCK_SIZE] = '\0';
            tempOverflow[BLOCK_SIZE] = '\0';
        } else {
            memset(overflowData, 0, sizeof(overflowData));
            finalWrite = true; // If no more overflow data, set finalWrite to true
        }

        // Update the next block with the overflow data and write it back to the file
        strncpy(nextBlock.data, tempOverflow, BLOCK_SIZE);
        nextBlock.Byte_Used = strlen(nextBlock.data);
        nextBlock.Number_of_records = countRecordsInBlock(nextBlock.data);
        writeBlock(file->file, currentBlock++, &nextBlock); // Write the updated block to the file
    }

    file->header.Number_of_Records++; // Increment the number of records in the file
    setHeader(file->file, &file->header); // Update the file header with the new record count
}

// Function to load initial records into the TOVS file structure
void initialLoad_TOVS(File *file, int min, int rate) {
    // Generate and insert records with keys from min to rate
    for (int i = min; i <= rate; i++) {
        Record rec;
        rec.key = i;
        snprintf(rec.First_Name, sizeof(rec.First_Name), "First%d", i);
        snprintf(rec.Last_Name, sizeof(rec.Last_Name), "Last%d", i);
        snprintf(rec.Description, sizeof(rec.Description), "Record number %d", i);
        rec.Eraser = false; // Set the Eraser flag to false (not deleted)

        insertRecord_TOVS(file, rec); // Insert the record into the file
    }
    // Print a message when the initial load is completed
    printf("Initial load is completed with %d records.\n", (rate - min));
}

// Function to display the header information of the TOVS file structure
void Display_Header_TOVS(File *file) {
    printf("The number of blocks here is  : %d \n", file->header.Number_of_Blocks); // Print the total number of blocks
    printf("The number of records here is  : %d \n", file->header.Number_of_Records); // Print the total number of records
}

// Function to display the entire content of the TOVS file structure, including all blocks
void display_File_TOVS(File *file) {
    Display_Header_TOVS(file); // First, display the header information
    Block block;

    // Iterate through each block in the file and display its contents
    for (int i = 0; i < file->header.Number_of_Blocks; i++) {
        readBlock(file->file, i, &block); // Read the block data from the file
        char *token = strtok(block.data, DELIMITER); // Tokenize the block data using the delimiter
        
        printf("Block %d:\n", i); // Print the block number
        // Loop through all tokens (records) in the block and display them
        while (token != NULL) {
            Record existingRecord;
            String_to_Record(token, &existingRecord); // Convert the token into a record

            // Only display records that are not marked for deletion (Eraser flag is false)
            if (!existingRecord.Eraser) {
                printf("  %s\n", token); // Print the record data
            }
            token = strtok(NULL, DELIMITER); // Move to the next token
        }
    }
}

// Function to display the contents of a specific block by its block number
void Display_block_TOVS(File *file, int num_of_block) {
    Block block;
    readBlock(file->file, num_of_block, &block); // Read the block from the file
    char *token = strtok(block.data, DELIMITER); // Tokenize the block data using the delimiter
    
    printf("Contents of Block %d:\n", num_of_block); // Print the block number
    // Loop through all tokens (records) in the block and display them
    while (token != NULL) {
        Record existingRecord;
        String_to_Record(token, &existingRecord); // Convert the token into a record

        // Only display records that are not marked for deletion
        if (!existingRecord.Eraser) {
            printf("  %s\n", token); // Print the record data
        }
        token = strtok(NULL, DELIMITER); // Move to the next token
    }
}

// Function to search for overlapping records that span multiple blocks
void display_Overlapping_TOVS(File *file) {
    Block block;
    bool foundOverlap = false; // Flag to track if any overlapping records are found

    // Iterate through blocks to find overlapping records
    for (int blockNumber = 0; blockNumber < file->header.Number_of_Blocks - 1; blockNumber++) {
        readBlock(file->file, blockNumber, &block); // Read the block

        // Check if the block has unused space at the end (potential for overlapping)
        if (block.Byte_Used > 0 && block.data[block.Byte_Used - 1] != '\0') {
            foundOverlap = true;

            // Locate the last delimiter in the block data to identify partial records
            char *lastDelimiter = strrchr(block.data, DELIMITER[0]);
            if (lastDelimiter != NULL && (lastDelimiter - block.data) < BLOCK_SIZE - 1) {
                char partialRecordStr[BLOCK_SIZE + 1] = {0};
                strncpy(partialRecordStr, lastDelimiter + 1, BLOCK_SIZE - (lastDelimiter - block.data + 1));
                partialRecordStr[BLOCK_SIZE] = '\0'; // Ensure the string is null-terminated

                Record overlappingRecord;
                String_to_Record(partialRecordStr, &overlappingRecord); // Convert the partial record to a full record

                printf("Overlapping Record Detected: Key %d starts in Block %d and continues to Block %d.\n",
                       overlappingRecord.key, blockNumber, blockNumber + 1); // Display overlap information
            } else {
                printf("Error: Could not find a valid starting point for the overlapping record.\n");
            }
        }
    }

    // If no overlapping records were found, display a message
    if (!foundOverlap) {
        printf("No overlapping records across blocks found in the entire file.\n");
    }
}

// Function to search for a specific record by its key in the file
void search_TOVS(File *file, int key) {
    Block block;
    bool found = 0; // Flag to track if the record was found

    // Iterate through all blocks to search for the record
    for (int i = 0; i < file->header.Number_of_Blocks; i++) {
        readBlock(file->file, i, &block); // Read the block from the file
        char *token = strtok(block.data, DELIMITER); // Tokenize the block data

        int pos = 0; // Position counter within the block
        while (token != NULL) {
            Record existingRecord;
            String_to_Record(token, &existingRecord); // Convert the token into a record

            // If the record is found and not erased, display its information
            if (!existingRecord.Eraser && existingRecord.key == key) {
                found = 1;
                printf("Record with key %d was found in Block %d at Position %d\n", key, i, pos + 1);
            }

            token = strtok(NULL, DELIMITER); // Move to the next token
            pos++;
        }
    }

    // If the record was not found, display a message
    if (!found) {
        printf("Record with key %d was not found.\n", key);
    }
}

// Function to logically delete a record by marking it as erased (Eraser flag = true)
void logicalDelete_TOVS(File *file, int key) {
    Block block;
    bool found = false; // Flag to track if the record was found
    int blockNumberToDelete = -1; // Variable to store the block number where the record is located

    // Iterate through all blocks to find the record to delete
    for (int blockNumber = 0; blockNumber < file->header.Number_of_Blocks; blockNumber++) {
        readBlock(file->file, blockNumber, &block); // Read the block

        char tempBlockData[BLOCK_SIZE];
        strcpy(tempBlockData, block.data); // Copy block data into a temporary buffer

        char *token = strtok(tempBlockData, DELIMITER);

        // Loop through all records in the block
        while (token != NULL) {
            Record existingRecord;
            String_to_Record(token, &existingRecord); // Convert the token into a record

            // Skip records that are already logically deleted
            if (existingRecord.Eraser) {
                token = strtok(NULL, DELIMITER);
                continue;
            }

            // If the record is found, mark it as logically deleted
            if (existingRecord.key == key) {
                found = true;
                existingRecord.Eraser = true;

                // Convert the updated record to a string
                char updatedRecordStr[BLOCK_SIZE];
                Record_to_String(existingRecord, updatedRecordStr);

                // Update block data with the deleted record
                char updatedBlockData[BLOCK_SIZE];
                memset(updatedBlockData, 0, sizeof(updatedBlockData)); // Clear the updated data buffer
                char *innerToken = strtok(block.data, DELIMITER);

                // Rebuild the block data, skipping the deleted record
                while (innerToken != NULL) {
                    Record tempRecord;
                    String_to_Record(innerToken, &tempRecord); // Convert each token into a record

                    // Skip deleted records
                    if (tempRecord.Eraser && tempRecord.key != key) {
                        innerToken = strtok(NULL, DELIMITER);
                        continue;
                    }

                    // Add the record to the updated block data
                    if (strlen(updatedBlockData) > 0) {
                        strcat(updatedBlockData, DELIMITER);
                    }

                    if (tempRecord.key == key) {
                        strcat(updatedBlockData, updatedRecordStr); // Add the updated (deleted) record
                    } else {
                        strcat(updatedBlockData, innerToken); // Add the unchanged record
                    }

                    innerToken = strtok(NULL, DELIMITER);
                }

                // Update the block with the new data
                strncpy(block.data, updatedBlockData, BLOCK_SIZE);
                block.Byte_Used = strlen(updatedBlockData); // Update the number of bytes used
                block.Number_of_records = countRecordsInBlock(updatedBlockData); // Update the number of records
                writeBlock(file->file, blockNumber, &block); // Write the updated block to the file

                blockNumberToDelete = blockNumber;
                break;
            }

            token = strtok(NULL, DELIMITER); // Move to the next token
        }

        if (found) break;
    }

    // Display a message depending on whether the record was found and deleted
    if (found) {
        printf("Record with key %d successfully logically deleted.\n", key);
    } else {
        printf("Record with key %d not found or already deleted.\n", key);
    }
}


void physicalDelete_TOVS(File *file, int key) {
    Block block;  // Declare a block to hold data read from the file
    bool found = false;  // Flag to track whether the record is found
    // Buffer to store all the records from the file
    char allRecords[BLOCK_SIZE * file->header.Number_of_Blocks];
    memset(allRecords, 0, sizeof(allRecords));  // Initialize the buffer to 0

    int totalRecordsLength = 0;  // Variable to track the total length of all records

    // Loop through all blocks in the file
    for (int i = 0; i < file->header.Number_of_Blocks; i++) {
        // Read the current block
        readBlock(file->file, i, &block);

        // Temporary buffer to store the block's data
        char tempBlockData[BLOCK_SIZE];
        strcpy(tempBlockData, block.data);  // Copy the block data into the temp buffer

        // Tokenize the block's data using the delimiter
        char *token = strtok(tempBlockData, DELIMITER);

        // Loop through all records in the current block
        while (token != NULL) {
            Record existingRecord;  // Declare a variable to store the record
            // Parse the token into the fields of the Record structure
            sscanf(token, "%d,%20[^,],%20[^,],%100[^,],%d",
                   &existingRecord.key, existingRecord.First_Name,
                   existingRecord.Last_Name, existingRecord.Description,
                   (int *)&existingRecord.Eraser);

            // If the record matches the key and is not marked as deleted (Eraser = 0), skip it
            if (existingRecord.key == key && !existingRecord.Eraser) {
                found = true;  // Set the flag to indicate the record was found
                token = strtok(NULL, DELIMITER);  // Move to the next token (record)
                continue;  // Skip adding this record to the new records buffer
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

        // Loop to write records back to the blocks
        while (remainingBytes > 0) {
            int availableSpace = BLOCK_SIZE - newBlock.Byte_Used;  // Calculate available space in the block

            // If the current block cannot hold all the remaining records, fill it partially
            if (remainingBytes > availableSpace) {
                strncat(newBlock.data, recordPointer, availableSpace);  // Copy part of the data to the block
                newBlock.Byte_Used += availableSpace;  // Update the byte usage in the block
                newBlock.Number_of_records = countRecordsInBlock(newBlock.data);  // Update the record count
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
                newBlock.Number_of_records = countRecordsInBlock(newBlock.data);  // Update record count
                remainingBytes = 0;  // All remaining bytes have been written
            }
        }

        // If there are remaining records in the last block, write it
        if (newBlock.Byte_Used > 0) {
            writeBlock(file->file, blockNumber++, &newBlock);
        }

        // Update the file header to reflect the new number of blocks and records
        file->header.Number_of_Blocks = blockNumber;  // Set the final block count
        file->header.Number_of_Records--;  // Decrease the record count as one was deleted
        // Update the file header on disk
        setHeader(file->file, &file->header);

        // Inform the user that the record has been deleted
        printf("Record with key %d successfully deleted.\n", key);
    } else {
        // If the record wasn't found, inform the user
        printf("Record with key %d not found.\n", key);
    }
}

// Main menu function for the TOVS file operations
void TOVS(){
    int key;   // Key input for menu navigation
    int choice = 0; // The current menu option selected
    char filename[24];  // Name of the file to open
    Before_tovs(filename); // Prepare the filename (e.g., prompt user for the base filename)

    strcat(filename, ".tovs"); // Append the file extension

    // Open the file in read-write binary mode
    File *tovsFile = Open(filename, "rb+");
    sleep(3); // Pause for 3 seconds to simulate loading

    // Infinite loop to display the menu until the user exits
    while (1) {
        display_TnOVS_Menu(choice); // Display the menu
        key = getch(); // Read user input for menu navigation
        int reckey ,reckey2,reckey3 , Blocknum; // Variables to hold record keys for operations;
        if (key == 224) {
            key = getch(); // Capture special keys like arrow keys
            switch (key) {
                case 72: 
                    choice = (choice - 1 + 11) % 11; // Up arrow: move the selection up
                    break;
                case 80: 
                    choice = (choice + 1) % 11; // Down arrow: move the selection down
                    break;
            }
        } else if (key == 13) { 
            switch (choice) {
                case 0:
                    system("cls");
                    int rate , min;
                    printCentered("Enter the starting number of records you want to insert as an initial load : \n");
                    printf("\033[1A\033[63C"); 
                    scanf("%d" , &min);
                    printCentered("Enter the ending number of records you want to insert as an initial load : \n");
                    printf("\033[1A\033[63C"); 
                    scanf("%d" , &rate);
                    initialLoad_TOVS(tovsFile ,min ,rate); // Load records from min to rate
                    break;
                case 1:
                    system("cls");
                    Record rec ;
                    printCentered("Enter the information of the record that you want to insert : \n");
                    printCentered("key : ");
                    printf("\033[1A\033[65C"); 
                    scanf("%d", &rec.key);
                    printCentered("First Name : ");
                    printf("\033[0A\033[66C"); 
                    scanf("%20s",&rec.First_Name);
                    printCentered("Last Name : ");
                    printf("\033[0A\033[66C"); 
                    scanf("%20s",&rec.Last_Name);
                    printCentered("Description : ");
                    printf("\033[0A\033[68C"); 
                    scanf("%100s",&rec.Description);
                    rec.Eraser=false ;
                    insertRecord_TOVS(tovsFile , rec); // Insert the new record
                    break;
                case 2:
                    
                    system("cls");
                    printCentered("Enter the key of the record that you want to logically delete : \n");
                    printf("\033[1A\033[63C"); 
                    scanf("%d" , &reckey);
                    logicalDelete_TOVS(tovsFile ,reckey); // Logically delete a record by key
                    break;
                case 3:
                    system("cls");
                    printCentered("Enter the key of the record that you want to physically delete : \n");
                    printf("\033[1A\033[63C"); 
                    scanf("%d" , &reckey2);
                    physicalDelete_TOVS(tovsFile ,reckey2); // Physically delete a record by key
                    break;
                case 4:
                    system("cls");
                    printCentered("Enter the key of the record you want to search for : \n");
                    printf("\033[1A\033[63C"); 
                    scanf("%d" , &reckey3);
                    search_TOVS(tovsFile ,reckey3); // Search for a record by key
                    break;
                case 5:
                    system("cls");
                    printCentered("These are the information of your header : \n");
                    Display_Header_TOVS(tovsFile); // Display the file header
                    break;
                case 6:
                    system("cls");
                    printCentered("Enter the number of the block you want to display : \n");
                    scanf("%d" , &Blocknum );
                    Display_block_TOVS(tovsFile , Blocknum); // Display a specific block
                    break;
                case 7:
                    system("cls");
                    display_Overlapping_TOVS(tovsFile); // Display overlapping records
                    break;
                case 8:
                    system("cls");
                    display_File_TOVS(tovsFile); // Display all records in the file
                    break;
                case 9 :
                    return ; // Exit the function
                case 10:
                    exit(0); // Exit the program
                    break ;
                default:
                    printf("Invalid option\n"); // Handle invalid input
                    break;
            }
            printf("Press any key to continue...\n");
            getch(); // Wait for user input before continuing
        }
    }
}
