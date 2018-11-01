/*	SphericalHarmonics.cpp

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

#include "SphericalHarmonics.h"
#include "Math.h"
#include <cassert>
#include <random>

namespace molecular
{

namespace SphericalHarmonics
{

/** https://en.wikipedia.org/wiki/Associated_Legendre_polynomials */
double AssociatedLegendrePolynomial(int l, int m, double x)
{
	assert(l <= 3);
	assert(m <= 3);
	assert(m >= -3);

	if(l == 0 && m == 0)
	{
		return 1.0;
	}
	else if(l == 1)
	{
		if(m == -1)
			return -0.5 * AssociatedLegendrePolynomial(1, 1, x);
		else if(m == 0)
			return x;
		else if(m == 1)
			return -std::sqrt(1 - x * x);
	}
	else if(l == 2)
	{
		if(m == -2)
			return 0.04166666666666667 * AssociatedLegendrePolynomial(2, 2, x);
		else if(m == -1)
			return 0.16666666666666667 * AssociatedLegendrePolynomial(2, 1, x);
		else if(m == 0)
			return 0.5 * (3 * x * x - 1);
		else if(m == 1)
			return -3 * x * std::sqrt(1 - x * x);
		else if(m == 2)
			return 3 * (1 - x * x);
	}
	else if(l == 3)
	{
		if(m == -3)
			return -0.00138888888888888889 * AssociatedLegendrePolynomial(3, 3, x);
		else if(m == -2)
			return 0.00833333333333333333 * AssociatedLegendrePolynomial(3, 2, x);
		else if(m == -1)
			return -0.0833333333333333333 * AssociatedLegendrePolynomial(3, 1, x);
		else if(m == 0)
			return 0.5 * (5 * x * x * x - 3.0 * x);
		else if(m == 1)
			return -1.5 * (5 * x * x - 1.0) * std::sqrt(1 - x * x);
		else if(m == 2)
			return 15 * x * (1 - x * x);
		else if(m == 3)
			return 15 * x * x * std::sqrt(1 - x * x) - 15 * std::sqrt(1 - x * x);
	}

	assert(false);
	return 0.0;
}

double Factorial(int x)
{
	static const double table[] = {
		/*0*/	1.0,
		/*1*/	1.0,
		/*2*/	2.0,
		/*3*/	6,
		/*4*/	24,
		/*5*/	120,
		/*6*/	720,
		/*7*/	5040,
		/*8*/	40320,
		/*9*/	362880,
		/*10*/	3628800,
		/*11*/	39916800,
		/*12*/	479001600,
		/*13*/	6227020800,
		/*14*/	87178291200,
		/*15*/	1307674368000,
		/*16*/	20922789888000,
		/*17*/	355687428096000,
		/*18*/	6402373705728000,
		/*19*/	121645100408832000,
		/*20*/	2432902008176640000,
		/*21*/	51090942171709440000.0,
		/*22*/	1124000727777607680000.0,
		/*23*/	25852016738884976640000.0,
		/*24*/	6.2044840173323943936e23,
		/*25*/	1.5511210043330985984e25,
		/*26*/	4.03291461126605635584e26,
		/*27*/	1.0888869450418352160768e28,
		/*28*/	3.04888344611713860501504e29,
		/*29*/	8.841761993739701954543616e30,
		/*30*/	2.6525285981219105863630848e32,
		/*31*/	8.22283865417792281772556288e33,
		/*32*/	2.6313083693369353016721801216e35,
		/*33*/	8.68331761881188649551819440128e36,
		/*34*/	2.9523279903960414084761860964352e38,
		/*35*/	1.03331479663861449296666513375232e40
	};
	assert(x >= 0 && x < sizeof(table));
	return table[x];
}

double ShRenormalisation(int l, int m)
{
	return std::sqrt((Factorial(l - m) * (2.0 * l + 1.0)) / (Factorial(l + m) * 4.0 * Math::kPi_d));
}

double SphericalHarmonic(int l, int m, double theta, double phi)
{
	if(m > 0)
		return std::sqrt(2.0) * ShRenormalisation(l, m) * std::cos(m * phi) * AssociatedLegendrePolynomial(l, m, std::cos(theta));
	else if(m == 0)
		return ShRenormalisation(l, 0) * AssociatedLegendrePolynomial(l, m, std::cos(theta));
	else // m < 0
		return std::sqrt(2.0) * ShRenormalisation(l, -m) * std::sin(-m * phi) * AssociatedLegendrePolynomial(l, -m, std::cos(theta));
}

Vector3d UnitVectorFromSphericalCoords(double theta, double phi)
{
	return Vector3d(std::sin(theta) * std::cos(phi), std::sin(theta) * std::sin(phi), std::cos(theta));
}

template<int numBands>
std::vector<Sample<numBands>> SetupSphericalSamples(unsigned int samplesCount)
{
	std::vector<Sample<numBands>> samples;
	samples.reserve(samplesCount * samplesCount);
	std::mt19937 randomEngine;
	std::uniform_real_distribution<double> randomDist(0, 1);

	const double samplesCountInv = 1.0 / samplesCount;
	for(unsigned int i = 0; i < samplesCount; i++)
	{
		for(unsigned int j = 0; j < samplesCount; j++)
		{
			Sample<numBands> sample;
			const double u = i + randomDist(randomEngine);
			const double v = j + randomDist(randomEngine);
			sample.theta = 2.0 * std::acos(sqrt(1.0 - u * samplesCountInv));
			sample.phi = 2.0 * Math::kPi_d * v * samplesCountInv;
			sample.vec = UnitVectorFromSphericalCoords(sample.theta, sample.phi);

			for(int l = 0; l < numBands; ++l)
			{
				for(int m = -l; m <= l; ++m)
					sample.coeff[l * (1 + l) + m] = SphericalHarmonic(l, m, sample.theta, sample.phi);
			}
			samples.push_back(sample);
		}
	}
	return samples;
}

template std::vector<Sample<3>> SetupSphericalSamples<3>(unsigned int samplesCount);
template std::vector<Sample<4>> SetupSphericalSamples<4>(unsigned int samplesCount);

}

} // namespace molecular
