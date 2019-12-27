#include "TrafficLight.h"

#include <chrono>
#include <future>
#include <iostream>
#include <random>
#include <thread>

/* Implementation of class "MessageQueue" */

template <class T> T MessageQueue<T>::receive() {
  // FP.5a : The method receive should use std::unique_lock<std::mutex> and
  // _condition.wait()
  // to wait for and receive new messages and pull them from the queue using
  // move semantics.
  // The received object should then be returned by the receive function.

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
  // FP.4a : The method send should use the mechanisms
  // std::lock_guard<std::mutex>
  // as well as _condition.notify_one() to add a new message to the queue and
  // afterwards send a notification.

  // perform messages modification under the lock
  std::lock_guard<std::mutex> uLock(_mutex);
  _queue.push_back(std::move(message));

  // Notify client after pushing new message into the queue
  _cond.notify_one();
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight() { 
  _currentPhase = TrafficLightPhase::red; 
  _message_queue = std::make_shared<MessageQueue<TrafficLightPhase>>();
}

void TrafficLight::waitForGreen() {
  // FP.5b : add the implementation of the method waitForGreen, in which an
  // infinite while-loop runs and repeatedly calls the receive function on the
  // message queue. Once it receives TrafficLightPhase::green, the method
  // returns.

  // The while loop is not needed in this version because we only enter this
  // method when the traffic light is red and the state of the traffic lights
  // is just toggled. However, it is used here to work with future extensions 
  // that would might use an amber light as well.
  while (true) {
    // sleep at every iteration to reduce CPU usage
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

    TrafficLightPhase phase = _message_queue->receive();
    setCurrentPhase(phase);
    if (phase == TrafficLightPhase::green) { return; }
  }
}

TrafficLightPhase TrafficLight::getCurrentPhase() { return _currentPhase; }

void TrafficLight::setCurrentPhase(TrafficLightPhase phase) {
  _currentPhase = phase;
}

void TrafficLight::simulate() {
  // FP.2b : Finally, the private method „cycleThroughPhases“ should be
  // started in a thread when the public method „simulate“ is called. To do
  // this, use the thread queue in the base class.
  threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases() {
  // FP.2a : Implement the function with an infinite loop that measures the time
  // between two loop cycles and toggles the current phase of the traffic light
  // between red and green and sends an update method to the message queue using
  // move semantics. The cycle duration should be a random value between 4 and 6
  // seconds. Also, the while-loop should use std::this_thread::sleep_for to
  // wait 1ms between two cycles.

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
    if (timeSinceLastUpdate >= 4 && timeSinceLastUpdate <= 6) {
      if (_currentPhase == TrafficLightPhase::red) {
        _currentPhase = TrafficLightPhase::green;
      } else {
        _currentPhase = TrafficLightPhase::red;
      } // End inner if-else

      TrafficLightPhase message = getCurrentPhase();
      auto future = std::async(std::launch::async, 
                               &MessageQueue<TrafficLightPhase>::send,
                               _message_queue, 
                               std::move(message));
      future.wait();

      // reset stop watch for next cycle
      lastUpdate = std::chrono::system_clock::now();
    } // End outer if
  } // End while
}
