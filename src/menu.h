#pragma once

typedef struct sMenu tMenu;
typedef struct sMenuItem tMenuItem;

typedef struct sMenuInfo {
    const tMenu *menu;
    const tMenuItem *item;
    void *data;
    int index;
}tMenuInfo;

// The actual menu item.
// Holds the name of the item, and function pointers for when the item is displayed
// and if the user selects the item.
// The data pointer can point to any piece of data, and will be passed into the execute and draw
// functions.
typedef struct sMenuItem{
    const char *name;
    void (*onExecute)(const tMenuInfo *menuInfo);
    void (*onDraw)(const tMenuInfo *menuInfo);
    void *data;
}tMenuItem;

// The actual menu (or submenu).
// Holds the name of the menu, and the array of menu items.
// The index of the top level menu needs to be set to 0,
// and the previous pointer to NULL.
// Notice that the addition of the "previous" pointer makes our menu
// act a bit like a linked list, so we can dynamically move up and down in
// the menu structure.
typedef struct sMenu{
    const char *name;   // Leave this NULL to skip drawing the menu name.
    const tMenuItem *items;
    int index;
    struct sMenu *previous;
}tMenu;

// Instead of having to pass a pointer-to-pointer into MenuDraw, we can pass a pointer
// to tCurrentMenu, which makes it easier to think about.
typedef struct {
    tMenu *menu;
}tCurrentMenu;


// An enumeration for separating menu button presses from the actual key strokes used
// to navigate the menu.
typedef enum {
    eButtonNone,
    eButtonUp,
    eButtonDown,
    eButtonLeft,
    eButtonRight,
}eButton;

#ifdef __cplusplus
extern "C"
{
#endif

    // Place this function in the "onExecute" callback, and the address of the target menu in the data pointer,
    // to navigate to a submenu.
    void MenuOnExecuteSubMenu(const tMenuInfo *);

    void MenuInit(tCurrentMenu *currentMenu, tMenu *menu);
    void MenuUpdate(tCurrentMenu *currentMenu, eButton buttonPress);

    // MenuClearScreen needs to be defined somewhere in the calling code.
    typedef enum {
        eMCC_ClearScreen,
        eMCC_FinishedDrawingItem,
        eMCC_DrawingSelectedItem,
        eMCC_DrawingItem,
        eMCC_CallingOnDraw
    }eMenuControlCode;
    extern void MenuControlCode(eMenuControlCode);
    // MenuDrawString needs to be defined somewhere in the calling code.
    // It may be called multiple times to draw each line of the menu.
    // A string containing only a newline character will be sent to indicate the end of a menu line.
    // IE: "\n".
    extern void MenuDrawString(const char*);

#ifdef __cplusplus
}
#endif
