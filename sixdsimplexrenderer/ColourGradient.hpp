#pragma once
#include <cstdint>
#include <vector>
#include <mutex>

using uint8 = std::uint8_t;

struct Colour
{
public:
  Colour() : r(0), g(0), b(0), a(0) {};
  Colour(uint8 r, uint8 g, uint8 b, uint8 a) : r(r), g(g), b(b), a(a) {}

  uint8 r, g, b, a;
};

struct GradientPoint
{
  double pos;
  Colour colour;
};

class ColourGradient
{
public:
  ColourGradient();

  void AddGradientPoint(double gradientPos, const Colour &colour);
  void Clear();

  const Colour& GetColour(double gradientPos);

private:
  std::mutex lookupMutex;
  int FindInsertionPos(double gradientPos);
  void InsertAtPos(int insertionPos, double gradientPos, const Colour &colour);
  int gradientPointCount;
  //mutable Colour workingColour;
  std::vector<GradientPoint> gradientPoints;

  uint8 LinearBlend(const uint8 channel0, const uint8 channel1, double t) const
  {
    double c0 = static_cast<double>(channel0) / 255.0;
    double c1 = static_cast<double>(channel1) / 255.0;
    return static_cast<uint8>(((c1 * t) + (c0 * (1.0 - t))) * 255.0);
  }

  Colour LinearInteropColour(const Colour &colour0, const Colour &colour1, double t) const
  {
    Colour out;
    out.r = LinearBlend(colour0.r, colour1.r, t);
    out.g = LinearBlend(colour0.g, colour1.g, t);
    out.b = LinearBlend(colour0.b, colour1.b, t);
    out.a = LinearBlend(colour0.a, colour1.a, t);
    return out;
  }

  //uint8 CubicBlend(const uint8 channel0, const uint8 channel1, const uint8 channel2, const uint8 channel3, double t) const
  //{
  //  double c0 = static_cast<double>(channel0) / 255.0;
  //  double c1 = static_cast<double>(channel1) / 255.0;
  //  double c2 = static_cast<double>(channel2) / 255.0;
  //  double c3 = static_cast<double>(channel3) / 255.0;
  //  
  //  double p = (c3 - c2) - (c0 - c1);
  //  double v = t * t * t * p + t * t * ((c0 - c1) - p) + t * (c2 - c0) + c1;
  //  return static_cast<uint8>(v * 255.0);
  //}

  //void CubicInteropColour(const Colour &colour0, const Colour &colour1, const Colour &colour2, const Colour &colour3, double t, Colour &out) const
  //{
  //  out.r = CubicBlend(colour0.r, colour1.r, colour2.r, colour3.r, t);
  //  out.g = CubicBlend(colour0.g, colour1.g, colour2.g, colour3.g, t);
  //  out.b = CubicBlend(colour0.b, colour1.b, colour2.b, colour3.b, t);
  //  out.a = CubicBlend(colour0.a, colour1.a, colour2.a, colour3.a, t);
  //}
};
