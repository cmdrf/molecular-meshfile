/*	SphericalHarmonics.h

MIT License

Copyright (c) 2018 Fabian Herb

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef SPHERICALHARMONICS_H
#define SPHERICALHARMONICS_H

#include "Vector3.h"
#include "Vector.h"
#include <array>
#include <vector>

namespace SphericalHarmonics
{

template<int numBands>
struct Sample
{
	double theta, phi;
	Vector3d vec;
	Vector<numBands * numBands, double> coeff;
};

double AssociatedLegendrePolynomial(int l, int m, double x);

/// Fast factorial function using a precomputed table
double Factorial(int x);

/// Generate samples that are uniformly distributed on a sphere
template<int numBands>
std::vector<Sample<numBands>> SetupSphericalSamples(unsigned int samplesCount = 100);

extern template std::vector<Sample<3>> SetupSphericalSamples<3>(unsigned int samplesCount);
extern template std::vector<Sample<4>> SetupSphericalSamples<4>(unsigned int samplesCount);

}

#endif // SPHERICALHARMONICS_H
