#ifndef LAB04_RANDOM_H
#define LAB04_RANDOM_H

#include <random>
#include <chrono>

template <typename T>
class random
{
	std::default_random_engine gen;
	std::uniform_real_distribution<double> dist;
public:
	random(double min, double max): gen(std::chrono::system_clock::now().time_since_epoch().count()),
	                                dist(min, max)
	{
	}

	T next()
	{
		return static_cast<T>(dist(gen));
	}
};

#endif
