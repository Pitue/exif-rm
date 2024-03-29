#include <iostream>
#include <filesystem>

#include <CLI/CLI.hpp>
#include <exiv2/exif.hpp>
#include <exiv2/image.hpp>
#include <fmt/format.h>

bool RemoveFromFile(const std::string& path);

int main(int argc, char **argv) {
	namespace fs = std::filesystem;

	std::string path;
	bool copy = false;
	std::stringstream sstream;
	CLI::App app("remove exif data from file or folder.", "exif-rm");
	app.add_option("-f,--file", path, "the image file or folder to remove data from")->required();
	app.add_flag("-c,--copy", copy, "copy files before overwriting it")->default_val(false);
	try {
		CLI11_PARSE(app, argc, argv);
		Exiv2::ExifData data;
		
		if (fs::is_regular_file(path)) {
			if (copy) {
				sstream << path << ".orig";
				fs::copy_file(path, sstream.str());
			}
			if (!RemoveFromFile(path)) {
				std::cout << std::format("Couldn't clear meta data from file \"{}\".\n", path);
			} else {
				std::cout << std::format("Successfully cleared meta data from file \"{}\".\n", path);
			}
		} else if (fs::is_directory(path)) {
			for (auto p : fs::directory_iterator(path, fs::directory_options::skip_permission_denied)) {
				if (fs::is_regular_file(p)) {
					if (copy) {
						sstream << p.path().string() << ".orig";
						fs::copy_file(path, sstream.str());
						sstream.clear();
					}
					if (!RemoveFromFile(p.path().string())) {
						std::cout << std::format("Couldn't clear meta data from file \"{}\".\n", p.path().string());
					} else {
						std::cout << std::format("Successfully cleared meta data from file \"{}\".\n", p.path().string());
					}
				}
			}
		} else {
			throw std::runtime_error(std::format("\"{}\" isn't a regular file nor a directory", path));
		}
	} catch (Exiv2::Error& e) {
		std::cerr << std::format("Exiv2 error: \"{}\".\n", e.what());
	}	catch (std::exception& ex) {
		std::cerr << std::format("Error: \"{}\".\n", ex.what());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

bool RemoveFromFile(const std::string& path) {
	auto image = Exiv2::ImageFactory::open(path);
	if (image) {
		image->readMetadata();
		image->clearMetadata();
		image->writeMetadata();
		return true;
	}
	return false;
}