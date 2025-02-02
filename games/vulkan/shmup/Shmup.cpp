//
// Created by mgrus on 02.02.2025.
//

#include "Shmup.h"


void Shmup::update() {

}
void Shmup::init() {
  }

void Shmup::render() {

}


bool Shmup::shouldStillRun() {
  return true;
  }

std::vector<std::string> Shmup::getAssetFolder() {
    return {"assets"};
  }

bool Shmup::shouldAutoImportAssets() {
  return true;
 }


DefaultGame* getGame() {
    return new Shmup();
}
