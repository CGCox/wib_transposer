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

#include "wib_transposer/BinaryFileReader.hpp"
#include "detdataformats/wib/WIBFrame.hpp"

namespace fs = std::filesystem;
using namespace dunedaq;

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

  wib_transposer::BinaryFileReader bfr(filename);
  fmt::print("Found {} of size {} \n", bfr.get_path(), bfr.get_size());

  constexpr size_t protowib_frame_size = sizeof(detdataformats::wib::WIBFrame);

  constexpr uint32_t n_frames = 2;
  fmt::print("Reading {} frames of size {} bytes \n", n_frames, protowib_frame_size);

  auto block = bfr.read_block(protowib_frame_size);

  for (uint32_t i(0); i<n_frames; ++i ) {

    detdataformats::wib::WIBFrame *frame = reinterpret_cast<detdataformats::wib::WIBFrame *>(block.data());
    auto* header = frame->get_wib_header();

    fmt::print("---Printing WIB information - frame {}\n", i);

    fmt::print("{}\n", frame->get_timestamp());

    fmt::print("fiber: {} crate: {} slot: {}\n", (uint8_t)header->fiber_no, (uint8_t)header->crate_no, (uint8_t)header->slot_no);
    for (uint16_t i(0); i<256; ++i) {
      fmt::print("{:03} {:03x} {:4d}\n", i, frame->get_channel(i), frame->get_channel(i));
    }
  }

  return 0;
}