//
// Created by mgrus on 25.03.2025.
//

#ifndef TIMING_H
#define TIMING_H



class Timing {
  friend class Application;
  public:
    static float lastFrameTimeInSeconds();

    private:
      inline static float frameTimeSecs = 0;

};



#endif //TIMING_H
