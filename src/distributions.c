#include "distributions.h"
#include <float.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

double gauss_noise(double mu, double sigma) {
	static const double epsilon = -DBL_MAX;
	static const double two_pi = 2.0*3.14159265358979323846;

	static double z0, z1;
	static bool generate;
	generate = !generate;

	if (!generate)
		return z1 * sigma + mu;

	double u1, u2;
	do {
		u1 = rand() * (1.0 / RAND_MAX);
		u2 = rand() * (1.0 / RAND_MAX);
	}
	while ( u1 <= epsilon );

	z0 = sqrt(-2.0 * log(u1)) * cos(two_pi * u2);
	z1 = sqrt(-2.0 * log(u1)) * sin(two_pi * u2);
	return z0 * sigma + mu;
}

double gauss_noise_with_limit(double mu, double sigma, double min, double max) {
	double r = gauss_noise(mu, sigma);

	while (r < min || floor(r) > max) r = gauss_noise(mu, sigma);

	return r;
}
