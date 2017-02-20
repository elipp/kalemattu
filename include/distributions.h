#ifndef DISTRIBUTIONS_H
#define DISTRIBUTIONS_H

long get_random(long min, long max);
double get_randomf();

double gauss_noise(double mu, double sigma);
double gauss_noise_with_limit(double mu, double sigma, double min, double max);

#endif
