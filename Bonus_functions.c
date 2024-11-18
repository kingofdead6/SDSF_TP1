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

#define MAX_KEY 1000  // Define the maximum possible key value for records

// Function to transfer 50% of records from the source file to the new file
void transfer50PercentRecords(File *sourceFile, File *newFile) {
    int totalRecords = sourceFile->header.Number_of_Records;  // Get the total number of records in the source file
    int recordsToTransfer = totalRecords / 2;  // Calculate 50% of the records to transfer
    int transferredCount = 0;  // Track the number of transferred records

    // Loop through each block in the source file
    for (int blockNumber = 0; blockNumber < sourceFile->header.Number_of_Blocks; blockNumber++) {
        Block block;
        readBlock(sourceFile->file, blockNumber, &block);  // Read a block of data from the source file

        char tempBlockData[BLOCK_SIZE];
        strcpy(tempBlockData, block.data);  // Copy block data into a temporary buffer

        char *token = strtok(tempBlockData, DELIMITER);  // Tokenize the block data to process each record

        // Loop through each token (record) and transfer 50% of them to the new file
        while (token != NULL && transferredCount < recordsToTransfer) {
            Record record;
            String_to_Record(token, &record);  // Convert token into a record

            if (!record.Eraser) {  // Check if the record is not marked for deletion
                insertRecord_TnOVS(newFile, record);  // Insert the record into the new file
                transferredCount++;  // Increment the transferred count
            }

            token = strtok(NULL, DELIMITER);  // Move to the next token
        }

        if (transferredCount >= recordsToTransfer) {
            break;  // Exit the loop if 50% of the records have been transferred
        }
    }

    printf("Successfully transferred %d records to the new file.\n", transferredCount);  // Print a success message
}

// Function to find the intersection of two files and store it in a result file
void findIntersection(File *file1, File *file2, File *resultFile) {
    // Loop through each block in the first file
    for (int blockNumber1 = 0; blockNumber1 < file1->header.Number_of_Blocks; blockNumber1++) {
        Block block1;
        readBlock(file1->file, blockNumber1, &block1);  // Read a block from the first file

        char tempBlockData1[BLOCK_SIZE];
        strcpy(tempBlockData1, block1.data);  // Copy block data into a temporary buffer

        char *token1 = strtok(tempBlockData1, DELIMITER);  // Tokenize the block data to process each record

        // Loop through each token (record) in the first file
        while (token1 != NULL) {
            Record record1;
            String_to_Record(token1, &record1);  // Convert token into a record

            if (!record1.Eraser) {  // Check if the record is not marked for deletion
                // Compare this record with records in the second file
                for (int blockNumber2 = 0; blockNumber2 < file2->header.Number_of_Blocks; blockNumber2++) {
                    Block block2;
                    readBlock(file2->file, blockNumber2, &block2);  // Read a block from the second file

                    char tempBlockData2[BLOCK_SIZE];
                    strcpy(tempBlockData2, block2.data);  // Copy block data into a temporary buffer

                    char *token2 = strtok(tempBlockData2, DELIMITER);  // Tokenize the block data to process each record

                    // Loop through each token (record) in the second file
                    while (token2 != NULL) {
                        Record record2;
                        String_to_Record(token2, &record2);  // Convert token into a record

                        if (!record2.Eraser && record1.key == record2.key) {  // Check if the records have the same key
                            insertRecord_TnOVS(resultFile, record1);  // Insert the record into the result file
                        }

                        token2 = strtok(NULL, DELIMITER);  // Move to the next token in the second file
                    }
                }
            }

            token1 = strtok(NULL, DELIMITER);  // Move to the next token in the first file
        }
    }

    printf("Intersection of files has been stored in the result file.\n");  // Print a success message
}

