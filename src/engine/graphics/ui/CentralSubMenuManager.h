//
// Created by mgrus on 02.04.2025.
//

#ifndef CENTRALSUBMENUMANAGER_H
#define CENTRALSUBMENUMANAGER_H

#include <memory>
#include <engine/graphics/ui/MenuBar.h>

/**
* This class ensures that only one submenu is every opened at the same time,
* over all menus/bars.
*/
class CentralSubMenuManager {

  public:
    void registerMenuBar(std::shared_ptr<MenuBar> menuBar);

    void onMessage(const UIMessage& message);

    void registerSubMenuHolder(std::shared_ptr<Menu> menu);

private:
    std::vector<std::shared_ptr<MenuBar>> menu_bars_;
    std::shared_ptr<Menu> last_open_menu_;
    std::unordered_set<std::shared_ptr<Menu>> open_sub_menus_;
};



#endif //CENTRALSUBMENUMANAGER_H
