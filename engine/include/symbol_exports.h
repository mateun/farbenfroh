//
// Created by mgrus on 23.04.2025.
//

#ifndef SYMBOL_EXPORTS_H
#define SYMBOL_EXPORTS_H

#ifdef ENGINE_EXPORTS
#  define ENGINE_API __declspec(dllexport)
#else
#  define ENGINE_API __declspec(dllimport)
#endif

#endif //SYMBOL_EXPORTS_H
