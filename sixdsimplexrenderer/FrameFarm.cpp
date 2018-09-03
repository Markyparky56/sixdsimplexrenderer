#include "FrameFarm.hpp"
#include <sstream>
#include <iostream>

Channel<OutputFrame> *FrameFarm::outputChannel = nullptr;

void FrameFarm::Run(unsigned int NumWorkerThreads)
{
  std::vector<std::thread> workers(NumWorkerThreads);

  for (unsigned int i = 0; i < NumWorkerThreads; i++)
  {
    workers[i] = std::thread(std::mem_fun(&FrameFarm::WorkerThread), this);
  }
  unsigned int i = 0;
  for (auto &worker : workers)
  {
    std::stringstream ss;
    worker.join();
    ss << "Worker " << i+1 << " Joined" << std::endl;
    outputChannel->Write({ ss.str(), "", nullptr });
    i++;
  }
}

void FrameFarm::WorkerThread()
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
    outputChannel->Write({ ss.str(), "", nullptr });
  }
  workerIdPool.ReturnID(id);
}
