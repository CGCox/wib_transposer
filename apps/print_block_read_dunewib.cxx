/* Purpose of this program is to read blocks of WIB2 data and print to file

Expanding upon the simple dune wib reader and the proto wib block reader

This should be understandable to someone who is newer to c++ by explaining each components definition too
*/

//Including fundamental files
#include "CLI/CLI.hpp" //this is a command line interface in order to interpret lines of code

//Including libraries for code
#include <fmt/core.h> // C++ formatting library
#include <filesystem> // Provides the foundation for operations on files and their constituent components e.g. repositories, paths etc.
#include <fstream> // High level input/output operations; allowing for operations on files
#include <iostream> // Takes on all components from 'istream' and 'ostream' performing both input and output operations
// fstream -> iostream; effectively makes a storage with an inherent initialiser and flusher
#include <chrono> //Timing libraries
#include <string> //For using strings

#include "wib_transposer/BinaryFileReader.hpp" //allows for block reading, advancing on simple reader
#include "detdataformats/wib2/WIB2Frame.hpp"//contains classes for accessing raw wib2 data with definitive bindings
//access more definitions and bindings at https://edms.cern.ch/document/2088713/4


//---- Initial code

//namespaces
namespace fs = std::filesystem; //creates a shorthand for the filesystem namespace
using namespace dunedaq; //adds dunedaq to the global namespace to save time

inline std::string trim(std::string& str)
{
    str.erase(str.find_last_not_of(' ')+1);         //suffixing spaces
    str.erase(0, str.find_first_not_of(' '));       //prefixing spaces
    return str;
}

int main(int argc, char ** argv) {
    CLI::App app;
/*
global function 'main' at start of program
DEFINITIONS OF ELEMENTS
argc is the number of arguments passed to the program
argv pointer to the first element 'argc+1', pointing to a string that reprsents the name invoked in the program, otherwise an empty string
char ** points at an array of pointers to the execution environment variables
*/

// here we add options if incuding the filename (this must be required)

//Adds new options/flags - Need Clarity
std::string filename; //standard 'std' namespace string which gives the file name component of the path
app.add_option("filename", filename, "name of the wib file")->required();

CLI11_PARSE(app, argc, argv) //Parse translates each element such that, element 1=app, element 2=argc and element 3=argv.
 
 
 const fs::path file_path(filename); //gives the path for the given file
 if (! fs::exists(file_path)) {
//! Makes this an if "not" 

    fmt::print("ERROR: can't locate file; try checking the name?\n", filename); //prints out when the file, as input, doesn't exist
    exit(-1); /*exit success is exit(0); exit failure is exit(1), but can still be completed; whereas exit(-1) is the command can't be completed:
 EXAMPLE - Terminating Program
 exit(0) = Program was completed successfully
 exit(1) = Program was terminated abnormally
 exit(-1) = Program could not be terminated
 */

 }

//---- Extracting the Data
 
// from standard ifstream namespace - creates an empty variables
    std::ifstream ifile;
    ifile.open(filename, std::ios::binary); //Uses ifile library to provide open command;
   //must always check that file is open; below, is the outcome if file is NOT open
    if ( !ifile.is_open() ) {
        throw std::runtime_error(fmt::format("file couldn't be opened {}", filename));
    }

//separate binary file reader program from within wib transposer repo
  wib_transposer::BinaryFileReader bfr(filename);
  fmt::print("Found {} of size {} \n", bfr.get_path(), bfr.get_size());

  constexpr size_t dune_frame_size = sizeof(detdataformats::wib2::WIB2Frame); //creates a constant expression for the wib frame (how big each frame, or block, is going to be) 
 
 

 //---- User Input & Checks

  fmt::print("How many frames do you wish to readout?\n");
  std::string line;
  std::getline(std::cin, line);
  int n_frames;
 
 size_t pos;
  try {
        n_frames = std::stoi(trim(line), &pos);
      } catch(std::invalid_argument const& ex)
      {
        fmt::print("Invalid input number; please ensure it is a positive integer\n");
        exit(-1);
      }

      if ( pos != line.size()) {
        fmt::print("Invalid input number; please ensure it is a positive integer\n {} {}", pos, line.size());
        exit(-1);
      }

      if ( (n_frames <= 0)) {
        fmt::print("Your input of '{}' for the number of frames that you wish to read is invalid.\nPlease ensure it is a positive integer.\n",n_frames);
        exit(-1);
      }

  fmt::print("What would you like to name the textfile?\n");
  std::string NTest;
  std::getline(std::cin, NTest);

  if (NTest.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890_-.*") != std::string::npos)
{
  std::cerr << "Please do not use special characters which won't be accepted in text file names.\n";

}


  fmt::print("Reading {} frames of size {} bytes \n", n_frames, dune_frame_size); //reads x number of frames of size y

  auto block = bfr.read_block(n_frames * dune_frame_size); //creates a container equating to the size of the frame (roughly 1024/1kB generally) times the number of blocks you wish to read

  //printing data

  std::ofstream MyFile("{}.txt\n", FNcheck);


  for (uint32_t i(0); i<n_frames; ++i ) {
    ifile.read(block.data(), dune_frame_size);
    detdataformats::wib2::WIB2Frame *frame = reinterpret_cast<detdataformats::wib2::WIB2Frame *>(block.data()+i*dune_frame_size);
    fmt::print("Outputting DUNE WIB frame {} information\n", i);

    fmt::print("{}\n", frame->get_timestamp());

    fmt::print("crate: {} fiber: {} slot: {}\n", (uint8_t)frame->header.crate, (uint8_t)frame->header.link, (uint8_t)frame->header.slot);
    for (uint16_t i(0); i<256; ++i) {
      fmt::print("{:03}, {:03x}, {:4d}\n", i, frame->get_adc(i), frame->get_adc(i));
    }
  }
  fmt::print("Output is {}.\n", filename);

  
  //writing data to text file

  std::ofstream out;
  out.open("{}.txt",NTest);

  cin >> fmt::print("Outputting DUNE WIB frame {} information\n", i);

  cin >> fmt::print("{}\n", frame->get_timestamp());

  cin>> fmt::print("crate: {} fiber: {} slot: {}\n", (uint8_t)frame->header.crate, (uint8_t)frame->header.link, (uint8_t)frame->header.slot);
    
  for (uint16_t i(0); i<256; ++i) {
    cin >>  fmt::print("{:03}, {:03x}, {:4d}\n", i, frame->get_adc(i), frame->get_adc(i));

 }

std::out.close();

return 0;
}
