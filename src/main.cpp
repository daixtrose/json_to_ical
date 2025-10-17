#include "json_to_ical.h"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <input.json> [output.ics]" << std::endl;
        std::cout << "\nConvert JSON file containing calendar events to iCalendar format." << std::endl;
        std::cout << "If output file is not specified, output.ics will be used." << std::endl;
        return 1;
    }

    std::string input_file = argv[1];
    std::string output_file = (argc >= 3) ? argv[2] : "output.ics";

    ICalGenerator generator;

    std::cout << "Loading events from " << input_file << "..." << std::endl;
    if (!generator.loadFromJson(input_file)) {
        std::cerr << "Failed to load JSON file." << std::endl;
        return 1;
    }

    std::cout << "Converting to iCalendar format..." << std::endl;
    if (!generator.saveToFile(output_file)) {
        std::cerr << "Failed to save iCalendar file." << std::endl;
        return 1;
    }

    std::cout << "Conversion completed successfully!" << std::endl;
    return 0;
}
