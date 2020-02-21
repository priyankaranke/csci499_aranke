#include "gtest/gtest.h"

#include "func.h"

// The fixture for testing class KeyValueStoreBackend.
class FuncTest : public ::testing::Test {
 protected:
   FuncTest();

   // You can do clean-up work that doesn't throw exceptions here.
   virtual ~FuncTest();

   // If the constructor and destructor are not enough for setting up 
   // and cleaning up each test, you can define the following methods:

   // Code here will be called immediately after the constructor (right
   // before each test).
   virtual void SetUp();

   // Code here will be called immediately after each test (right
   // before the destructor).
   virtual void TearDown();

   Func func;
};