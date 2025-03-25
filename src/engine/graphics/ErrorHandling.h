//
// Created by mgrus on 25.03.2025.
//

#ifndef ERRORHANDLING_H
#define ERRORHANDLING_H

#include <GL\glew.h>
#include <iostream>

inline void GL_ERROR_EXIT(GLint code, std::string msg = "") {
  auto err = glGetError();
    if (err != 0) {
      if (!msg.empty()) {
          std::cerr << msg << std::endl;
      }
        exit(code);
    }
}


#endif //ERRORHANDLING_H
