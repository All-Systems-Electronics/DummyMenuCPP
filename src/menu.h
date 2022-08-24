#pragma once
#include <cstdint>
#include <cstddef>
#include <array>
#include <experimental/array>

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

    using fCallback = void (const tInfo& menuInfo);

    // Place this function in the "onExecute" callback, and the address of the target menu in the data pointer,
    // to navigate to a submenu.
    static void OnExecuteSubMenu(const tInfo&) {}

    // The actual menu item.
    // Holds the name of the item, and function pointers for when the item is displayed
    // and if the user selects the item.
    // The data pointer can point to any piece of data, and will be passed into the execute and draw
    // functions.
    struct tItem{
        const char *name;
        fCallback* onExecute;
        fCallback* onDraw;
        void *data;
    };

    // The actual menu (or submenu).
    // Holds the name of the menu, and the array of menu items.
    // The index of the top level menu needs to be set to 0,
    // and the previous pointer to NULL.
    // Notice that the addition of the "previous" pointer makes our menu
    // act a bit like a linked list, so we can dynamically move up and down in
    // the menu structure.
    class tSub{
    private:
    friend class tMenu;
        tSub(const char* name, const tItem *items):
            name(name),
            items(items)
        {
        }
        const char *name {};   // Leave this NULL to skip drawing the menu name.
        const tItem *items {};
        int index {};
        struct tSub *previous {};
    };
    static tSub MakeSubMenu(const char* name, const tItem *items)
    {
        return tSub(name, items);
    }

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
    virtual uint64_t TimeMS() const
    {
        return 0;
    }
    virtual uint64_t UpdatePeriodMS() const
    {
        return 0;
    } 
private:
    uint64_t lastMS_ {};
    tSub* currentSub_ {};
    bool exitAttempted_ {};
    bool refreshRequested_ {true};

    void Navigate(eButton buttonPress);
    void Draw(const tSub *menu);
};

#if (__cplusplus >= 201703L)
// If possible, use this in the following way, without the size parameter.
// Note that it should be either constexpr or const so its stored in flash.
// Prefer constexpr. However, if some of the menu items call functions that are not able to be called from constexpr, then declare it static const instead.
//static constexpr auto MenuItems = MakeMenu (
//{
//    {"Item Name", OnExecute, OnDraw, nullptr},
//    {nullptr,     nullptr,   nullptr,nullptr}
//});
template <std::size_t N>
constexpr std::array<std::remove_cv_t<const tMenu::tItem>, N> MakeMenuItems(const tMenu::tItem (&&a)[N])
{
    return std::experimental::__to_array(std::move(a), std::make_index_sequence<N>{});
}

// Use this to create an empty menu in SRAM that can be dynamically modified as needed.
template <std::size_t N>
constexpr std::array<std::remove_cv_t<const tMenu::tItem>, N> MakeMenuItemsEmpty()
{
    return MakeMenuItems<N>( {
        {nullptr,	nullptr,	nullptr,	nullptr},
    });
}
#endif

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

    if (buttonPress != eButton::None) {
        refreshRequested_ = true;
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
                    item->onExecute(menuInfo);
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
            item->onDraw(menuInfo);
        }

        ControlCode(eControlCode::FinishedDrawingItem);
        ++i;
    } 
}

inline void tMenu::Update(eButton buttonPress)
{
    const auto currentMS = TimeMS();
    const auto updateMS = UpdatePeriodMS();
    if (updateMS > 0) {
        if ((currentMS - lastMS_) >= updateMS) {
            lastMS_ = currentMS;
            refreshRequested_ = true;
        }
    }

    Navigate(buttonPress);

    if (refreshRequested_ && currentSub_) {
        ControlCode(eControlCode::ClearScreen);
        refreshRequested_ = false;
        Draw(currentSub_);
    }
}