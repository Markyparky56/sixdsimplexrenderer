#include "FastNoise.h"
#include "png.hpp"
#include <boost/filesystem.hpp>
#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>
#include "ColourGradient.hpp"
#include "FrameFarm.hpp"
#include "GenerateFrameTask.hpp"
#include <memory>

//const auto PI = 3.14159265358979323846264338327950288419716939937510582097494459230781640628620899;
bool running;
pThread outputThread;
std::unique_ptr<ThreadFarm> farm;
std::unique_ptr<Channel<OutputFrame>> outputChannel;

void OutputThread()
{
  while (running)
  {
    auto output = outputChannel->Read();
    if (!output.msg.empty())
    {
      std::cout << output.msg;
    }
    if (output.frame != nullptr)
    {
      output.frame->write(output.framename);
      delete output.frame;
    }
  }
  // Clear the buffer before exiting
  while(!outputChannel->BufferEmpty())
  {
    auto output = outputChannel->Read();
    if (!output.msg.empty())
    {
      std::cout << output.msg;
    }
    if (output.frame != nullptr)
    {
      output.frame->write(output.framename);
      delete output.frame;
    }
  }
}

int main(int argc, char* argv[])
{
  const uint32_t framerate = 30; 
  const uint32_t videoLength = 15;
  const uint32_t numFrames = framerate * videoLength;
  const float startX = -25.71f
            , startY = 874.32f
            , startZ = -90.13f
            , startW = 756.21f
            , startV = 0.1234f
            , startU = 123.45f;

  // Some what akin to 16:9 radiuses, not sure what z should be since there is no 3D equivalent
  // I suspect zRadius will affect the variation between frames?
  const float xRadius = 54;
  const float yRadius = 54;
  const float zRadius = 54;
  const uint32_t dimW = 1080;
  const uint32_t dimH = 1080;

  std::unique_ptr<FastNoise> noise = std::make_unique<FastNoise>(42);
  noise->SetFrequency(0.005f);
  noise->SetFractalGain(0.5f);
  noise->SetFractalLacunarity(2.0f);
  noise->SetFractalOctaves(2);
  noise->SetFractalType(FastNoise::FractalType::FBM);

  // Setup a nice colour gradient
  std::unique_ptr<ColourGradient> gradient = std::make_unique<ColourGradient>();
  //gradient.AddGradientPoint(0.0  , { 0  , 64 , 64, 255 });
  //gradient.AddGradientPoint(0.333, { 0  , 124, 64 , 255 });
  //gradient.AddGradientPoint(0.5  , { 0  , 255, 124, 255 });
  //gradient.AddGradientPoint(0.666, { 0  , 124, 255, 255 });
  //gradient.AddGradientPoint(0.900, { 0  , 255, 255, 255 });
  //gradient.AddGradientPoint(1.0  , { 255, 255, 255, 255 });
  gradient->AddGradientPoint(0.0, { 64, 0, 64, 255 });
  gradient->AddGradientPoint(0.25, { 128, 0, 128, 255 });
  gradient->AddGradientPoint(0.33, { 255, 0, 128, 255 });
  gradient->AddGradientPoint(0.44, { 128, 0, 255, 255 });
  gradient->AddGradientPoint(0.66, { 0, 255, 255, 255 });
  gradient->AddGradientPoint(0.75, { 0, 0, 255, 255 });
  gradient->AddGradientPoint(0.88, { 255, 0, 255, 255 });
  gradient->AddGradientPoint(1.0, { 255, 255, 255, 255 });

  // Create a directory to store our rendered images in
  std::stringstream ss;
  auto t = std::time(nullptr);
  auto tm = *std::localtime(&t);
  ss << std::put_time(&tm, "%Y-%m-%d-%H-%M-%S");
  boost::filesystem::create_directory(ss.str());

  // Move into the new directory
  boost::filesystem::current_path(ss.str());

  // Workout how many threads we can use
  unsigned int numThreads = std::thread::hardware_concurrency();
  if (numThreads > 2) numThreads -= 2; // Reserve some threads so the computer doesn't lockup

  // Setup ThreadFarm
  farm = std::make_unique<FrameFarm>();
  outputChannel = std::make_unique<Channel<OutputFrame>>();
  FrameFarm::outputChannel = outputChannel.get();

  // Start output thread
  running = true;
  outputThread = pThread(new std::thread(&OutputThread));

  // Queue up tasks
  for (uint32_t f = 0; f < numFrames; f++)
  {
    GenerateFrameTask *task = new GenerateFrameTask;
    GenerateFrameTask::Args *args = new GenerateFrameTask::Args(noise.get(), gradient.get()
                                                              , startX, startY, startZ, startW, startV, startU
                                                              , xRadius, yRadius, zRadius
                                                              , f, numFrames
                                                              , dimW, dimH);
    args->outputChannel = outputChannel.get();
    task->SetArgs(args);
    farm->PushNewTask(task);
  }
  farm->Run(numThreads);
  running = false;
  outputThread->join();

  /*
  // Setup our png image which we'll fill and save for each frame

  png::image<png::rgba_pixel> frame(dimW, dimH);

  // Loop through each frame, filling it with values
  for (uint32_t f = 0; f < numFrames; f++)
  {
    // Track min/max values to normalise values
    float min = 9999999.f; 
    float max = -9999999.f;

    std::vector<float> valueBuffer(dimW*dimH);

    for (uint32_t y = 0; y < dimH; y++)
    {
      for (uint32_t x = 0; x < dimW; x++)
      {
        float Nx = static_cast<float>(x) / static_cast<float>(dimW);
        float Ny = static_cast<float>(y) / static_cast<float>(dimH);
        float Nz = static_cast<float>(f) / static_cast<float>(numFrames);
        float rdx = Nx*2*static_cast<float>(PI);
        float rdy = Ny*2*static_cast<float>(PI);
        float rdz = Nz*2*static_cast<float>(PI);

        // Calculate point to evaluate in noise
        float vX = xRadius * std::sinf(rdx);
        float vY = xRadius * std::cosf(rdx);
        float vZ = yRadius * std::sinf(rdy);
        float vW = yRadius * std::cosf(rdy);
        float vV = zRadius * std::sinf(rdz);
        float vU = zRadius * std::cosf(rdz);

        float value = noise.GetSimplexFractal(vX, vY, vZ, vW, vV, vU);
        valueBuffer[x + (y * dimW)] = value;
        min = (min < value) ? min : value;
        max = (max > value) ? max : value;
      }
    }

    // Normalise values
    //float norm, temp;
    //if (min < 0) temp = min * -1;
    //if (temp > max) norm = 1 / temp;
    //else norm = 1 / max;

    float norm = ((1 / (min*-1)) < (1 / max)) ? (1 / (min*-1)) : (1 / max);

    for (auto &v : valueBuffer)
    {
      v = (v - min) / (max - min);
      //v = ((v + 0.5f) * 0.5f);
    }

    // Insert values into image
    for (uint32_t y = 0; y < dimH; y++)
    {
      for (uint32_t x = 0; x < dimW; x++)
      {
        // Convert float value to 0-255 range
        //auto value = ((valueBuffer[x + (y * dimW)] + 0.5f) * 0.5f);
        auto value = valueBuffer[x + (y*dimW)];
        //if (value > 1 || value < -1) std::cout << value << std::endl;

        //uint32_t pixelValue = static_cast<png::byte>(value * 255.f);
        //frame[y][x] = png::rgba_pixel(pixelValue, pixelValue, pixelValue, 255);

        auto colour = gradient.GetColour(value);
        frame[y][x] = png::rgba_pixel(colour.r, colour.g, colour.b, 255);
      }
    }

    // Save the frame
    std::stringstream filename;
    filename << f << ".png";
    frame.write(filename.str());

    std::cout << filename.str() << " written to disk" << std::endl;
  }*/

  return 0;
}
