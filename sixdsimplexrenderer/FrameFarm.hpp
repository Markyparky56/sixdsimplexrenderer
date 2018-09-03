#pragma once
#include "ThreadFarm.hpp"
#include "png.hpp"

struct OutputFrame {
  std::string msg;
  std::string framename;
  png::image<png::rgba_pixel> *frame;
};

class FrameFarm : public ThreadFarm
{
public:
  FrameFarm()
    : ThreadFarm()
  {}

  void Run(unsigned int NumWorkerThreads) override;

  static Channel<OutputFrame> *outputChannel;

protected:
  void WorkerThread() override;
};
