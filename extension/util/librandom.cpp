#include <random>
#include "librandom.h"

int random::generate_random_int(int min, int max)
{
	std::random_device generator;
	std::uniform_int_distribution<int> dist(min, max);
	return dist(generator);
}

unsigned int random::generate_random_uint(unsigned int min, unsigned int max)
{
	std::random_device generator;
	std::uniform_int_distribution<unsigned int> dist(min, max);
	return dist(generator);
}

bool random::generate_random_bool()
{
	return generate_random_int(0, 1) == 1;
}

float random::generate_random_float(float min, float max)
{
	std::random_device generator;
	std::uniform_real_distribution<float> dist(min, max);
	return dist(generator);
}

double random::generate_random_double(double min, double max)
{
	std::random_device generator;
	std::uniform_real_distribution<double> dist(min, max);
	return dist(generator);
}

// generates a random bool with a specific chance
bool random::random_chance(int chance)
{
	return generate_random_int(0, 99) < chance;
}
