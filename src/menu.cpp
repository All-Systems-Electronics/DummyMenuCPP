#include "menu.h"
#include "stddef.h"

void MenuOnExecuteSubMenu(const tMenuInfo *){}


void MenuInit(tCurrentMenu *currentMenu, tMenu *menu)
{
    menu->index = 0;
    menu->previous = NULL;
    currentMenu->menu = menu;
    MenuControlCode(eMCC_ClearScreen);
}

// Handles the drawing, navigating and updating of the menu.
static void MenuNavigate(tCurrentMenu *currentMenu, eButton buttonPress)
{
    // Make sure sensible values have been passed into the function.
    if (!currentMenu) {
        return;
    }
    if (!currentMenu->menu) {
        return;
    }

    tMenu *menu = currentMenu->menu;

    // Handle the navigation of the menu first, so that we update the menu before we draw it.
    switch (buttonPress) {
        case eButtonUp:
            if (menu->index > 0) {
                --menu->index;
            }
            else {
                int i = menu->index + 1;
                while (menu->items[i].name) {
                    ++menu->index;
                    ++i;
                }
            }
            break;
        case eButtonDown:
            if (menu->items[menu->index].name && menu->items[menu->index + 1].name) {
                ++menu->index;
            }
            else {
                menu->index = 0;
            }
            break;
        case eButtonLeft:
            currentMenu->menu = menu->previous;
            break;
        case eButtonRight:
            {
                const tMenuItem *item = &menu->items[menu->index];
                if (item->onExecute == MenuOnExecuteSubMenu) {
                    if (item->data) {
                        tMenu *subMenu = (tMenu*)item->data;
                        subMenu->previous = currentMenu->menu;
                        subMenu->index = 0;
                        currentMenu->menu = subMenu;
                    }
                }
                else if (item->onExecute) {
                    tMenuInfo menuInfo = {
                        .menu = menu,
                        .item = item,
                        .data = item->data,
                        .index = menu->index
                    };
                    item->onExecute(&menuInfo);
                }
                break;
            }
        default:
            break;
    }
}

static void MenuDraw(const tMenu *menu)
{
    // Make sure sensible values have been passed into the function.
    if (!menu) {
        return;
    }

    if (menu->name) {
        MenuDrawString(menu->name);
        MenuControlCode(eMCC_FinishedDrawingItem);
    }

    int i = 0;
    while (menu->items[i].name != NULL) {
        const tMenuItem *item = &menu->items[i];
        if (i == menu->index) {
            MenuControlCode(eMCC_DrawingSelectedItem);
        }
        else {
            MenuControlCode(eMCC_DrawingItem);
        }
        MenuDrawString(item->name);

        if (item->onDraw) {
            MenuControlCode(eMCC_CallingOnDraw);
            
            tMenuInfo menuInfo = {
                .menu = menu,
                .item = item,
                .data = item->data,
                .index = i
            };
            item->onDraw(&menuInfo);
        }

        MenuControlCode(eMCC_FinishedDrawingItem);
        ++i;
    } 
}


void MenuUpdate(tCurrentMenu *currentMenu, eButton buttonPress)
{
    MenuNavigate(currentMenu, buttonPress);

    MenuControlCode(eMCC_ClearScreen);
    if (currentMenu->menu) {
        MenuDraw(currentMenu->menu);
    }
}
