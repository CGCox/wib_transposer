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
std::ifstream tsv_file;
std::string line;
tsv_file.open(tsvinput); // open the file
if (!tsv_file.is_open()) {
    std::cerr << "Error: file couldn't be opened " << tsvinput << std::endl;
    return 1;
}

std::cout << "TSV file opened successfully." << std::endl;

// Read the TSV input file into temporary memory
int frame;
unsigned long long timestamp;
int offlinechannelid;
int adc;
std::vector<std::tuple<int, long long, std::vector<int>>> data_vector; //(frame, timestamp, (offline_channel_id, adc));
int row_num = 0;
int BN = 0; // Block number
int Header_FN = 0;
unsigned long long Header_T = 0;
bool Header = true;
std::string Element1;
std::string Element2;
std::regex rgx("\\s+");
int channels_per_block = 0; // number of channels per block
int current_channel = 0; // current channel in the block
std::vector<std::vector<int>> block_data; // stores the data of a block
//Predfines a bunch of variables to simplify loop

while (std::getline(tsv_file, line)) {
    std::sregex_token_iterator iter(line.begin(), line.end(), rgx, -1); //Separates the tsv row into two distinct elements separated by a tab/space
    std::vector<std::string> data(iter, {});
    std::string Element1 = data[0]; //the first element of the tow

    if (Element1 == "E") { //End of TSV data
        break;
    }
    
    std::string Element2 = data[1]; //the second element of the row
    

    if(Header){ //starts of true, assuming first line of TSV is a header line
        Header_FN = std::stoi(Element1); //assigns the frame number for a block
        Header_T = std::stoull(Element2); //assigns the timestamp for a block
        BN++; //Increases the block number counter - everytime there's a new headerline, BN increases by 1
        Header = false;
    } else {
        if(std::stoi(data[0]) < 0 /*|| std::stoi(data[1]) < 0*/) { //checks if the row is negative, signifying the end of a block, resetting 'Header' boolean and disarding the negative values
            Header = true;
            Header_FN = 0;
            Header_T = 0;
        } else {
            std::vector<int> row_data(2); //creates a vector for the offline channel ID and adc values; the row is complete by pairing with the block frame number and timestamp
            row_data[0] = std::stoi(data[0]);
            row_data[1] = std::stoi(data[1], nullptr, 16);
            data_vector.push_back(std::make_tuple(Header_FN, Header_T, row_data));
        }
    }


}



fmt::print("TSV Parsing successful, with {} blocks with {} size\n",BN,data_vector.size());
//fmt::print("Sample row 56: ({}, {}, ({}, {})) \n", std::get<0>(data_vector[56]), std::get<1>(data_vector[56]), std::get<2>(data_vector[56])[0], std::get<2>(data_vector[56])[1]);




// Print the vector of vectors
/*
    for (const auto& row : tsv_data) {
        for (const auto& field : row) {
            fmt::print("{}\t", field);
        }
        fmt::print("\n");
    }
*/


    //ERROR CHECKS not working - need debugged

    /*
    These must occur before temporary memory transposition to ensure errors aren't erroneously spit out when checking lossy data or partial data
    */
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


//Transposition

