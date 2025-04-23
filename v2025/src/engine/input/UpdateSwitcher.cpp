//
// Created by mgrus on 15.02.2025.
//

#include "UpdateSwitcher.h"
#include <stdexcept>
#include <engine/input/Input.h>

UpdateSwitcher::UpdateSwitcher(const std::vector<Updatable *> &sceneNodes, int triggerKey) : _nodes(sceneNodes), _triggerKey(triggerKey) {
    if (!sceneNodes.empty()) {
        _currentUpdatable = sceneNodes[0];
    } else {
        throw std::invalid_argument("No updatable found within list, at least 2 should be present)");
    }

}

void UpdateSwitcher::poll() {
    static int index = 0;
    if (Input::getInstance()->wasKeyPressed(_triggerKey)) {
        index++;
        if (index >= _nodes.size()) {
            index = 0;
        }
        _currentUpdatable = _nodes[index];
    }
}

void UpdateSwitcher::update() {
    // First check if we shall switch:
    poll();

    // Then do the actual update
    if (_currentUpdatable) {
        _currentUpdatable->update();
    }

}

void UpdateSwitcher::addTarget(Updatable *target) {
    _nodes.push_back(target);
}

void UpdateSwitcher::removeTarget(Updatable *target) {
    _nodes.erase(std::find(_nodes.begin(), _nodes.end(), target));
}
