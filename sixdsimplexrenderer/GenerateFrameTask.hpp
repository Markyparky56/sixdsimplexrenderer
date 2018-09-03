#pragma once
#include "ThreadTask.hpp"
#include "ColourGradient.hpp"
#include "png.hpp"
#include "FastNoise.h"
#include <sstream>

const auto PI = 3.14159265358979323846264338327950288419716939937510582097494459230781640628620899;

class GenerateFrameTask : public ThreadTask
{
public:
  struct Args : public TaskArgs
  {
    Args(const FastNoise *noise
      ,  ColourGradient *gradient
      ,  const double startX, const double startY, const double startZ, const double startW, const double startV, const double startU
      ,  const double xRadius, const double yRadius, const double zRadius
      ,  const uint32_t frame, const uint32_t numFrames
      ,  const uint32_t dimW, const uint32_t dimH)
      : noise(noise)
      , gradient(gradient)
      , startX(startX), startY(startY), startZ(startZ), startW(startW), startV(startV), startU(startU)
      , xRadius(xRadius), yRadius(yRadius), zRadius(zRadius)
      , frame(frame), numFrames(numFrames)
      , dimW(dimW), dimH(dimH)
    {}
    const double startX, startY, startZ, startW, startV, startU;
    const double xRadius, yRadius, zRadius;
    const uint32_t frame, numFrames;
    const uint32_t dimW, dimH;
    const FastNoise *noise;
    ColourGradient *gradient;
    Channel<OutputFrame> *outputChannel;
  };

  inline void Run() override
  {
    assert(args != nullptr);
    Args *taskArgs = static_cast<Args*>(args);
    assert(taskArgs->dimW > 0 && taskArgs->dimH > 0);
    assert(taskArgs->numFrames > 0);
    
    png::image<png::rgba_pixel> *frame = new png::image<png::rgba_pixel>(taskArgs->dimW, taskArgs->dimH);
    
    // Track min/max values to normalise values
    double min = 9999999.0;
    double max = -9999999.0;

    std::vector<double> valueBuffer(taskArgs->dimW*taskArgs->dimH);

    for (uint32_t y = 0; y < taskArgs->dimH; y++)
    {
      for (uint32_t x = 0; x < taskArgs->dimW; x++)
      {
        double s = static_cast<double>(x) / static_cast<double>(taskArgs->dimW);
        double t = static_cast<double>(y) / static_cast<double>(taskArgs->dimH);
        double u = static_cast<double>(taskArgs->frame) / static_cast<double>(taskArgs->numFrames);
        double s2PI = s * 2.0 * PI;
        double t2PI = t * 2.0 * PI;
        double u2PI = u * 2.0 * PI;       

        // Calculate point to evaluate in noise
        double vX = taskArgs->xRadius * std::sin(s2PI);
        double vY = taskArgs->xRadius * std::cos(s2PI);
        double vZ = taskArgs->yRadius * std::sin(t2PI);
        double vW = taskArgs->yRadius * std::cos(t2PI);
        double vV = taskArgs->zRadius * std::sin(u2PI);
        double vU = taskArgs->zRadius * std::cos(u2PI);

        double value = taskArgs->noise->GetSimplexFractal(vX, vY, vZ, vW, vV, vU);
        valueBuffer[(y * taskArgs->dimW) + x] = value;
        min = (min < value) ? min : value;
        max = (max > value) ? max : value;
      }
    }

    // Insert values into image
    for (uint32_t y = 0; y < taskArgs->dimH; y++)
    {
      for (uint32_t x = 0; x < taskArgs->dimW; x++)
      {
        // Convert float value to 0-255 range
        auto value = valueBuffer[(y * taskArgs->dimW) + x];
        value = (value - min) / (max - min);
        auto colour = taskArgs->gradient->GetColour(value);
        (*frame)[y][x] = png::rgba_pixel(colour.r, colour.g, colour.b, 255);
        //auto iV = static_cast<uint8>(value * 255.0);
        //(*frame)[y][x] = png::rgba_pixel(iV, iV, iV, 255);
      }
    }

    // Save the frame
    std::stringstream filename;
    filename << taskArgs->frame << ".png";

    std::stringstream output;
    output << "Task ID: " << id << " completed, " << filename.str() << " written to disk" << std::endl;
    std::unique_lock<std::mutex> lock(outputMutex);
    taskArgs->outputChannel->Write({ output.str(), filename.str(), frame });
  }
};
