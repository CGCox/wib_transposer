#include "CLI/CLI.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <fmt/core.h>
#include <filesystem>
#include <chrono>
#include <string>
#include <stdio.h>
#include <memory>
#include <map>
#include <regex>
#include <cstdint>
#include <sstream>
#include <tuple>
#include <array>

#include <unordered_map>
#include <algorithm>
#include <stdexcept>
#include <cstring>
#include <cstdlib>
//#include <Frame.hpp> //frame constructor hierarch: frame; timestamp; offchID; adc. *No longer available

#include "wib_transposer/BinaryFileReader.hpp"

#include "detdataformats/wib/WIBFrame.hpp"

//#include "detchannelmaps/HardwareMap.hpp"
#include "detchannelmaps/TPCChannelMap.hpp"

#include "logging/Logging.hpp"

using namespace dunedaq;


struct Data {
    int c1;
    double c2;
};

int main(int argc, char **argv) {
    // Create a CLI app object
    CLI::App app{"Format convertor from Universal Intermediate"};

    bool P = false; //WIB-I this is the default conversion format - if not options input, this is the format chosen
    bool D = false; //WIB-II format
    bool E = false; //Ethernet format
    bool help = false; //help option
    std::string tsvinput;
    std::string binoutput;

    app.add_flag("-?,--help", help, "Assistance");

    // input file from Universal Format
    app.add_option("tsv input", tsvinput, "Name of input TSV file")->required();

    // format selection
    app.add_flag("-P,--format-protowib", P, "protowib, or WIB-I, output format");
    app.add_flag("-D,--format-wib2", D, "dunewib, or WIB-II, output format");
    app.add_flag("-E,--format-ethernet", E, "ethernet output format");

    // output file to dump data
    app.add_option("output binary file(s) name", binoutput, "Name of output .out file");

    // parse the command-line arguments
    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &e) {
        return app.exit(e);
    }

    // check if input file has a .tsv extension; if not, it is added before checking if the file exists
    std::regex iregex(R"((.*)\.tsv)");
    if (!std::regex_match(tsvinput, iregex)) {
        tsvinput += ".tsv"; // fixed variable name and added missing semicolon
    }

    if (std::filesystem::exists(binoutput)) {
        std::cerr << "Output file already exists. Exiting.\n";
        exit(-1);
    }

        //The help description
    if (help) {
fmt::print("Please enter the command as follows:\n");
fmt::print("            <program>   <input file name>   -WhichFormatDoYouWant   <output file name>\n");
fmt::print("EXAMPLE:    UIFT        input.tsv           -PDE                    output \n");
fmt::print("The latter two options, the naming of the output file and formatting options are not required\n");
fmt::print("\n");
fmt::print("-P or --format-protowib         will translate the data into WIB-I binary format\n");
fmt::print("-D or --format-wib2             will translate the data into WIB-II binary format\n");
fmt::print("-E or --format-ethernet         will translate the data into ethernet binary format\n");
fmt::print("\n");
fmt::print("As a default, the translator will translate into WIB-I if given no format option instructions\n");
fmt::print("\n");
fmt::print("You can also translate into multiple formats at once if you so wish by inputting multiple options such as:\n");
fmt::print("-P -D   or equally   -PD\n");
        exit(0);
    }



    if (P || D || E) {
        // if any output format flag is specified, set the corresponding bool to true
        if (P) {
            P = true;
        }
        if (D) {
            D = true;
        }
        if (E) {
            E = true;
        }
    } else {
        // if no output format flag is specified, use the default format (WIB-I)
        P = true;
    }


//unpack TSV file into temporary struct

  std::ifstream tsv_file(tsvinput);
  std::string line;

  tsv_file.open(tsvinput, std::ios::binary); //Uses ifile library to provide open command;
 if ( !tsv_file.is_open() ) {
        throw std::runtime_error(fmt::format("file couldn't be opened {}", tsvinput));
    }


    // Read the TSV input file into temporary memory
    int frame;
    int timestamp;
    int offline_channel_id;
    int adc;
    std::vector<std::vector<std::string>> tsv_data; (frame, timestamp, (offline_channel_id, adc));
    while (getline(tsv_file, line)) {
        // Skip blank lines and lines that start with a comment character #
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // Split the line into fields separated by tabs
        std::istringstream iss(line);
        std::string field;
        std::vector<std::string> row;
        while (getline(iss, field, '\t')) {
            row.push_back(field);
        }

        // Check if the row has the correct number of fields
        if (row.size() != 2) {
            std::cerr << "Error: incorrect number of fields in TSV file at line " << tsv_data.size() + 1 << std::endl;
            return 1;
        }

        // Check if the row represents the start of a new block of data
        int frame_number = stoi(row[0]);
        int timestamp = stoi(row[1]);
        if (frame_number >= 0 && timestamp >= 0 && tsv_data.size() % 257 == 0) {
            // This row represents the start of a new block of data
            tsv_data.push_back({row[0], row[1]});
        } else {
            // This row represents a channel in the current block of data
            tsv_data.back().push_back(row[0]);
            tsv_data.back().push_back(row[1]);
        }
    }

    // Check if the TSV file ends with a single E element
    if (tsv_data.back().size() != 2 || tsv_data.back()[1] != "E") {
        std::cerr << "Error: TSV file does not end with a single E element" << std::endl;
        return 1;
    }

