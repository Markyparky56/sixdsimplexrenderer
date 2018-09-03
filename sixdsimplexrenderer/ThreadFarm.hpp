#pragma once
#include <thread>
#include <vector>
#include <memory>
#include <queue>
#include <mutex>
#include <string>
#include "IdPool.hpp"
#include "ThreadTask.hpp"
#include "Channel.hpp"
using pThread = std::unique_ptr<std::thread>;

// Simple Thread Farm
// Creates a specified number of worker threads
// Queue taks which are allocated to worker threads

class ThreadFarm
{
public:
  ThreadFarm();
  ~ThreadFarm();

  virtual void Run(unsigned int NumWorkerThreads);
  virtual void PushNewTask(ThreadTask *InTask);

  static Channel<std::string> *outputChannel;

protected:
  virtual void WorkerThread();
  IdPool workerIdPool;
  std::queue<ThreadTask*> taskQueue;
  std::mutex taskQueueMutex;
};