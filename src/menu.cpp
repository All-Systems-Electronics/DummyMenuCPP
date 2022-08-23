#include "menu.h"
#include "stddef.h"

tMenu::tMenu(tSub* topMenu):
    currentSub_(topMenu)
{
    currentSub_->index = 0;
    currentSub_->previous = nullptr;
}

tMenu::~tMenu()
{
    
}

// Handles the drawing, navigating and updating of the menu.
void tMenu::Navigate(eButton buttonPress)
{
    if (!currentSub_) {
        return;
    }

    auto* menu = currentSub_;
    exitAttempted_ = false;

    // Handle the navigation of the menu first, so that we update the menu before we draw it.
    switch (buttonPress) {
        case eButton::Up:
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
        case eButton::Down:
            if (menu->items[menu->index].name && menu->items[menu->index + 1].name) {
                ++menu->index;
            }
            else {
                menu->index = 0;
            }
            break;
        case eButton::Left:
            if (menu->previous) {
                currentSub_ = menu->previous;
            }
            else {
                exitAttempted_ = true;
            }
            break;
        case eButton::Right:
            {
                const tItem *item = &menu->items[menu->index];
                if (item->onExecute == OnExecuteSubMenu) {
                    if (item->data) {
                        tSub *subMenu = (tSub*)item->data;
                        subMenu->previous = currentSub_;
                        subMenu->index = 0;
                        currentSub_ = subMenu;
                    }
                }
                else if (item->onExecute) {
                    tInfo menuInfo = {
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

void tMenu::Draw(const tSub *menu)
{
    // Make sure sensible values have been passed into the function.
    if (!menu) {
        return;
    }

    if (menu->name) {
        DrawString(menu->name);
        ControlCode(eControlCode::FinishedDrawingItem);
    }

    int i = 0;
    while (menu->items[i].name != NULL) {
        const tItem *item = &menu->items[i];
        if (i == menu->index) {
            ControlCode(eControlCode::DrawingSelectedItem);
        }
        else {
            ControlCode(eControlCode::DrawingItem);
        }
        DrawString(item->name);

        if (item->onDraw) {
            ControlCode(eControlCode::CallingOnDraw);
            
            tInfo menuInfo = {
                .menu = menu,
                .item = item,
                .data = item->data,
                .index = i
            };
            item->onDraw(&menuInfo);
        }

        ControlCode(eControlCode::FinishedDrawingItem);
        ++i;
    } 
}


void tMenu::Update(eButton buttonPress)
{
    Navigate(buttonPress);

    ControlCode(eControlCode::ClearScreen);
    if (currentSub_) {
        Draw(currentSub_);
    }
}
