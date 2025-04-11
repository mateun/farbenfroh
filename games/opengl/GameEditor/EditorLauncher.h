//
// Created by mgrus on 27.03.2025.
//

#ifndef EDITORLAUNCHER_H
#define EDITORLAUNCHER_H

#include <engine/graphics/Application.h>
#include <engine/graphics/ui/LabelWidget.h>
#include <engine/graphics/ui/TreeViewer.h>


class EditorLauncher : public Application, public FrameMessageSubscriber, public std::enable_shared_from_this<FrameMessageSubscriber>   {

  public:
    EditorLauncher(int width, int height, bool fullscreen);

    void makeLegacyUI();

    void showNewProjectDialog(std::shared_ptr<Widget>);



    void onCreated() override;

    void doFrame() override;

    std::shared_ptr<LabelWidget> lblMouseCoords;
    std::shared_ptr<LabelWidget> lblFrameTime;
    std::shared_ptr<MenuBar> main_menu_bar_;
    std::wstring project_folder_ = L"";
    bool show_dir_selector_ = false;
    std::shared_ptr<TreeModel> game_object_tree_model_;

    void onFrameMessages(const std::vector<RawWin32Message> &msgs) override;
};



#endif //EDITORLAUNCHER_H
