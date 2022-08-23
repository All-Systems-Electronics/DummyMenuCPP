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
