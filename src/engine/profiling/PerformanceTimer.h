//
// Created by mgrus on 15.03.2025.
//

#ifndef PERFORMANCETIMER_H
#define PERFORMANCETIMER_H

class PerformanceTimer {

public:
    explicit PerformanceTimer(bool startImmediately = false);
    void start();
    void stop();
    float durationInSeconds() const;

private:
    LARGE_INTEGER startTime;
    LARGE_INTEGER endTime;
    float diffInSeconds = 0.0f;

};



#endif //PERFORMANCETIMER_H
