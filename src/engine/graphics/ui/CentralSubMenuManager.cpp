//
// Created by mgrus on 02.04.2025.
//

#include "CentralSubMenuManager.h"

#include <iostream>
#include <ostream>

#include "Menu.h"

void CentralSubMenuManager::registerMenuBar(std::shared_ptr<MenuBar> menuBar) {
    menu_bars_.push_back(menuBar);
}

void CentralSubMenuManager::onMessage(const UIMessage &message) {
    if (message.type != MessageType::WidgetGainedFocus) return;

    auto menuWidget = std::dynamic_pointer_cast<Menu>(message.focusMessage.widget);
    if (!menuWidget) return;

    std::cout << "menu got focus: " << menuWidget->getId() << std::endl;

    for (auto openSub : open_sub_menus_) {
        // We go through all known submenus which are open now.
        // We need to check if the latest opened menu is in any relation with the current
        // "open sub menu" from the list.
        // If it is, we assume it is ok to leave this submenu open.
        // Otherwise, we can close this submenu.
        bool newIsTheParentOfOpenSub = openSub->parentMenu().lock() == menuWidget;
        bool newIsTheSameAsOpenSub = openSub == menuWidget;
        bool newIsChildOfOpenSub = openSub->isWidgetAChild(menuWidget);
        bool openSubIsChildOfNew = menuWidget->isWidgetAChild(openSub);
        bool openSubIsAncestorOfNew = openSub->isAncestorOf(menuWidget);

        std::cout << "openSub: " << openSub->getId() << std::endl;
        std::cout << "\tchecks: newIsParentOfOpenSub: " << newIsTheParentOfOpenSub << std::endl;
        std::cout << "\tchecks: newIsTheSameAsOpenSub: " << newIsTheSameAsOpenSub << std::endl;
        std::cout << "\tchecks: newIsChildOfOpenSub: " << newIsChildOfOpenSub << std::endl;
        std::cout << "\tchecks: openSubIsChildOfNew: " << openSubIsChildOfNew << std::endl;
        std::cout << "\tchecks: openSubIsAncestorOfNew" << openSubIsAncestorOfNew << std::endl;

        if (!newIsTheParentOfOpenSub && !newIsTheSameAsOpenSub && !newIsChildOfOpenSub  && !openSubIsChildOfNew
            && !openSubIsAncestorOfNew) {
            std::cout << "closing submenupanel for: " << openSub->getId() << std::endl;
            openSub->closeSubMenuPanel();
        }

    }

}

void CentralSubMenuManager::registerSubMenuHolder(std::shared_ptr<Menu> menu) {
    std::cout << "new submenu holder registered: " << menu->getId() << std::endl;
    open_sub_menus_.insert(menu);
}
