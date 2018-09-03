#pragma once
#include <memory>
#include <mutex>
#include "Channel.hpp"

static std::mutex outputMutex;

// Base Thread Task Class
class ThreadTask
{
public:
  ThreadTask()
    : args(nullptr)
  {
    id = TaskCount++;
  }
  virtual ~ThreadTask() {}

  struct TaskArgs
  {
    Channel<std::string> *outputChannel;
  };
  inline void SetArgs(TaskArgs *InArgs) { args = InArgs; }

  virtual void Run() = 0;

protected:
  static unsigned int TaskCount;
  unsigned int id;
  TaskArgs *args;
};
