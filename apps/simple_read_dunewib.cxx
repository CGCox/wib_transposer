/* Simple Binary File Reader for DUNE WIB data (WIB-II)

The purpose of this binary file reader is a task to beter understand C++

The goal is to be able to read, breakdown and translate WIB-II data

Modelled after the simple protowib binary reader by Alessandro Thea

*Note to self: TURN ON AUTOSAVE*
*/

#include "CLI/CLI.hpp" //Needs Clarity

//Adding relevant libraries
#include <fmt/core.h> // C++ formatting library
#include <filesystem> // Provides the foundation for operations on files and their constituent components e.g. repositories, paths etc.
#include <fstream> // High level input/output operations; allowing for operations on files
#include <iostream> // Takes on all components from 'istream' and 'ostream' performing both input and output operations
// fstream -> iostream; effectively makes a storage with an inherent initialiser and flusher
#include <chrono> //Timing libraries

#include "detdataformats/wib2/WIB2Frame.hpp" //contains classes for accessing raw wib2 data with definitive bindings
//access more definitions and bindings at https://edms.cern.ch/document/2088713/4


//defining namespaces
namespace fs = std::filesystem; //adds the filesystem header into a shorthand variable for namespace
using namespace dunedaq; //adds dunedaq to the global namespace to reduce line length and improve simplicity

//effective start of program
int main(int argc, char** argv) {
/*global function 'main' at start of program
DEFINITIONS OF ELEMENTS
argc is the number of arguments passed to the program
argv pointer to the first element 'argc+1', pointing to a string that reprsents the name invoked in the program, otherwise an empty string
char ** points at an array of pointers to the execution environment variables

*/
 CLI::App app; //Need Clarity

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

// from standard ifstream namespace - creates an empty variables
    std::ifstream ifile;
    ifile.open(filename, std::ios::binary); //Uses ifile library to provide open command;
   //must always check that file is open; below, is the outcome if file is NOT open
    if ( !ifile.is_open() ) {
        throw std::runtime_error(fmt::format("file couldn't be opened {}", filename));
    }

    //Now we look at the file size
    ifile.seekg(0, std::ios_base::end); //sets the initial position
    // size_t ifsize = ifile.tellg(); //requires absolute position from tellg. Not necessary here
    ifile.seekg(0, std::ios_base::beg); //set the final position at the beginning; effectively resetting

    // prepares data block, using a unique pointer to stand in for a container.
    size_t block_size = 1024; // 1kB
    auto block_data = std::make_unique<char[]>(block_size); //created the container block_size of size 1024

    // populates container (fills the block with the first kB of data)
    ifile.read(block_data.get(), block_size);
    detdataformats::wib2::WIB2Frame *frame = reinterpret_cast<detdataformats::wib2::WIB2Frame *>(block_data.get());
    fmt::print("Outputting DUNE WIB information\n");

   fmt::print("crate: {} fiber: {} slot: {}\n", (uint8_t)frame->header.crate, (uint8_t)frame->header.link, (uint8_t)frame->header.slot); //frame points to header struct which hosts the variables we want. 
    for (uint16_t i(0); i<256; ++i) { //fiber is now referred to as link
        fmt::print("{:03} {:03x} {:4d}\n", i, frame->get_adc(i), frame->get_adc(i)); //prints channel, then adc value in hex and decimal respectively
    }
    fmt::print("Output is {}.\n", filename);

   return 0;
}
//simple_read_dunewib /nfs/rscratch/username/WIB2.out for the data