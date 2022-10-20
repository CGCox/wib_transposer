#include "wib_transposer/BinaryFileReader.hpp"

#include <iostream>
#include <fmt/core.h>

namespace dunedaq {
namespace wib_transposer {

//-----------------------------------------------------------------------------
BinaryFileReader::BinaryFileReader(const std::string &path) : m_path(path) {
  m_file.open(m_path, std::ios::binary);
  if (!m_file.is_open()) {
    // throw something here
    throw std::runtime_error("failed to open file");
  }

  m_file.seekg(0, std::ios_base::end);
  m_size = m_file.tellg();
  m_file.seekg(0, std::ios_base::beg);

  // std::cout << "Opened " << m_path << " size " << m_size << std::endl;
}

//-----------------------------------------------------------------------------
BinaryFileReader::~BinaryFileReader() {
  if (m_file.is_open()) m_file.close();
}

//-----------------------------------------------------------------------------
BinaryDataBlock BinaryFileReader::read_block(size_t size, size_t offset) {
  if (offset > m_size) {
    throw std::runtime_error(fmt::format("Requestes offset than file size - offset={}, file size={}", offset, size));
  }

  if (offset + size > m_size) {
    std::cout << "WARNING: file shorter than requested size. file size="
              << m_size << " offset+size=" << offset + size << std::endl;
    std::cout << "WARNING: file shorter than requested size. New size: "
              << m_size - offset << std::endl;
    size = m_size - offset;
    // throw something
  }

  m_file.seekg(offset);

  BinaryDataBlock block(size);
  m_file.read(block.m_data.get(), block.m_size);

  return block;
}

}  // namespace wib_transposer
}  // namespace dunedaq
