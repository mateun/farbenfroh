//
// Created by mgrus on 25.03.2025.
//

#ifndef FPSCOUNTER_H
#define FPSCOUNTER_H



class FpsCounter {
  public:
    void update();

    float frameTimeInMicroSecondsAvg();
    float lastFPSAverage();

private:
      int currentFrame = 0;
      float ftMicrosAvg =0;
      float ftSecondsAvg = 0;
      float lastAvgFPS = 0;

};



#endif //FPSCOUNTER_H
