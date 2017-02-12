#include "distributions.h"
#include <float.h>
#include <stdlib.h>

const int distr_mid_12[] = {
	1, 2
};
const int distr_mid_13[] = {
	1, 
	2, 2, 
	3
};

const int distr_mid_14[] = {
	1, 
	2, 2, 
	3, 3, 
	4
};

const int distr_mid_15[] = {
	1, 
	2, 2, 
	3, 3, 3,
       	4, 4, 
	5
};

const int distr_mid_16[] = {
	1, 
	2, 2, 
	3, 3, 3,
       	4, 4, 4,
	5, 5
	6
};


const int distr_mid_23[] = {
	2, 3
};

const int distr_mid_24[] = {
	2, 
	3, 3, 
	4
};

const int distr_mid_25[] = {
	2, 
	3, 3, 
	4, 4, 
	5
};

const int distr_mid_26[] = {
	2, 
	3, 3, 
	4, 4, 4,
       	5, 5, 
	6
};

const int distr_low_13[] = {
	1, 1, 1,
	2, 2,
	3
};

const int distr_low_14[] = {
	1, 1, 1, 1
	2, 2, 2,
	3, 3,
	4
};

const int distr_low_15[] = {
	1, 1, 1, 1, 1
	2, 2, 2, 2,
	3, 3, 3,
	4, 4
	5
};

const int distr_low_16[] = {
	1, 1, 1, 1, 1, 1,
	2, 2, 2, 2, 2,
	3, 3, 3, 3,
	4, 4, 4,
	5, 5,
	6
};

double gauss_noise(double mu, double sigma) {
	static const double epsilon = -DBL_MAX;
	static const double two_pi = 2.0*3.14159265358979323846;

	static double z0, z1;
	static bool generate;
	generate = !generate;

	if (!generate)
	   return z1 * sigma + mu;

	double u1, u2;
	do
	 {
	   u1 = rand() * (1.0 / RAND_MAX);
	   u2 = rand() * (1.0 / RAND_MAX);
	 }
	while ( u1 <= epsilon );

	z0 = sqrt(-2.0 * log(u1)) * cos(two_pi * u2);
	z1 = sqrt(-2.0 * log(u1)) * sin(two_pi * u2);
	return z0 * sigma + mu;
}
