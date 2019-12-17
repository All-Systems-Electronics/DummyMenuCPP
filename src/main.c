#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "menu.h"

const char* VersionString = "0.1.0";

void InputsInputOnDraw(const tMenuInfo *info)
{
    printf("Input %d", info->index+1);
}

#define MENU_INPUTS_COUNT (8)
tMenuItem menuInputsItems[MENU_INPUTS_COUNT+1];

tMenu menuInputs = {
    "Inputs",
    menuInputsItems
};

void MainDiagnosticsCounterOnExecute(const tMenuInfo *info)
{
    int *counter = (int*)info->data;
    ++*counter;
}

void MainDiagnosticsCounterOnDraw(const tMenuInfo *info)
{
    int *counter = (int*)info->data;
    printf("%d", *counter);
}
void MainDiagnosticsCounterExpOnExecute(const tMenuInfo *info)
{
    int *counter = (int*)info->data;
    *counter *= *counter;
}

static int diagnosticsCounter = 0;
const tMenuItem menuDiagnosticsItems[] = {
    {"Counter",  MainDiagnosticsCounterOnExecute, MainDiagnosticsCounterOnDraw, &diagnosticsCounter},
    {"Counter Exp",  MainDiagnosticsCounterExpOnExecute, NULL, &diagnosticsCounter},
    {"Inputs",  MenuOnExecuteSubMenu, NULL, (void*)&menuInputs},
    {NULL}
};

tMenu menuDiagnostics = {
    "Diagnostics",
    menuDiagnosticsItems
};

void MenuMainDateOnDraw(const tMenuInfo *info)
{
    time_t rawTime;
    time(&rawTime);
    char tempString[50];
    strftime(tempString, 50, "%F", localtime(&rawTime));
    printf("%s", tempString);
}

void MenuMainTimeOnDraw(const tMenuInfo *info)
{
    time_t rawTime;
    time(&rawTime);
    char tempString[50];
    strftime(tempString, 50, "%T", localtime(&rawTime));
    printf("%s", tempString);
}

void MenuMainVersionOnDraw(const tMenuInfo *info)
{
    printf("%s", VersionString);
}

const tMenuItem menuMainItems[] = {
    {"Date",  NULL, MenuMainDateOnDraw, NULL},
    {"Time",  NULL, MenuMainTimeOnDraw, NULL},
    {"Diagnostics",  MenuOnExecuteSubMenu, NULL, (void*)&menuDiagnostics},
    {"Version", NULL, MenuMainVersionOnDraw, NULL},
    {NULL}
};

tMenu menuMain = {
    "Main",
    menuMainItems,
    0,
    NULL
};

int main()
{
    // Modify terminal so we can read characters a single char at a time without having to press enter (unbuffered).
    system("stty -icanon min 0");
    printf ("----- Darren's Dummy Menu Version %s -----\n\n", VersionString);

    // Dynamically create the items for the Inputs menu.
    for (int i = 0; i < MENU_INPUTS_COUNT; ++i) {
        menuInputsItems[i].name = "";
        menuInputsItems[i].onDraw = InputsInputOnDraw;
    }
    // Don't forget to add the final "NULL" item.
    menuInputsItems[MENU_INPUTS_COUNT].name = NULL;

    // Initialise the menu with the top level menu.
    tCurrentMenu currentMenu;
    MenuInit(&currentMenu, &menuMain);
    // Call update once so that it draws the first time.
    MenuUpdate(&currentMenu, eButtonNone);

    bool quit = false;
    do {
        int inputChar = getchar();
        eButton buttonPress = eButtonNone;
        if (inputChar > 0) {
            switch ((char)inputChar) {
                case 'w':
                    buttonPress = eButtonUp;
                    break;
                case 's':
                    buttonPress = eButtonDown;
                    break;
                case 'a':
                    buttonPress = eButtonLeft;
                    break;
                case 'd':
                    buttonPress = eButtonRight;
                    break;
                case 'q':
                    quit = true;
                    break;
                default:
                    break;
            }
        }

        MenuUpdate(&currentMenu, buttonPress);

        if (!currentMenu.menu) {
            printf ("Exiting Menu\n");
            break;
        }
        else {
            usleep(100);
        }
    } while (!quit);


    printf ("\nExiting Program\n");

    // Return the terminal to its normal 'buffered' behaviour.
    system("stty cooked");

    return 0;
}