// Check if each block of data has exactly 256 channels
for (int i = 0; i < tsv_data.size() - 1; i += 257) {
    if (tsv_data[i + 1].size() != 2 || tsv_data[i + 257].size() != 2) {
        std::cerr << "Error: TSV data block #" << (i / 257) << " does not have exactly 256 channels." << std::endl;
        throw std::invalid_argument("Error: TSV data block does not have exactly 256 channels.");
    }
    // Check if the block ends with -1 -1
    if (tsv_data[i + 256][0].compare("-1") != 0 || tsv_data[i + 256][1].compare("-1") != 0) {
        std::cerr << "Error: TSV data block #" << (i / 257) << " does not end with -1 -1." << std::endl;
        throw std::invalid_argument("Error: TSV data block does not end with -1 -1.");
    }
}

// Check if the last line of the TSV file is a single E element
if (tsv_data[tsv_data.size() - 1][0].compare("E") != 0 || tsv_data[tsv_data.size() - 1].size() != 1) {
    std::cerr << "Error: TSV file does not end with a single E element." << std::endl;
    throw std::invalid_argument("Error: TSV file does not end with a single E element.");
}


  // Temporary memory to store TSV data

  // Read the TSV file
  while (std::getline(tsv_file, line)) {
    std::istringstream iss(line);
    std::vector<std::string> row_data(std::istream_iterator<std::string>{iss},
                                       std::istream_iterator<std::string>{});

    tsv_data.push_back(row_data);
  }

  // Print the temporary memory for testing
  for (auto row : tsv_data) {
    for (auto elem : row) {
      std::cout << elem << " ";
    }
    std::cout << std::endl;
  }

//Transposition

if (P == true) { //protowib translation

// Convert temporary memory to WIBFrames and ADCs
// Create a new vector of vectors to store the CSLW numbers


//wire loop: 256; fibre loop: 4; slot loop: 10; crate: 5.
struct TempWIBFrame {
  uint64_t frame_number;
  uint64_t timestamp;
  std::array<std::array<std::array<std::array<uint16_t, 256>, 4>, 6>, 2> adc_values;
  std::vector<std::tuple<uint8_t, uint8_t, uint8_t, uint8_t>> cslw_values;
};

std::vector<TempWIBFrame> temp_wib_data;
uint64_t frame_number = 0;
for (auto row : tsv_data) {
  ++frame_number;
  for (uint16_t i = 0; i < 256; ++i) {
    std::shared_ptr<detchannelmaps::TPCChannelMap> pdps1_map = detchannelmaps::make_map("ProtoDUNESP1ChannelMap");
    uint ocid = pdps1_map->get_offline_channel_from_crate_slot_fiber_chan(std::stoi(row[0]), std::stoi(row[1]), std::stoi(row[2]), i);
    if (ocid != 0) {
      TempWIBFrame temp_frame;
      temp_frame.frame_number = frame_number;
      temp_frame.timestamp = std::stoull(row[3]);
      for (uint8_t board = 0; board < 2; ++board) {
        for (uint8_t link = 0; link < 6; ++link) {
          for (uint8_t channel = 0; channel < 4; ++channel) {
            if (pdps1_map->get_offline_channel_from_crate_slot_fiber_chan(std::stoi(row[0]), std::stoi(row[1]), link, channel) == ocid) {
              for (uint8_t sample = 0; sample < 128; ++sample) {
                uint16_t adc_value = std::stoi(row[4 + 128 * (4 * board + 6 * channel + link) + sample]);
                temp_frame.adc_values[channel][link][board][i] = adc_value;
              }
            }
          }
        }
      }
      uint8_t crate_no = std::stoi(row[0]);
      uint8_t slot_no = std::stoi(row[1]);
      uint8_t fiber_no = std::stoi(row[2]);
      uint8_t wire_no = i;
      temp_frame.cslw_values.push_back(std::make_tuple(crate_no, slot_no, fiber_no, wire_no));
      temp_wib_data.push_back(temp_frame);
    }
  }
}

//Populating protowib data struct
// create a WIBFrame object
detdataformats::wib::WIBFrame wibframe;

// populate the WIBFrame object with the data from the temporary WIB memory struct



   
}



if (D == true) { // WIB-II translation
    std::cout << "WIB-II, dunewib data format, transposition is not yet available";

}

if (E == true) { // ethernet translation
    std::cout << "Ethernet, ethernet data format, transposition is not yet available";

}
/*
    std::string fin = (tsvinput + ".tsv");
    std::string input_file_name = fin;
    std::string output_file_name = "output.out";

    std::ifstream input_file(input_file_name);
    if (!input_file.is_open()) {
        std::cerr << "Failed to open input file: " << input_file_name << std::endl;
        return 1;
    }
*/

/*
    

    std::vector<Data> data;
    int c1;
    double c2;
    while (input_file >> c1 >> c2) {
        data.push_back({c1, c2});
    }

    std::ofstream output_file(output_file_name, std::ios::binary);
    if (!output_file.is_open()) {
        std::cerr << "Failed to open output file: " << output_file_name << std::endl;
        return 1;
    }

    for (const auto& d : data) {
        output_file.write(reinterpret_cast<const char*>(&d), sizeof(d));
    }
*/
    return 0;
}