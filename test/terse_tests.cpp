
#include "gtest/gtest.h"
#include <fstream>
#include <filesystem>
#include "Terse.hpp"
#include <numeric>

class TerseTests: public ::testing::Test {
public:

    TerseTests() = default;

};

TEST_F(TerseTests, testname1){
    std::vector<int> numbers(1000);            // Uncompressed data location
    std::iota(numbers.begin(), numbers.end(), -500);   // Fill with numbers -500, -499, ..., 499
    Terse compressed(numbers);                      // Compress the data to less than 30% of memory
    std::cout << "compression rate " << float(compressed.terse_size()) / (numbers.size() * sizeof(unsigned))
              << std::endl;
    std::ofstream outfile("junk.terse");
    outfile << compressed;                          // Write Terse data to disk
    std::ifstream infile("junk.terse");
    Terse from_file(infile);                        // Read it back in again
    std::vector<int> uncompressed(1000);
    from_file.prolix(uncompressed.begin());         // Decompress the data...
    for (int i=0; i != 5; ++i)
    std::cout << uncompressed[i] << std::endl;
    for (int i=995; i != 1000; ++i)
    std::cout << uncompressed[i] << std::endl;


}





int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
















