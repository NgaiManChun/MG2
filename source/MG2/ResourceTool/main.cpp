#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include "MGResource.h"

using namespace MG;
namespace fs = std::filesystem;

std::vector<fs::path> CollectFiles(const fs::path& inputDir)
{
	std::vector<fs::path> files;

	for (const auto& entry : fs::recursive_directory_iterator(inputDir)) {
		if (!entry.is_regular_file())
			continue;

		files.push_back(entry.path());
	}

	return files;
}

int main(int argc, char* argv[])
{
	if (argc < 3) {
		std::cerr << "Usage: MGResource <input_dir> <output_pak>\n";
		return 1;
	}

	const char* inputDir = argv[1];
	const char* outputPak = argv[2];

	std::cout << "Input Dir : " << inputDir << "\n";
	std::cout << "Output Pak: " << outputPak << "\n";

	std::vector<fs::path> filePaths = CollectFiles(inputDir);

	fs::path inputPath(inputDir);

	MGResource resource(outputPak);

	for (auto path : filePaths) {
		fs::path relative = fs::relative(path, inputDir);
		std::string relative_path = relative.generic_string();
		resource.Add(path.string().data(), relative_path.data());
	}

	resource.Write(outputPak);

	resource.Release();

	return 0;
}