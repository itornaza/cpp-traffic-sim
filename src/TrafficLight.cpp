#include <chrono>
#include <future>
#include <iostream>
#include <random>
#include <thread>

#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <class T> T MessageQueue<T>::receive() {
  // create the lock to perform messages modification
  std::unique_lock<std::mutex> uLock(_mutex);

  // pass the lock to the condition variable (it has to be a unique lock)
  _cond.wait(uLock, [this](){ return !_queue.empty(); });

  // Remove last element from queue
  T message = std::move(_queue.back());
  _queue.pop_back();

  // will not be copied due to return value optimization (RVO) in C++
  return message;
}

template <class T> void MessageQueue<T>::send(T &&message) {
  // perform messages modification under the lock
  std::lock_guard<std::mutex> uLock(_mutex);
  _queue.push_back(std::move(message));

  // Notify client after pushing new message into the queue
  _cond.notify_one();
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight() { 
  _currentPhase = TrafficLightPhase::red; 
  _phasesQueue = std::make_shared<MessageQueue<TrafficLightPhase>>();
}

void TrafficLight::waitForGreen() {
  while (true) {
    // sleep at every iteration to reduce CPU usage
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

    // just get the traffic light phase, without updating any state. The phase
    // is toggled within the simulate method.
    TrafficLightPhase phase = _phasesQueue->receive();
    if (phase == TrafficLightPhase::green) { return; }
  }
}

TrafficLightPhase TrafficLight::getCurrentPhase() { return _currentPhase; }

void TrafficLight::setCurrentPhase(TrafficLightPhase phase) {
  _currentPhase = phase;
}

void TrafficLight::simulate() {
  threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases() {
  // random number generator
  // https://stackoverflow.com/questions/7560114/random-number-c-in-some-range
  // http://www.cplusplus.com/reference/random/mersenne_twister_engine/
  std::random_device rd; // obtain a random number from hardware
  std::mt19937 eng(rd()); // seed the generator
  std::uniform_real_distribution<> distr(4.0, 6.0); // define the range

  // init stop watch
  std::chrono::time_point<std::chrono::system_clock> lastUpdate;
  lastUpdate = std::chrono::system_clock::now();

  while (true) {
    // sleep at every iteration to reduce CPU usage
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    // compute time difference to stop watch
    long timeSinceLastUpdate =  std::chrono::duration_cast<std::chrono::seconds>(
                                  std::chrono::system_clock::now() - lastUpdate)
                                .count();

    // toggle the state of the traffic light
    if (timeSinceLastUpdate >= distr(eng)) {
      if (_currentPhase == TrafficLightPhase::red) {
        _currentPhase = TrafficLightPhase::green;
      } else {
        _currentPhase = TrafficLightPhase::red;
      } // End inner if-else

      // the _currentPhase that is toggled after every cycle duration is sent 
      // to the queue.
      auto future = std::async(std::launch::async, 
                               &MessageQueue<TrafficLightPhase>::send,
                               _phasesQueue, 
                               std::move(_currentPhase));
      future.wait();

      // reset stop watch for next cycle
      lastUpdate = std::chrono::system_clock::now();
    } // End outer if
  } // End while
}
