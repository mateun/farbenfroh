//
// Created by mgrus on 27.03.2025.
//

#ifndef EDITORLAUNCHER_H
#define EDITORLAUNCHER_H

#include <engine/graphics/Application.h>


class EditorLauncher : public Application    {

  public:
    EditorLauncher(int width, int height, bool fullscreen);

    void onCreated() override;

};



#endif //EDITORLAUNCHER_H
