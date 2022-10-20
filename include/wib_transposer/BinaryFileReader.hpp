
#ifndef DUNEDAQ_WIBTRANSPOSER_BINARYFILEREADER_HPP_
#define DUNEDAQ_WIBTRANSPOSER_BINARYFILEREADER_HPP_

#include <string>
#include <memory>
#include <fstream>

namespace dunedaq
{
  namespace wib_transposer
  {

    class BinaryDataBlock
    {
    public:
      

      BinaryDataBlock(size_t size) : m_size(size)
      {
        m_data = std::make_unique<char[]>(size);

      }
      ~BinaryDataBlock()
      {
        // delete[] m_data;
      }

      BinaryDataBlock(BinaryDataBlock&& ) = default;
      BinaryDataBlock& operator=(BinaryDataBlock&& ) = default; // not needed for the example, but good for completeness


      char* data() { return m_data.get(); }
      size_t size() { return m_size; }

    private:
      // char *m_data;
      std::unique_ptr<char[]> m_data;
      size_t m_size;

      friend class BinaryFileReader;
    };

    class BinaryFileReader
    {
    public:
      BinaryFileReader(const std::string &path);
      ~BinaryFileReader();

      size_t get_size() const { return m_size; }
      const std::string& get_path() const { return m_path; }

      BinaryDataBlock read_block(size_t size, size_t offset = 0);

    private:
      std::string m_path;
      size_t m_size;

      std::ifstream m_file;
    };

  }
}

#endif /* DUNEDAQ_WIBTRANSPOSER_BINARYFILEREADER_HPP_ */