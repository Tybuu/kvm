#include <gtest/gtest.h>

TEST(KvmProtocolTests, SampleSanityCheck) {
  int expected_packet_size = 64;
  int actual_packet_size = 64;

  EXPECT_EQ(expected_packet_size, actual_packet_size);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
