#include "ThreadFarm.hpp"
#include <iostream>
#include <functional>
#include <sstream>

Channel<std::string> *ThreadFarm::outputChannel = nullptr;
unsigned int ThreadTask::TaskCount = 0;

ThreadFarm::ThreadFarm()
  : workerIdPool(16)
{}

ThreadFarm::~ThreadFarm()
{}

void ThreadFarm::Run(unsigned int NumWorkerThreads)
{
  std::vector<std::thread> workers(NumWorkerThreads);
  
  for (unsigned int i = 0; i < NumWorkerThreads; i++)
  {
    workers.push_back(std::thread(std::mem_fun(&ThreadFarm::WorkerThread), this));
  }
  for (auto &worker : workers)
  {
    worker.join();
  }
}

void ThreadFarm::PushNewTask(ThreadTask *InTask)
{
  taskQueue.push(InTask);
}

void ThreadFarm::WorkerThread()
{
  ThreadTask *task;
  int id = workerIdPool.GetNextID();
  while (true)
  {
    // Grab a task
    {
      taskQueueMutex.lock();
      if (!taskQueue.empty())
      {
        task = taskQueue.front();
        taskQueue.pop();
        taskQueueMutex.unlock();
      }
      else
      {
        // End this worker thread since we're out of tasks
        taskQueueMutex.unlock();
        break;
      }
    }
    // Execute it
    task->Run();
    // Delete it
    delete task;
  }
  {
    std::stringstream ss;
    ss << "Worker " << id << " finished" << std::endl;
    std::unique_lock<std::mutex> lock(outputMutex);
    outputChannel->Write(ss.str());
  }
  workerIdPool.ReturnID(id);
}
