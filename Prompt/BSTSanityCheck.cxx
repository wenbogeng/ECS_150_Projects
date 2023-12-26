#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>
#include <vector>

#include "BST.h"

#define SAMPLE_SIZE 1000
#define NUM_TESTS 10000

int main() {

	// C++11 random number tutorial: https://gist.github.com/PhDP/5289449
	// Seed random number generator
	std::mt19937_64 rng(time(0));
	// Create uniform distribution
	std::uniform_int_distribution<int> unif(
		std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
	std::uniform_int_distribution<int> op(0,10);

	std::vector<int> sampleData, BSTSortedData;
	sampleData.reserve(SAMPLE_SIZE);
	BSTSortedData.reserve(SAMPLE_SIZE);
	std::cout << "Running tests..." << std::flush;
	for (unsigned int sample = 0; sample < NUM_TESTS; sample++) {
		BST T;
		// On size_t usage here: https://stackoverflow.com/questions/131803/unsigned-int-vs-size-t
		for (size_t i = 0; i < SAMPLE_SIZE; i++) {
			if (op(rng) == 0 && !T.empty()) {
				T.Delete(sampleData.back());
				sampleData.pop_back();
			} else {			
				// Add random integer to array
				int x = unif(rng);
				T.Insert(x);
				sampleData.push_back(x);
			}
		}
		while (!T.empty()) {
			BSTSortedData.push_back(T.DeleteMin());
		}
		std::sort(sampleData.begin(), sampleData.end());
		assert(sampleData == BSTSortedData);
		BSTSortedData.clear();
		sampleData.clear();
		if (sample % (NUM_TESTS / 10) == 0) {
			std::cout << "." << std::flush;
		}
	}
	std::cout << "Tests complete.\n";
}