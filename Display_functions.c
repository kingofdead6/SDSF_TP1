#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <conio.h>
#include <windows.h>
#include <unistd.h> 
#include "Display_functions.h"

// DISPLAY FUNCTIONS

// Welcome function displays a welcoming message and clears the screen
// It also sets the console text color and prints a formatted message with details about the program
void welcome (){
    system("cls"); // Clears the console screen
    printf("\n\n");
    
    // Sets the text color to light yellow (color code 3)
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),3);
    
    // Prints the welcome message in a formatted box
    printf("     %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",201,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,203,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,187);
    printf("     %c .88d88b.  .d88888b    dP   %c     KAHLOUCHE YOUCEF   %c\n",186,186,186);
    printf("     %c 88.  `88  88.    \"'        %c                        %c\n",186,186,186);
    printf("     %c 888d888;  ~Y88888b.   88   %c     GROUPE  : 02       %c\n",186,186,186);
    printf("     %c 88.             `8b   88   %c     TP N:1             %c\n",186,186,186);
    printf("     %c  Y88888    Y88888P    dP   %c     TOVS TnOVS         %c\n",186,186,186);
    printf("     %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",204,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,202,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,185);
    printf("     %c   HIGHER   SCHOOL   OF   COMPUTER   SIENCE          %c\n",186,186);
    printf("     %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",200,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,188);
    printf("\n");

    // Resets the text color back to default (white)
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),7);

    // Displays a thank you message and prompts the user to continue
    printf("        THANKS FOR CHOSING OUR APPLICATION! ");
    printf(" \n\n        press ENTER to continue ! ");
    getchar(); // Waits for the user to press ENTER before continuing
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

// Sets the console text color using the specified color code
void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

// Resets the console text color back to the default (white)
void resetColor() {
    setColor(7);
}

// Prints the given text centered on the screen
// This function calculates the width of the console and prints spaces before the text to center it
void printCentered(const char *text) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    
    // Gets the current console screen buffer info to determine the screen width
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int consoleWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;

    // Calculates the number of spaces required to center the text
    int textLength = strlen(text);
    int space = (consoleWidth - textLength) / 2;

    // Prints the spaces to center the text and then prints the text
    for (int i = 0; i < space; i++) {
        printf(" ");
    }
    printf("%s\n", text);
}

// Displays the menu for the TnOVS file system
// Highlights the selected option and provides a formatted menu layout
void display_TnOVS_Menu(int choice) {
    system("cls"); // Clears the console screen
    printf("\033[32m"); // Sets the text color to green
    printCentered(" ____________________________________________ ");
    printCentered("|                                            |");
    printCentered("|                    MENU                    |");
    printCentered("|____________________________________________|");
    printf("\033[0m"); // Resets the text color

    printf("\n\n");
    
    // List of menu options
    const char *options[11] = {
        "1: Give an initial load",
        "2: Insert a Record",
        "3: Delete Logically a Record",
        "4: Delete Physically a Record",
        "5: Search about a Record",
        "6: Display the Header",
        "7: Display a Certain Block",
        "8: Display the Overlapping Records",
        "9: Display the Whole File_tnovs",
        "10: Back",
        "0: Exit"
    };

    // Loops through the options and prints each one
    for (int i = 0; i < 11; i++) {
        if (i == choice) {
            setColor(4); // Yellow for highlighted option
            printCentered(options[i]);
            resetColor();
        } else {
            printCentered(options[i]);
        }
    }
}

// Displays the menu for the TOVS file system, similar to the TnOVS menu
// Highlights the selected option and provides a formatted menu layout
void display_TOVS_Menu(int choice) {
    system("cls");
    printf("\033[32m");
    printCentered(" ____________________________________________ ");
    printCentered("|                                            |");
    printCentered("|                    MENU                    |");
    printCentered("|____________________________________________|");
    printf("\033[0m");
    printf("\n\n");

    // List of menu options for TOVS
    const char *options[11] = {
        "1: Give an initial load",
        "2: Insert a Record",
        "3: Delete Logically a Record",
        "4: Delete Physically a Record",
        "5: Search about a Record",
        "6: Display the Header",
        "7: Display a Certain Block",
        "8: Display the Overlapping Records",
        "9: Display the Whole File_tnovs",
        "10: Back",
        "0: Exit"
    };

    // Loops through the options and prints each one
    for (int i = 0; i < 11; i++) {
        if (i == choice) {
            setColor(4); // Yellow for highlighted option
            printCentered(options[i]);
            resetColor();
        } else {
            printCentered(options[i]);
        }
    }
}

