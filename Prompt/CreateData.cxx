#include <algorithm>
#include <cstring>
#include <iostream>
#include <random>
#include <set>

#include "json.hpp"

int main(int argc, char** argv) {
	const std::string usage = "Usage: " + std::string(argv[0]) + " numOps minSize mode\n" \
													+ "  mode is a string in (d|D)(m|M)\n" \
													+ "    d disables delete operations, D enables delete operations\n" \
													+ "    m disables deleteMin operations, M enables deleteMin operations\n";
	int numOps = 0, minSize = 0;
	if (argc != 4 ||
			sscanf(argv[1], "%d", &numOps) != 1 || numOps < 1 ||
			sscanf(argv[2], "%d", &minSize) != 1 || minSize < 1 ||
			strlen(argv[3]) != 2 || (tolower(argv[3][0]) != 'd' && tolower(argv[3][1]) != 'd') ||
			(tolower(argv[3][0]) != 'm' && tolower(argv[3][1]) != 'm') || argv[3][0] == argv[3][1]) {
		std::cerr << usage;
		exit(EXIT_FAILURE);
	}
	bool deleteEnabled = (argv[3][0] == 'D'), deleteMinEnabled = (argv[3][1] == 'M');
	// C++11 random number tutorial: https://gist.github.com/PhDP/5289449
	// Seed random number generator
	std::mt19937_64 rng(time(0));
	// Create uniform distribution
	std::uniform_int_distribution<int> unif(
		std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
	std::uniform_int_distribution<int> opDist(0,6);
	nlohmann::json result;
	std::set<int> keys;		
	result["metadata"]["numOps"] = numOps;
	unsigned int totalZeros = (int) floor(log10((double) numOps)) + 1;
	for (size_t op = 1; op <= numOps; op++) {
		int operation = opDist(rng);
		int opDigits = (int) floor(log10((double) op)) + 1;
		std::string opKey = std::string(totalZeros - opDigits, '0')
			.append(std::to_string(op));
		if (operation == 0 && keys.size() >= minSize && deleteMinEnabled) {
			result[opKey] = nlohmann::json();
			result[opKey]["operation"] = "DeleteMin";
			result[opKey]["key"] = *(keys.begin());
			keys.erase(keys.begin());
		} else if (operation == 1 && keys.size() >= minSize && deleteEnabled) {
			result[opKey] = nlohmann::json();
			result[opKey]["operation"] = "Delete";
			int position = unif(rng) % keys.size();
			for (auto itr = keys.begin(); itr != keys.end(); ++itr, --position) {
				if (position == 0) {
					result[opKey]["key"] = *itr;
					keys.erase(itr);
					break;
				}
			}
		} else {
			int key;
			do {
				key = unif(rng);
			} while (keys.count(key) != 0);
			result[opKey] = nlohmann::json();
			result[opKey]["operation"] = "Insert";
			result[opKey]["key"] = key;
			keys.insert(key);
		}
	}
	std::cout << result.dump(2) << std::endl;
}
