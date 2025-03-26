#pragma once

#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <vector>

class RandomGenerator
{
private:
	std::random_device rd;
	std::mt19937_64 gen;
	std::uniform_real_distribution<float> distrib{ 0.0f, 1.0f };

public:
	RandomGenerator() : gen(rd() ^ std::chrono::steady_clock::now().time_since_epoch().count()) {}

	void fillGridWithNoise(std::vector<float>& grid) {
		std::generate(grid.begin(), grid.end(), [this]() {return distrib(gen); });
	}

	void diagnosticPrintout(const std::vector<float>& grid, int width) {
		std::cout << "Checking for pattern repetition:\n";
		for (size_t i = 0; i < grid.size(); ++i) {
			// Print every 5th pixel value
			if (i % 4 == 0) {
				std::cout << "Pixel " << i << ": " << grid[i] << std::endl;
			}
		}
	}
};

