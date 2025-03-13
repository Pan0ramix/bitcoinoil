// Copyright (c) 2022-2023 The Bitcoinoil developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

// This file contains minimal stubs needed for linking AuxPow functionality

// Define this to prevent duplicate definitions in validation_fix.cpp
#define BITCOINOIL_AUXPOW_STUB_CPP

#include "stub_config.h"
#include <validation.h>
#include <logging.h>
#include <chain.h>
#include "auxpow.h"
#include <hash.h>

// Stub implementation - not needed for AuxPow functionality
bool IsBIP30Repeat(const CBlockIndex& block_index) {
    LogPrintf("IsBIP30Repeat stub called\n");
    return false;
}

bool IsBIP30Unspendable(const CBlockIndex& block_index) {
    LogPrintf("IsBIP30Unspendable stub called\n");
    return false;
}

bool TestBlockValidity(BlockValidationState& state, const CChainParams& chainparams, Chainstate& chainstate, const CBlock& block, CBlockIndex* pindexPrev, 
                      const std::function<NodeClock::time_point()>& adjusted_time_callback, bool fCheckPOW, bool fCheckMerkleRoot) {
    LogPrintf("TestBlockValidity stub called\n");
    return true;
}

// We'll let validation_fix.cpp define this function
// arith_uint256 CalculateHeadersWork(const std::vector<CBlockHeader>& headers) {
//     LogPrintf("CalculateHeadersWork stub called\n");
//     return arith_uint256();
// }

void PruneBlockFilesManual(Chainstate& active_chainstate, int nManualPruneHeight) {
    LogPrintf("PruneBlockFilesManual stub called\n");
}

double GuessVerificationProgress(const ChainTxData& data, const CBlockIndex* pindex) {
    LogPrintf("GuessVerificationProgress stub called\n");
    return 1.0;
}

// Just enough implementation for the AuxPow functions to link
namespace node {
    class SnapshotMetadata;
} 

// Explicit template instantiations are now in auxpow.cpp 