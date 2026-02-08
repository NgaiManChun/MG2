#include "model.h"
#include "animation.h"
#include <iostream>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

int main(int argc, char* argv[])
{
	if (argc < 4) {
		std::cerr << "Usage: FBXTool <mode> <input> <output>\n";
		return 1;
	}

	std::string mode = argv[1];
	const char* input = argv[2];
	const char* output = argv[3];

	fs::path inputPath(input);

	if (fs::exists(inputPath)) {
		if (mode == "model") {
			ReadModel(input, output);
			std::cout << "Model Input : " << input << "\n";
			std::cout << "Model Output: " << output << "\n";
		}
		else if (mode == "animation") {
			ReadAnimation(input, output);
			std::cout << "Animation Input : " << input << "\n";
			std::cout << "Animation Output: " << output << "\n";
		}
	}

	return 0;
}