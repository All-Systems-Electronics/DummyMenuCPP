#pragma once
#include <cstdint>

class tMenu {
public:
    struct tSub;
    struct tItem;

    struct tInfo {
        const tSub *menu;
        const tItem *item;
        void *data;
        int index;
    };

    // Place this function in the "onExecute" callback, and the address of the target menu in the data pointer,
    // to navigate to a submenu.
    static void OnExecuteSubMenu(const tInfo *) {}

    // The actual menu item.
    // Holds the name of the item, and function pointers for when the item is displayed
    // and if the user selects the item.
    // The data pointer can point to any piece of data, and will be passed into the execute and draw
    // functions.
    struct tItem{
        const char *name;
        void (*onExecute)(const tInfo *menuInfo);
        void (*onDraw)(const tInfo *menuInfo);
        void *data;
    };

    // The actual menu (or submenu).
    // Holds the name of the menu, and the array of menu items.
    // The index of the top level menu needs to be set to 0,
    // and the previous pointer to NULL.
    // Notice that the addition of the "previous" pointer makes our menu
    // act a bit like a linked list, so we can dynamically move up and down in
    // the menu structure.
    struct tSub{
        const char *name;   // Leave this NULL to skip drawing the menu name.
        const tItem *items;
        int index;
        struct tSub *previous;
    };

    // An enumeration for separating menu button presses from the actual key strokes used
    // to navigate the menu.
    enum class eButton : uint8_t {
        None,
        Up,
        Down,
        Left,
        Right
    };

    tMenu() = default;
    tMenu(tSub* topMenu);
    virtual ~tMenu() = 0;
    void Update(eButton buttonPress);
    bool ExitAttempted() const
    {
        return exitAttempted_;
    }
protected:
    // MenuClearScreen needs to be defined somewhere in the calling code.
    enum eControlCode{
        ClearScreen,
        FinishedDrawingItem,
        DrawingSelectedItem,
        DrawingItem,
        CallingOnDraw
    };
    virtual void ControlCode(eControlCode) = 0;
    // DrawString may be called multiple times to draw each line of the menu.
    virtual void DrawString(const char*) = 0;
private:
    tSub* currentSub_ {};
    bool exitAttempted_ {};

    void Navigate(eButton buttonPress);
    void Draw(const tSub *menu);
};

inline tMenu::tMenu(tSub* topMenu):
    currentSub_(topMenu)
{
    currentSub_->index = 0;
    currentSub_->previous = nullptr;
}

inline tMenu::~tMenu()
{
    
}

// Handles the drawing, navigating and updating of the menu.
inline void tMenu::Navigate(eButton buttonPress)
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

inline void tMenu::Draw(const tSub *menu)
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


inline void tMenu::Update(eButton buttonPress)
{
    Navigate(buttonPress);

    ControlCode(eControlCode::ClearScreen);
    if (currentSub_) {
        Draw(currentSub_);
    }
}