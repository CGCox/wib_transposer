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
#include <sstream>
#include <tuple>
#include <array>



#include <unordered_map>
#include <algorithm>
#include <stdexcept>
#include <cstring>
#include <cstdlib>
#include <cstdint>
//#include <Frame.hpp> //frame constructor hierarch: frame; timestamp; offchID; adc. *No longer available

#include "wib_transposer/BinaryFileReader.hpp"

#include "detdataformats/wib/WIBFrame.hpp"

//#include "dune-raw-data/Overlays/WIBFrame.hh"

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
    //bool help = false; //help option
    std::string tsvinput;
    std::string binoutput;

    //app.add_flag("-?,--help", help, "Assistance");

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

    if (binoutput.empty()) {
        binoutput = "output";
    }

    std::ifstream infile(tsvinput);
    std::string columnline;
    int num_cols = -1;
    
    if (infile) {
        while (getline(infile, columnline)) {
            int count = 0;
            for (char& c : columnline) {
                if (c == '\t') {
                    count++;
                }
            }
            if (num_cols == -1) {
                num_cols = count;
            } else if (count != num_cols) {
                std::cerr << "Error: File does not have two columns" << std::endl;
                return 1;
            }
        }
        infile.close();
    } else {
        std::cerr << "Error: Could not open file" << std::endl;
        return 1;
    }

/*
        //The help description - LEAVING OUT for now, std::exception error thrown if included
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
*/


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

    //ERROR CHECKS not working - need debugged
/*
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
*/






  // Temporary memory to store TSV data

  // Read the TSV file
  while (std::getline(tsv_file, line)) {
std::istringstream iss(line);
std::vector<std::string> fields;
std::string field;
while (std::getline(iss, field, '\t')) {
    fields.push_back(field);
  }
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
    uint16_t crate;
    uint16_t slot;
    uint16_t link;
    uint16_t wire;
    uint16_t adc[256];
    std::vector<std::tuple<uint8_t, uint8_t, uint8_t, uint8_t>> cslw_values;
    std::array<std::array<std::array<std::array<uint16_t, 128>, 6>, 4>, 4> adc_values;
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

struct WIBHeader {
    uint32_t crate_number : 8;
    uint32_t slot_number : 8;
    uint32_t fiber_number : 8;
    uint32_t geo_address : 8;
    uint32_t reserved1 : 24;
    uint32_t timestamp_high : 32;
    uint32_t timestamp_mid : 32;
    uint32_t timestamp_low : 16;
    uint32_t version : 4;
    uint32_t format : 4;
    uint32_t crate_bits : 4;
    uint32_t slot_bits : 4;
    uint32_t fiber_bits : 4;
    uint32_t reserved2 : 4;
    uint32_t event_number_high : 16;
    uint32_t event_number_low : 16;
};

struct WIBFrame {
    uint64_t frame_number;
    uint64_t timestamp;
    std::array<std::array<std::array<std::array<uint16_t, 256>, 4>, 6>, 2> adc_values;
};

std::vector<WIBFrame> wib_data;

for (size_t i = 0; i < temp_wib_data.size(); i += 256) {
    WIBFrame wib_frame;
    wib_frame.frame_number = temp_wib_data[i].frame_number;
    wib_frame.timestamp = temp_wib_data[i].timestamp;
    for (size_t j = i; j < i + 256; ++j) {
        size_t wire = temp_wib_data[j].wire;
        uint8_t board = wire / 128;
        uint8_t channel = (wire % 128) / 32;
        uint8_t link = (wire % 128) % 32 / 8;
        uint8_t sample = wire % 8;
        wib_frame.adc_values[board][channel][link][sample * 32 + j % 32] = temp_wib_data[j].adc_values[channel][link][board][wire];
    }
    wib_data.push_back(wib_frame);
}

// Output the structs to a .out binary file

const uint32_t WIB_HEADERSIZE = 26;
const uint32_t WIB_CHANNELS_PER_FRAME = 256;
const uint32_t WIB_SAMPLES_PER_CHANNEL = 112;

struct WibFrame {
    uint16_t crate_id;
    uint16_t slot_id;
    uint16_t fiber_id;
    uint16_t wibcode;
    uint16_t timestamp;
    uint16_t frame_version;
    uint16_t trig_frame;
    uint16_t trig_sample;
    uint16_t trig_frame_mod8;
    uint16_t fib_fe_latency;
    uint16_t fib_be_latency;
    uint16_t fib_ch_latency[4];
    uint16_t wib_ch_ctrl[4];
    uint16_t adc[4][WIB_SAMPLES_PER_CHANNEL];
};

    // define vector to hold WibFrame structs
    std::vector<WibFrame> frames;

    // populate vector with WibFrame structs
    for (int i = 0; i < 10; i++) {
        WibFrame frame;
        frame.crate_id = 0x1;
        frame.slot_id = 0x2;
        frame.fiber_id = 0x3;
        frame.wibcode = 0x4;
        frame.timestamp = 0x5;
        frame.frame_version = 0x6;
        frame.trig_frame = 0x7;
        frame.trig_sample = 0x8;
        frame.trig_frame_mod8 = 0x9;
        frame.fib_fe_latency = 0xA;
        frame.fib_be_latency = 0xB;
        for (int j = 0; j < 4; j++) {
            frame.fib_ch_latency[j] = 0xC + j;
            frame.wib_ch_ctrl[j] = 0xD + j;
            for (int k = 0; k < WIB_SAMPLES_PER_CHANNEL; k++) {
                frame.adc[j][k] = k;
            }
        }
        frames.push_back(frame);
    }

    std::string protooutput;
    if (D == true || E == true){
       protooutput = binoutput + "-protowib.out";
    } else {
        protooutput = binoutput + ".out";
    }

    //check

        if (std::filesystem::exists(protooutput)) {
        std::cerr << "Protowib output file already exists.\n";
        } else {


    // open output file in binary mode
    std::ofstream outfile(protooutput, std::ios::out | std::ios::binary);

    // write vector of WibFrame structs to binary file
    for (const WibFrame& frame : frames) {
        outfile.write(reinterpret_cast<const char*>(&frame), sizeof(frame));
    }

    // close output file
    outfile.close();
        }
}



if (D == true) { // WIB-II translation
    std::cout << "WIB-II, dunewib data format, transposition is not yet available";

}

if (E == true) { // ethernet translation
    std::cout << "Ethernet, ethernet data format, transposition is not yet available";

}

    return 0;
}