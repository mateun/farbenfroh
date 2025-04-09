//
// Created by mgrus on 27.03.2025.
//

#ifndef EDITORLAUNCHER_H
#define EDITORLAUNCHER_H

#include <engine/graphics/Application.h>
#include <engine/graphics/ui/LabelWidget.h>


class EditorLauncher : public Application, public FrameMessageSubscriber, public std::enable_shared_from_this<FrameMessageSubscriber>   {

  public:
    EditorLauncher(int width, int height, bool fullscreen);

    void makeLegacyUI();

    void onCreated() override;

    void doFrame() override;

    std::shared_ptr<LabelWidget> lblMouseCoords;
    std::shared_ptr<LabelWidget> lblFrameTime;

    void onFrameMessages(const std::vector<RawWin32Message> &msgs) override;
};



#endif //EDITORLAUNCHER_H
