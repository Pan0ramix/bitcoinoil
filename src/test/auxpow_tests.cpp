// Copyright (c) 2023 The Bitcoinoil developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <test/util/setup_common.h>
#include <boost/test/unit_test.hpp>
#include <auxpow.h>
#include <primitives/block.h>
#include <chainparams.h>
#include <pow.h>
#include <validation.h>
#include <auxpow_constants.h>

BOOST_FIXTURE_TEST_SUITE(auxpow_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(auxpow_chain_id_test)
{
    // Create a block with auxpow flag set
    CBlockHeader header;
    
    // Set AuxPow version bit
    header.nVersion |= BLOCK_VERSION_AUXPOW;
    
    // Verify the block is marked as AuxPow
    BOOST_CHECK(header.IsAuxPow());
    
    // Get the chain ID from Consensus params
    const Consensus::Params& params = Params().GetConsensus();
    
    // Set the chain ID in the block version
    header.nVersion &= ~BLOCK_VERSION_CHAIN_ID_MASK; // Clear the chain ID bits
    header.nVersion |= (params.nAuxpowChainId << 24); // Set chain ID in the high bits
    
    // Extract the chain ID from the block
    int blockChainId = header.GetChainID();
    
    // Verify it matches what we set
    BOOST_CHECK_EQUAL(blockChainId, params.nAuxpowChainId);
    
    // Test block validation logic for AuxPow
    // In a typical AuxPow setup, the block should be rejected if:
    // 1. The auxpow flag is set but we're below the activation height
    // 2. The chain ID doesn't match our chain ID
    
    // This is just a basic test to verify the chain ID functionality
    // Full AuxPow validation would require creating a valid AuxPow structure
    // with parent/child block relationship which is beyond the scope of a simple test
    
    // Log the AuxPow parameters to help with debugging
    std::cout << "AuxPow Chain ID: " << params.nAuxpowChainId << std::endl;
    std::cout << "AuxPow Start Height: " << params.nAuxpowStartHeight << std::endl;
}

BOOST_AUTO_TEST_SUITE_END() 