// Function to process a file and insert records into the result file, avoiding duplicates
void processFile(File *file, File *resultFile, bool *insertedKeys) {
    // Loop through each block in the file
    for (int blockNumber = 0; blockNumber < file->header.Number_of_Blocks; blockNumber++) {
        Block block;
        readBlock(file->file, blockNumber, &block);  // Read a block from the file

        char tempBlockData[BLOCK_SIZE];
        strcpy(tempBlockData, block.data);  // Copy block data into a temporary buffer

        char *token = strtok(tempBlockData, DELIMITER);  // Tokenize the block data to process each record

        // Loop through each token (record)
        while (token != NULL) {
            Record record;
            String_to_Record(token, &record);  // Convert token into a record

            if (!record.Eraser && !insertedKeys[record.key]) {  // Check if the record is not deleted and not already inserted
                insertRecord_TnOVS(resultFile, record);  // Insert the record into the result file
                insertedKeys[record.key] = true;  // Mark the record's key as inserted
            }

            token = strtok(NULL, DELIMITER);  // Move to the next token
        }
    }
}

// Function to concatenate two files without duplicates
void concatenateFilesWithoutDuplicates(File *file1, File *file2, File *resultFile) {
    bool insertedKeys[MAX_KEY] = {false};  // Initialize an array to track inserted keys

    // Process the first file and insert records without duplicates
    processFile(file1, resultFile, insertedKeys);

    // Process the second file and insert records without duplicates
    processFile(file2, resultFile, insertedKeys);

    printf("Files concatenated without duplicates into the result file.\n");  // Print a success message
}

// Bonus function that presents a menu for bonus operations like transferring records, intersection, and concatenation
void Bonus() {
    int choice = 0;  // Initialize the user's menu choice
    int key;
    char filename[24], filename1[24], filename2[24];

    system("cls");  // Clear the screen

    while (1) {
        display_Bonus_Menu(choice);  // Display the bonus menu

        key = getch();  // Get the user's input

        if (key == 224) {  // Handle arrow key inputs
            key = getch();
            switch (key) {
                case 72:  // Up arrow
                    choice = (choice - 1 + 5) % 5;  // Move up in the menu
                    break;
                case 80:  // Down arrow
                    choice = (choice + 1) % 5;  // Move down in the menu
                    break;
            }
        } else if (key == 13) {  // Handle Enter key input
            switch (choice) {
                case 0: {  // Option 0: Transfer 50% of records from one file to another
                    system("cls");
                    printCentered("Enter the name of the Entrance file (you should have data before doing this):");
                    scanf("%s", filename);
                    strcat(filename, ".tnovs");
                    File *Entery_file = Open(filename, "rb+");

                    char newFilename[24];
                    snprintf(newFilename, sizeof(newFilename), "new_%s", filename);
                    File *Out_file = Open(newFilename, "rb+");

                    transfer50PercentRecords(Entery_file, Out_file);  // Transfer 50% of records
                    break;
                }
                case 1: {  // Option 1: Find intersection between two files
                    system("cls");
                    printCentered("Enter the name of the first file (you should have data before doing this):");
                    scanf("%s", filename1);
                    strcat(filename1, ".tnovs");
                    File *first_file = Open(filename1, "rb+");

                    printCentered("Enter the name of the second file (you should have data before doing this):");
                    scanf("%s", filename2);
                    strcat(filename2, ".tnovs");
                    File *second_file = Open(filename2, "rb+");

                    char newfile2name[24] = "Intersection.tnovs";
                    File *Out_file2 = Open(newfile2name, "rb+");

                    findIntersection(first_file, second_file, Out_file2);  // Find intersection of the files
                    break;
                }
                case 2: {  // Option 2: Concatenate two files without duplicates
                    system("cls");
                    printCentered("Enter the name of the first file (you should have data before doing this):");
                    scanf("%s", filename1);
                    strcat(filename1, ".tnovs");
                    File *first_file = Open(filename1, "rb+");

                    printCentered("Enter the name of the second file (you should have data before doing this):");
                    scanf("%s", filename2);
                    strcat(filename2, ".tnovs");
                    File *second_file = Open(filename2, "rb+");

                    char newfile2name[24] = "Concatenated.tnovs";
                    File *Out_file2 = Open(newfile2name, "rb+");

                    concatenateFilesWithoutDuplicates(first_file, second_file, Out_file2);  // Concatenate files without duplicates
                    break;
                }
                case 3:
                    return;  // Exit the function
                case 4:
                    exit(0);  // Exit the program
                    break;
                default:
                    printf("Invalid option\n");
                    break;
            }
            printf("Press any key to continue...\n");
            getch();  // Wait for user input to continue
        }
    }
}
