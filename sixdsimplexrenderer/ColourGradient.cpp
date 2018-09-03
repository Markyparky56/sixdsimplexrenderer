#include "ColourGradient.hpp"
#include <cassert>

inline int Clamp(int value, int lowerBound, int upperBound)
{
  if (value < lowerBound) return lowerBound;
  else if (value > upperBound) return upperBound;
  else return value;
}

ColourGradient::ColourGradient()
  : gradientPointCount(0)
  , gradientPoints(0)
{
}

void ColourGradient::AddGradientPoint(double gradientPos, const Colour &colour)
{
  int insertionPos = FindInsertionPos(gradientPos);
  InsertAtPos(insertionPos, gradientPos, colour);
}

void ColourGradient::Clear()
{
  gradientPoints.clear();
  gradientPointCount = 0;
}

const Colour &ColourGradient::GetColour(double gradientPos)
{
  assert(gradientPointCount >= 2);
  //std::unique_lock<std::mutex> lock(lookupMutex);

  int indexPos;
  for (indexPos = 0; indexPos < gradientPointCount; indexPos++)
  {
    if (gradientPos < gradientPoints[indexPos].pos)
    {
      break;
    }
  }

  //if (gradientPointCount < 3)
  {
    int index0 = Clamp(indexPos - 1, 0, gradientPointCount - 1);
    int index1 = Clamp(indexPos    , 0, gradientPointCount - 1);

    if (index0 == index1)
    {
      return gradientPoints[index1].colour;
    }

    double input0 = gradientPoints[index0].pos;
    double input1 = gradientPoints[index1].pos;
    double t = (gradientPos - input0) / (input1 - input0);

    // Linear interop
    const Colour &colour0 = gradientPoints[index0].colour;
    const Colour &colour1 = gradientPoints[index1].colour;
    return LinearInteropColour(colour0, colour1, t);
    //return workingColour;
  }
  //else // Use Cubic Interpolation
  //{
  //  int index1 = Clamp(indexPos - 1, 0, gradientPointCount - 1);
  //  int index2 = Clamp(indexPos    , 0, gradientPointCount - 1);

  //  if (index1 == index2)
  //  {
  //    workingColour = gradientPoints[index2].colour;
  //    return workingColour;
  //  }

  //  int index0 = Clamp(index1 - 1, 0, gradientPointCount - 1);
  //  int index3 = Clamp(index2 + 1, index2, gradientPointCount - 1);

  //  double input0 = gradientPoints[index1].pos;
  //  double input1 = gradientPoints[index2].pos;
  //  double t = (gradientPos - input0) / (input1 - input0);
  //
  //  // Cubic interop, 0 & 1 or 2 & 3 can be doubles
  //  const Colour &colour0 = gradientPoints[index0].colour;
  //  const Colour &colour1 = gradientPoints[index1].colour;
  //  const Colour &colour2 = gradientPoints[index2].colour;
  //  const Colour &colour3 = gradientPoints[index3].colour;
  //  CubicInteropColour(colour0, colour1, colour2, colour3, t, workingColour);
  //  return workingColour;
  //}
}

int ColourGradient::FindInsertionPos(double gradientPos)
{
  int insertionPos;
  for (insertionPos = 0; insertionPos < gradientPointCount; insertionPos++)
  {
    if (gradientPos < gradientPoints[insertionPos].pos)
    {
      break; // Found array index to insert new gradient point at
    }
    else if (gradientPos == gradientPoints[insertionPos].pos)
    {
      throw std::exception("Gradient Points Must Have Unique Positions");
    }
  }
  return insertionPos;
}

void ColourGradient::InsertAtPos(int insertionPos, double gradientPos, const Colour &colour)
{
  auto iter = gradientPoints.begin();
  std::advance(iter, insertionPos);
  gradientPoints.insert(iter, { gradientPos, colour });
  ++gradientPointCount;
}