if (P == true) { //protowib translation

// Convert temporary memory to WIBFrames and ADCs
// Create a new vector of vectors to store the CSLW numbers


//wire loop: 256; fibre loop: 4; slot loop: 10; crate: 5.

//New Method; retaining this section for future use
/*
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
*/
//End of section being used for future use

//Populating protowib data struct




 // define vector to hold WibFrame structs
 std::vector<detdataformats::wib::WIBFrame> frames;

// Loop through all blocks
for (int B = 0; B < BN; B++) {
    
    // create a WIBFrame object
    detdataformats::wib::WIBFrame wibframe;

    std::shared_ptr<detchannelmaps::TPCChannelMap> pdps1_map = detchannelmaps::make_map("ProtoDUNESP1ChannelMap");

    uint cbfr = B*(data_vector.size()/BN);

    // Get the offline channel ID from the first element of the second row
    uint offline_channel_id = std::get<2>(data_vector[cbfr])[0];

    // Initialize variables for the crate, slot, and fiber numbers
    uint crate_no, slot_no, fiber_no;
    bool found = false;

    // Loop through all possible combinations of crate, slot, and fiber numbers
    for (int crate = 1; crate < 7; crate++) { //There are 6 crates
        //fmt::print("Crate: {} \n", crate);
        for (int slot = 0; slot < 5; slot++) { //There are 5 slots
            //fmt::print("Slot: {} \n", slot);
            for (int fiber = 1; fiber < 3; fiber++) { //There are 2 fibres
                //fmt::print("Fiber: {} \n", fiber);

                // Get the offline channel ID for this combination of crate, slot, and fiber numbers
                uint ocid = pdps1_map->get_offline_channel_from_crate_slot_fiber_chan(crate, slot, fiber, 0);
                //fmt::print("CSF: {}, {}, {} - - - OCID: {} ~{}~ \n", crate, slot, fiber, offline_channel_id, ocid);
                // If the offline channel ID matches the one we're looking for, set the crate, slot, and fiber numbers
                if (ocid == offline_channel_id) {
                    crate_no = crate;
                    //fmt::print("Crate: {} ;", crate_no);
                    slot_no = slot;
                    //fmt::print("Slot: {} ;", slot_no);
                    fiber_no = fiber;
                    //fmt::print("Fiber: {}. \n", fiber_no);
                    found = true;
                }
                if (found) {
                    break;
                }
            }
            if (found) {
                break;
            }
        }
        if (found) {
            break;
        }
    }
    uint currentB = B+1;
    fmt::print("The Crate '{}', Slot '{}' and Fiber '{}' for block '{}' were isolated \n", crate_no, slot_no, fiber_no, currentB);
    

// Now 'crate', 'slot' and 'fiber' should be set to the corresponding values for 'ocid'


// struct WIBFrame {
//     uint64_t frame_number;
//     uint64_t timestamp;
//     std::array<std::array<std::array<std::array<uint16_t, 256>, 4>, 6>, 2> adc_values;
// };

// std::vector<WIBFrame> wib_data;

// for (size_t i = 0; i < temp_wib_data.size(); i += 256) {
//     WIBFrame wib_frame;
//     wib_frame.frame_number = temp_wib_data[i].frame_number;
//     wib_frame.timestamp = temp_wib_data[i].timestamp;
//     for (size_t j = i; j < i + 256; ++j) {
//         size_t wire = temp_wib_data[j].wire;
//         uint8_t board = wire / 128; 
//         uint8_t channel = (wire % 128) / 32;
//         uint8_t link = (wire % 128) % 32 / 8;
//         uint8_t sample = wire % 8;
//         wib_frame.adc_values[board][channel][link][sample * 32 + j % 32] = temp_wib_data[j].adc_values[channel][link][board][wire];
//     }
//     wib_data.push_back(wib_frame);
// }

// // Output the structs to a .out binary file

// const uint32_t WIB_HEADERSIZE = 26;
// const uint32_t WIB_CHANNELS_PER_FRAME = 256;
// const uint32_t WIB_SAMPLES_PER_CHANNEL = 112;

// struct WibFrame {
//     uint16_t crate_id;
//     uint16_t slot_id;
//     uint16_t fiber_id;
//     uint16_t wibcode;
//     uint16_t timestamp;
//     uint16_t frame_version;
//     uint16_t trig_frame;
//     uint16_t trig_sample;
//     uint16_t trig_frame_mod8;
//     uint16_t fib_fe_latency;
//     uint16_t fib_be_latency;
//     uint16_t fib_ch_latency[4];
//     uint16_t wib_ch_ctrl[4];
//     uint16_t adc[4][WIB_SAMPLES_PER_CHANNEL];
// };

    

    // populate vector with WibFrame structs
    /*
    for (int i = 0; i < 10; i++) {
        detdataformats::wib::WIBFrame frame;
        detdataformats::wib::WIBHeader* wh = frame.get_wib_header();
        wh->crate_no = 0x4;
        wh->slot_no = 0x0;
        wh->fiber_no = 0x1;
        wh->set_timestamp(0x5);
        for( size_t i(0); i<256; ++i) {
            frame.set_channel(i, i);
        }
        frames.push_back(frame);
    }
    */
   





   //HERE

    // Now you can use crate_no, slot_no, and fiber_no to create the WIBFrame object and populate it with data
    // Loop over all channels in the block
    detdataformats::wib::WIBHeader* wh = wibframe.get_wib_header();
    wh->crate_no = crate_no;
    wh->slot_no = slot_no;
    wh->fiber_no = fiber_no;
    //fmt::print("{} {} {}\n", crate_no, slot_no, fiber_no);
        int row = B*256;
        wh->set_timestamp(std::get<1>(data_vector[row]));
        //fmt::print("Timestamp: {}; \n", std::get<1>(data_vector[row]));
        for (size_t i = 0; i < 256; ++i) {
            wibframe.set_channel(i, std::get<2>(data_vector[row+i])[1]);  // +3 to skip the header columns
            }
        frames.push_back(wibframe);


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
    for (const  detdataformats::wib::WIBFrame& wibframe : frames) {
        outfile.write(reinterpret_cast<const char*>(&wibframe), sizeof(wibframe));
    }

    // close output file
    outfile.close();
        }
        
}



if (D == true) { // WIB-II translation
    std::cout << "WIB-II, dunewib data format, transposition is not yet available\n";

}

if (E == true) { // ethernet translation
    std::cout << "Ethernet, ethernet data format, transposition is not yet available\n";

}

    return 0;
}