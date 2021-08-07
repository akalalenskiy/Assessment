#include "pch.h"

TEST(TestCaseName, TestName) {
    //auto circularQueue = std::make_shared<QueueBase<std::string, CircularQueue<std::string>>>();
  EXPECT_EQ(1, 1);
  EXPECT_TRUE(true);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    RUN_ALL_TESTS();

    std::cout << "Hello World!\n";
}