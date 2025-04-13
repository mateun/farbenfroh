//
// Created by mgrus on 13.04.2025.
// Describes the API of the commandline REPL for the game-shell.
// We have the following basic architecture:
// - cmd-repl running in a standalone win32 process
// - the game-engine running in a separate process (or at least in a child-hwnd, to be defined)
// - game-engine receives command from the repl via tcp.
//

#ifndef CMD_REPL_H
#define CMD_REPL_H
#include <cinttypes>
#include <string>

/**
* A Repl_Message is sent from the repl to the engine server.
*/
struct Repl_Message {
  uint8_t id;
  std::string payload;

};

#endif //CMD_REPL_H
