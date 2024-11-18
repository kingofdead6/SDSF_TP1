#ifndef DISPLAY_FUNCTIONS_H  
#define DISPLAY_FUNCTIONS_H

#include <stdio.h>   
#include <stdlib.h>  
#include <time.h>    
#include <stdbool.h> 
#include <windows.h> 
#include <conio.h>
#include <unistd.h>  

// Macro to clear the screen based on the operating system
#ifdef _WIN32
    #define CLEAR_SCREEN "cls"  
#else
    #define CLEAR_SCREEN "clear"
#endif

// Function declarations for display-related functionalities
void welcome();  // Displays a welcome message with system details
void setColor(int color);  // Sets the text color in the console
void resetColor();  // Resets the console text color to default
void printCentered(const char *text);  // Prints the text centered on the console
void display_TnOVS_Menu(int choice);  // Displays the menu for the TnOVS file system
void display_TOVS_Menu(int choice);  // Displays the menu for the TOVS file system
void display_Main_Menu(int choice);  // Displays the main menu with options
void display_Bonus_Menu(int choice);  // Displays the bonus menu with additional options
void Before_tnovs(char *filename);  // Prompts the user to enter the TnOVS file name
void Before_tovs(char *filename);  // Prompts the user to enter the TOVS file name

#endif  // End of the header guard
