/* Purpose of this program is to read blocks of WIB2 data

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


//---- Extracting the data


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
  //constexpr uint32_t nframes  




  //---- User Input & Checks

  //Goal: Name of the text file to save data to (e.g. test.txt or 'this file name already exists')
  //Number of frames to be included (must be positive integer)
  
  //void n_frames(int n_frames)
  //std::cout<< "Enter a (positive integer) number for how many blocks you wish to read: "<< endl;
  //constexpr uint32_t n_frames{}; //initialises variables n_frames set at 0
  //std::cin >> n_frames; //defines n_frames as the input from cout
  //constexpr uint32_t n_frames = 2; //preset variable for simplicity

    int n_frames;
    fmt::print("Please enter the (positive integer) number of blocks you wish to read: ");
    std::cout <<'\n';
    std::getline(std::cin,n_frames);
    
    if (!(n_frames>0)) {
        throw std::runtime_error(fmt::format("Your value must be positive"));
        case n_frames>0:
        {
        exit(-1);
        }
        else
        {

        }
    }
     if (!(floor(n_frames) == n_frames)) {
        throw std::runtime_error(fmt::format("Your value must be whole"));
        case floor(n_frames) == n_frames:
        {
        exit(-1);
        }
        else
        {

        }
    }
    //For ensuring program doesn't overwork itself nor accept wrong input
    


    std::string FNSTORE;
    std::string FN;
    std::string FNcheck;
    std::cout << "What would you like the textfile to be called?\n";
    std::getline(std::cin, FNcheck);
    std::cin >> FNSTORE;
    fmt::print("FN is {}\n", FN);
    fmt::print("FNcheck is{}\n", FNcheck);

    if (FN==FNcheck){

    }
    else
    {
      std::cout << "Please do not include spaces in the file name\n";
      exit(-1);
    }

  
  fmt::print("Reading {} frames of size {} bytes \n", n_frames, dune_frame_size); //reads x number of frames of size y

  auto block = bfr.read_block(n_frames * dune_frame_size); //creates a container equating to the size of the frame (roughly 1024/1kB generally) times the number of blocks you wish to read



//---- Printing data


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


  return 0;
}

//block_read_dunewib /nfs/rscratch/username/WIB2.out for the data