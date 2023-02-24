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
#include <Frame.hpp> //frame constructor hierarch: frame; timestamp; offchID; adc.

#include "wib_transposer/BinaryFileReader.hpp"

#include "detdataformats/wib/WIBFrame.hpp"

//#include "detchannelmaps/HardwareMap.hpp"
#include "detchannelmaps/TPCChannelMap.hpp"

#include "logging/Logging.hpp"



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
    
    std::vector<dunedaq::flxlibs::Frame> read_tsv_file(const std::string& tsvinput) //the std::string& acts a reference to tsvinput
{ //calling function for later
    std::vector<dunedaq::flxlibs::Frame> frames;

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

    // continue with the rest of the program here
    // ...


 /* const fs::path file_path(tsvinput);
  if (! fs::exists(file_path)) {

    fmt::print("ERROR: File {} does not exist.\n", tsvinput);
    exit(-1);

  }
*/    
    
//if (isblank(wibformat)){
  // wibformat = P;
    


    //Unpacking tsv ********
struct Frame {
    int frame;
    long long timestamp;
    std::vector<int> adc_values;
        };

    std::vector<Frame> read_tsv_file(const std::string& filename) {
    std::ifstream input_file(filename);
    if (!input_file) {
        throw std::runtime_error("Failed to open input file");
    }

    std::vector<Frame> frames;
    int frame = 0;
    long long timestamp = 0;
    std::string line;
    while (std::getline(input_file, line)) {
        std::istringstream line_stream(line);
        int ocid, adc_value;
        if (line_stream >> ocid >> std::hex >> adc_value) {
            if (ocid < 0 && adc_value < 0) { // end of frame (marked by negative elements)
                frames.push_back({frame, timestamp, {}});
                ++frame;
                timestamp = 0;
            } else if (ocid < 0 || adc_value < 0) {
                throw std::runtime_error("Invalid block termination - incompatible elements: " + line);
            } else {
                if (frames.empty()) {
                    throw std::runtime_error("Missing frame header");
                }
                auto& adc_values = frames.back().adc_values;
                if (adc_values.size() <= ocid) {
                    adc_values.resize(ocid + 1);
                }
                adc_values[ocid] = adc_value;
            }
        } else if (!line_stream.eof()) {
            throw std::runtime_error("Invalid input line: " + line);
        }
    }
    if (!frames.empty() && frames.back().frame_number != frame - 1) {
        throw std::runtime_error("Missing frame(s)");
    }
    std::string last_line;
    if (!std::getline(input_file, last_line) || last_line.empty() || last_line[0] != 'E') { //this makes sure that the file is complete and there is no missing/buffered data
        throw std::runtime_error("Block information is incomplete, file may be missing data - please ensure you have the full file");
    }

    return frames;
}







    //PROTOWIB Output Format
if ((D == false && E == false) || P == true) { // Protowib is the standard translation
    std::cout << "proto-working"; // convert to protowib; temporary check for breaks

   
   
    std::string proto_output_file_name;
    if (D == true || E == true) {
        proto_output_file_name = binoutput + "_WIB-I.out";
    } else {
        proto_output_file_name = binoutput + ".out";
    }
    
    //std::string finp = (tsvinput + ".tsv");
    std::string proto_input_file_name = finp;

    std::ifstream input_file(proto_input_file_name); //file checks
    if (!input_file.is_open()) {
        fmt::print("Unable to open the input file: {}\n", proto_input_file_name);
        exit(-1);
    }

    std::shared_ptr<dunedaq::detchannelmaps::TPCChannelMap> proto-channelMap = dunedaq::detchannelmaps::make_map("ProtoDUNESP1ChannelMap");
    //channelMap.loadOnlineChannelMap("path/to/online/map.tsv");
/*Error - vector<data> can't accept two int arguments
    std::vector<Data> data;
    int slotID; // slot ID
    int channelID; // channel ID
    double adc; // adc value
*/


//Add for loop for CSL mapping - OCID matching

    class Data {
    public:
        int ocid; //offline channel ID
    }


    while (input_file >> slotID >> channelID >> adc) {
        auto ocid = channelMap.getOfflineChannel(slotID, channelID);
        data.push_back({ ocid, adc });
    }

    std::ofstream output_file(proto_output_file_name, std::ios::binary);
    if (!output_file.is_open()) {
        std::cerr << "Failed to open output file: " << proto_output_file_name << std::endl;
        return 1;
    }

    for (const auto& d : data) {
        output_file.write(reinterpret_cast<const char*>(&d), sizeof(d));
    }
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