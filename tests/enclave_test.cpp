#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "secure_vault.h"

TEST(AxiomEngine, EnclaveSealing) {
  AXIOM::SecureMantisVault vault;
  EXPECT_TRUE(vault.initialize_enclave());

  std::string secret_data = "SuperSecretMantisState123";

  // Mühürleme (Seal) işlemi
  EXPECT_TRUE(vault.seal(secret_data.data(), secret_data.size()));

  // Yanlış boyutta mühür açma (Unseal) girişimi
  std::vector<char> wrong_buffer(10);
  EXPECT_FALSE(vault.unseal(wrong_buffer.data(), wrong_buffer.size()));

  // Doğru mühür açma (Unseal) işlemi
  std::vector<char> unsealed(secret_data.size());
  EXPECT_TRUE(vault.unseal(unsealed.data(), unsealed.size()));

  std::string recovered(unsealed.begin(), unsealed.end());
  EXPECT_EQ(secret_data, recovered);
}
