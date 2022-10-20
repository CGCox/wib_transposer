/**
 * @file simple_read_protowib_write_dunewib.cxx
 *
 * Developer(s) of this DAQ application have yet to replace this line with a brief description of the application.
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "CLI/CLI.hpp"

#include <fmt/core.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <chrono>

#include "detdataformats/wib/WIBFrame.hpp"

namespace fs = std::filesystem;


int main(int argc, char **argv) {
  CLI::App app;

  // Add new options/flags here
  std::string filename;
  app.add_option("filename", filename, "name of the wib file")->required();

  CLI11_PARSE(app, argc, argv);

  const fs::path file_path(filename);
  if (! fs::exists(file_path)) {

    fmt::print("ERROR: file {} does not exist.\n", filename);
    exit(-1);

  }

  std::ifstream ifile;
  ifile.open(filename, std::ios::binary);
  if ( !ifile.is_open() ) {
    // throw something here
    throw std::runtime_error(fmt::format("failed to open file {}", filename));
  }

  // Reead file size
  ifile.seekg(0, std::ios_base::end);
  size_t ifsize = ifile.tellg();
  ifile.seekg(0, std::ios_base::beg);

  // Prepare data block (use a unique pointer as container)
  size_t block_size = 1024; // 1 kB
  auto block_data = std::make_unique<char[]>(block_size);

  // Populate the block
  ifile.read(block_data.get(), block_size);

  dunedaq::detdataformats::wib::WIBFrame *frame = reinterpret_cast<dunedaq::detdataformats::wib::WIBFrame *>(block_data.get());
  auto* header = frame->get_wib_header();

  fmt::print("Printing WIB information\n");

  // std::cout << *frame << std::endl;

  fmt::print("{}\n", frame->get_timestamp());

  fmt::print("fiber: {} crate: {} slot: {}\n", (uint8_t)header->fiber_no, (uint8_t)header->crate_no, (uint8_t)header->slot_no);
  for (uint16_t i(0); i<256; ++i) {
    fmt::print("{:03} {:03x} {:4d}\n", i, frame->get_channel(i), frame->get_channel(i));
  }
  fmt::print("The answer is {}.\n", filename);

  return 0;
}