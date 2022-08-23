#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "menu.h"
#include "key.h"

const char* VersionString = "0.5.0";

void InputsInputOnDraw(const tMenu::tInfo *info)
{
    printf("Input %d", info->index+1);
}

#define MENU_INPUTS_COUNT (8)
tMenu::tItem menuInputsItems[MENU_INPUTS_COUNT+1];

tMenu::tSub menuInputs = {
    "Inputs",
    menuInputsItems
};

void MainDiagnosticsCounterOnExecute(const tMenu::tInfo *info)
{
    int *counter = (int*)info->data;
    ++*counter;
}

void MainDiagnosticsCounterOnDraw(const tMenu::tInfo *info)
{
    int *counter = (int*)info->data;
    printf("%d", *counter);
}
void MainDiagnosticsCounterExpOnExecute(const tMenu::tInfo *info)
{
    int *counter = (int*)info->data;
    *counter *= *counter;
}

static int diagnosticsCounter = 0;
const tMenu::tItem menuDiagnosticsItems[] = {
    {"Counter",  MainDiagnosticsCounterOnExecute, MainDiagnosticsCounterOnDraw, &diagnosticsCounter},
    {"Counter Exp",  MainDiagnosticsCounterExpOnExecute, NULL, &diagnosticsCounter},
    {"Inputs",  tMenu::OnExecuteSubMenu, NULL, (void*)&menuInputs},
    {NULL}
};

tMenu::tSub menuDiagnostics = {
    "Diagnostics",
    menuDiagnosticsItems
};

void MenuMainDateOnDraw(const tMenu::tInfo *info)
{
    time_t rawTime;
    time(&rawTime);
    char tempString[50];
    strftime(tempString, 50, "%F", localtime(&rawTime));
    printf("%s", tempString);
}

void MenuMainTimeOnDraw(const tMenu::tInfo *info)
{
    time_t rawTime;
    time(&rawTime);
    char tempString[50];
    strftime(tempString, 50, "%T", localtime(&rawTime));
    printf("%s", tempString);
}

void MenuMainVersionOnDraw(const tMenu::tInfo *info)
{
    printf("%s", VersionString);
}

const tMenu::tItem menuMainItems[] = {
    {"Date",  NULL, MenuMainDateOnDraw, NULL},
    {"Time",  NULL, MenuMainTimeOnDraw, NULL},
    {"Diagnostics",  tMenu::OnExecuteSubMenu, NULL, (void*)&menuDiagnostics},
    {"Version", NULL, MenuMainVersionOnDraw, NULL},
    {NULL}
};

tMenu::tSub menuMain = {
    "Main",
    menuMainItems,
    0,
    NULL
};


class tMenuActual : public tMenu {
public:
    tMenuActual() :
        tMenu(&menuMain)
    {
	    KeyStartScanMode();
        ControlCode(eControlCode::ClearScreen);
    }
    ~tMenuActual() override
    {
	    KeyStopScanMode();
    }
    bool HasExited() const
    {
        return quit_;
    }
    void Update()
    {
        int inputChar = KeyGetChar();
        eButton buttonPress = eButton::None;
        if (inputChar > 0) {
            switch ((char)inputChar) {
                case 'w':
                    buttonPress = eButton::Up;
                    break;
                case 's':
                    buttonPress = eButton::Down;
                    break;
                case 'a':
                    buttonPress = eButton::Left;
                    break;
                case 'd':
                    buttonPress = eButton::Right;
                    break;
                case 'q':
                    quit_ = true;
                    break;
                default:
                    break;
            }
        }
        tMenu::Update(buttonPress);
        if (ExitAttempted()) {
            quit_ = true;
        }
    }
protected:
    void DrawString(const char *str) final
    {
        printf("%s", str);
    }

    void ControlCode(eControlCode code) final
    {
        switch (code) {
        case eControlCode::ClearScreen:
            printf("\033[H\033[J");
            break;
        case eControlCode::FinishedDrawingItem:
            printf("\n");
            break;
        case eControlCode::DrawingSelectedItem:
            printf("-> ");
            break;
        case eControlCode::DrawingItem:
            printf("   ");
            break;
        case eControlCode::CallingOnDraw:
            printf(" ");
            break;
        }
    }
private:
    bool quit_{};
};

int main()
{
    printf ("----- Darren's Dummy Menu Version %s -----\n\n", VersionString);

    // Dynamically create the items for the Inputs menu.
    for (int i = 0; i < MENU_INPUTS_COUNT; ++i) {
        menuInputsItems[i].name = "";
        menuInputsItems[i].onDraw = InputsInputOnDraw;
    }
    // Don't forget to add the final "NULL" item.
    menuInputsItems[MENU_INPUTS_COUNT].name = NULL;

    tMenuActual menu;
    // Call update once so that it draws the first time.
    menu.Update();

    bool quit = false;
    do {
        menu.Update();
        quit = menu.HasExited();

        if (quit) {
            printf ("Exiting Menu\n");
            break;
        }
        else {
            usleep(100);
        }
    } while (!quit);

    printf ("\nExiting Program\n");

    return 0;
}
