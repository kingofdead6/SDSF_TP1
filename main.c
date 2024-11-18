#include <stdio.h>
#include "TNOVS_functions.h"  // Header for functions related to the TNOVS file system
#include "TOVS_functions.h"   // Header for functions related to the TOVS file system
#include "Display_functions.h" // Header for display-related functions
#include "Bonus_functions.h"   // Header for bonus functions

int main() {
    int choice = 0; // Variable to keep track of the current menu option (initially 0)
    int key;        // Variable to capture the key press from the user
    
    // Call the welcome function to display any introductory messages
    welcome();
    
    system("cls"); // Clear the console screen
    
    // Infinite loop for the main menu, allowing continuous interaction with the program
    while (1) {
        // Display the main menu with the current selection highlighted
        display_Main_Menu(choice);

        // Capture the user's key press
        key = getch();

        // Check if the key is a special key (arrow keys)
        if (key == 224) {
            key = getch(); // Capture the second part of the special key
            switch (key) {
                case 72: // Up arrow key
                    choice = (choice - 1 + 4) % 4; // Move selection up in the menu, wrap around
                    break;
                case 80: // Down arrow key
                    choice = (choice + 1) % 4; // Move selection down in the menu, wrap around
                    break;
            }
        } 
        // Check if the Enter key was pressed
        else if (key == 13) {
            switch (choice) {
                case 0: // Option 0: TOVS functionality
                    system("cls");  // Clear the screen
                    TOVS();         // Call the TOVS function
                    break;
                case 1: // Option 1: TnOVS functionality
                    system("cls");  // Clear the screen
                    TnOVS();         // Call the TnOVS function
                    break;
                case 2: // Option 2: Bonus functionalities
                    system("cls");  // Clear the screen
                    Bonus();         // Call the Bonus function
                    break;
                case 3: // Option 3: Exit the program
                    exit(0);         // Exit the program
                    break;
                default:
                    printf("Invalid option\n"); // In case of an invalid menu option
                    break;
            }
            // Prompt the user to press any key to continue after an action
            printf("Press any key to continue...\n");
            getch();  // Wait for the user to press a key
        }
    }
}