// Displays the main menu of the program, offering options for TOVS, TNOVS, and Bonus
// Highlights the selected option and displays the menu in a formatted layout
void display_Main_Menu (int choice) {
    system("cls");
    printf("\033[31m"); // Sets the text color to red
    printCentered(" ____________________________________________ ");
    printCentered("|                                            |");
    printCentered("|               MAIN MENU                    |");
    printCentered("|____________________________________________|");
    printf("\033[0m"); // Resets the text color
    printf("\n\n");

    // List of main menu options
    const char *options[4] = {
        "1: TOVS",
        "2: TNOVS",
        "3: Bonus",
        "0: Exit"
    };

    // Loops through the options and prints each one
    for (int i = 0; i < 4 ; i++) {
        if (i == choice) {
            setColor(6); // Cyan for highlighted option
            printCentered(options[i]);
            resetColor();
        } else {
            printCentered(options[i]);
        }
    }
}

// Displays the Bonus menu with options for file operations
// It highlights the selected option and provides a formatted menu layout
void display_Bonus_Menu(int choice) {
    system("cls");  // Clears the console screen
    printf("\033[31m");  // Sets the text color to red
    printCentered(" ____________________________________________ ");
    printCentered("|                                            |");
    printCentered("|               Bonus MENU                   |");
    printCentered("|____________________________________________|");
    printf("\033[0m");  // Resets the text color back to default
    printf("\n\n");

    // List of bonus menu options
    const char *options[5] = {
        "1: Reorganize a file after deleting 50% of its characters :",
        "2: Intersection of 2 files :",
        "3: Concatenation of 2 files without duplicates :",
        "4: Return",
        "0: Exit"
    };

    // Loops through the options and prints each one
    // If the option is the currently selected one, it is highlighted
    for (int i = 0; i < 5; i++) {
        if (i == choice) {
            setColor(6);  // Cyan color for the highlighted option
            printCentered(options[i]);  // Prints the highlighted option centered
            resetColor();  // Resets the text color
        } else {
            printCentered(options[i]);  // Prints non-highlighted options centered
        }
    }
}

// Asks the user to input the file name for the TnOVS file system
// It displays a formatted prompt asking for the file name
void Before_tnovs(char *File_tnovsname) {
    printf("\033[31m");  // Sets the text color to red
    printCentered("______________________________________________________________");
    printCentered("|                                                             |");
    printCentered("|                                                             |");
    printCentered("|        BEFORE WE START YOU SHOULD ENTER THE File_tnovs NAME     |");
    printCentered("|                                                             |");
    printCentered("|                                                             |");
    printCentered("|_____________________________________________________________|\n");
    printf("\033[0m");  // Resets the text color back to default

    // Moves the cursor to a specific location to input the file name
    printf("\033[3A\033[50C");  

    // Reads the file name from the user, ensuring it does not exceed 19 characters
    scanf("%19s", File_tnovsname);

    printf("\n\n\n");  // Adds some spacing after the input
}

// Asks the user to input the file name for the TOVS file system
// It displays a formatted prompt asking for the file name
void Before_tovs(char *File_tovsname) {
    printf("\033[31m");  // Sets the text color to red
    printCentered("______________________________________________________________");
    printCentered("|                                                             |");
    printCentered("|                                                             |");
    printCentered("|        BEFORE WE START YOU SHOULD ENTER THE File_tovs NAME      |");
    printCentered("|                                                             |");
    printCentered("|                                                             |");
    printCentered("|_____________________________________________________________|\n");
    printf("\033[0m");  // Resets the text color back to default

    // Moves the cursor to a specific location to input the file name
    printf("\033[3A\033[50C");  

    // Reads the file name from the user, ensuring it does not exceed 19 characters
    scanf("%19s", File_tovsname);

    printf("\n\n\n");  // Adds some spacing after the input
}
