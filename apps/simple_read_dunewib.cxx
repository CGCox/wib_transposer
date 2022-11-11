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


 }


}