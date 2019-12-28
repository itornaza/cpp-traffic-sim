#ifndef TRAFFICLIGHT_H
#define TRAFFICLIGHT_H

#include <condition_variable>
#include <deque>
#include <mutex>

#include "TrafficObject.h"

// forward declarations to avoid include cycle
class Vehicle;
enum TrafficLightPhase { red, green };

/* Declaration of class "MessageQueue" */

template <class T> class MessageQueue {
public:
  T receive();
  void send(T &&);

private:
  std::mutex _mutex;
  std::condition_variable _cond;
  std::deque<T> _queue;
};

/* Declaration of class "TrafficLight" */

class TrafficLight : public TrafficObject {
public:
  // constructor / desctructor
  TrafficLight();

  // getters / setters
  TrafficLightPhase getCurrentPhase();
  void setCurrentPhase(TrafficLightPhase);

  // typical behaviour methods
  void waitForGreen();
  void simulate() override;

private:
  // typical behaviour methods
  void cycleThroughPhases();

  std::shared_ptr<MessageQueue<TrafficLightPhase>> _phasesQueue;
  TrafficLightPhase _currentPhase;
  std::condition_variable _condition;
  std::mutex _mutex;
};

#endif
