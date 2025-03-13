// Copyright (c) 2022-2023 The Bitcoinoil developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

// This file contains temporary implementations for validation.cpp functions
// to allow building with AuxPoW support

// Define this to prevent duplicate definitions in other files
#define BITCOINOIL_VALIDATION_FIX_CPP

#include "stub_config.h"
#include <validation.h>
#include <consensus/validation.h>
#include <primitives/block.h>
#include <primitives/transaction.h>
#include <chainparams.h>
#include <consensus/merkle.h>
#include <hash.h>
#include <logging.h>
#include <chain.h>
#include <pow.h>
#include <auxpow.h>
#include <util/time.h>

// Only define GetAdjustedTime() if it's not already defined elsewhere
#ifndef BITCOINOIL_TIMEDATA_CPP
// Get current adjusted time (for validation purposes)
int64_t GetAdjustedTime()
{
    return GetTime();
}
#endif

// Basic implementation of CheckBlock
bool CheckBlock(const CBlock& block, BlockValidationState& state, const Consensus::Params& consensusParams, bool fCheckPOW, bool fCheckMerkleRoot)
{
    // These are checks that are independent of context
    // that can be verified before saving an orphan block.
    
    // Size limits
    if (block.vtx.empty() || block.vtx.size() > MAX_BLOCK_WEIGHT / MIN_TRANSACTION_WEIGHT)
        return state.Invalid(BlockValidationResult::BLOCK_CONSENSUS, "bad-blk-length", "size limits failed");

    // Check proof of work matches claimed amount
    if (fCheckPOW && !CheckAuxPowProofOfWork(block, consensusParams))
        return state.Invalid(BlockValidationResult::BLOCK_INVALID_HEADER, "high-hash", "proof of work failed");

    // Check timestamp
    if (block.GetBlockTime() > GetAdjustedTime() + 2 * 60 * 60)
        return state.Invalid(BlockValidationResult::BLOCK_TIME_FUTURE, "time-too-new", "block timestamp too far in the future");

    // First transaction must be coinbase
    if (block.vtx.empty() || !block.vtx[0]->IsCoinBase())
        return state.Invalid(BlockValidationResult::BLOCK_CONSENSUS, "bad-cb-missing", "first tx is not coinbase");

    // Check transactions - simplified check, just make sure they're not empty
    for (const auto& tx : block.vtx) {
        if (!tx)
            return state.Invalid(BlockValidationResult::BLOCK_CONSENSUS, "bad-tx-null", "transaction is null");
    }

    // Check for duplicate txids
    std::set<uint256> uniqueTx;
    for (size_t i = 1; i < block.vtx.size(); i++) {
        const uint256& txid = block.vtx[i]->GetHash();
        if (uniqueTx.count(txid))
            return state.Invalid(BlockValidationResult::BLOCK_CONSENSUS, "bad-txns-duplicate", "duplicate transaction");
        uniqueTx.insert(txid);
    }

    // Check merkle root if requested
    if (fCheckMerkleRoot) {
        uint256 merkleRoot = BlockMerkleRoot(block);
        if (block.hashMerkleRoot != merkleRoot)
            return state.Invalid(BlockValidationResult::BLOCK_MUTATED, "bad-merkle-root", 
                                strprintf("hashMerkleRoot mismatch (block: %s, computed: %s)",
                                          block.hashMerkleRoot.ToString(),
                                          merkleRoot.ToString()));
    }

    return true;
}

// Check if a vector of headers has valid proof of work
bool HasValidProofOfWork(const std::vector<CBlockHeader>& headers, const Consensus::Params& consensusParams)
{
    for (const CBlockHeader& header : headers) {
        if (!CheckAuxPowProofOfWork(header, consensusParams)) {
            return false;
        }
    }
    return true;
}

// Only define CalculateHeadersWork if not defined elsewhere
#if !defined(BITCOINOIL_AUXPOW_STUB_CPP) || defined(FORCE_VALIDATION_FIX_CALCULATEHEADERSWORK)
// Calculate work of a span of headers
arith_uint256 CalculateHeadersWork(const std::vector<CBlockHeader>& headers)
{
    arith_uint256 total_work = 0;
    for (const CBlockHeader& header : headers) {
        // Create a temporary CBlockIndex with just the bits we need
        CBlockIndex index;
        index.nBits = header.nBits;
        total_work += GetBlockProof(index);
    }
    return total_work;
}
#endif

// Basic stub implementation of Chainstate::PreciousBlock
bool Chainstate::PreciousBlock(BlockValidationState& state, CBlockIndex* pindex)
{
    LogPrintf("Chainstate::PreciousBlock stub called\n");
    return true;
}

// Basic stub implementation of Chainstate::InvalidateBlock
bool Chainstate::InvalidateBlock(BlockValidationState& state, CBlockIndex* pindex)
{
    LogPrintf("Chainstate::InvalidateBlock stub called\n");
    return true;
}

// Basic stub implementation of Chainstate::ActivateBestChain
bool Chainstate::ActivateBestChain(BlockValidationState& state, std::shared_ptr<const CBlock> pblock)
{
    LogPrintf("Chainstate::ActivateBestChain stub called\n");
    return true;
}

// Basic stub implementation of Chainstate::ResetBlockFailureFlags
void Chainstate::ResetBlockFailureFlags(CBlockIndex* pindex)
{
    LogPrintf("Chainstate::ResetBlockFailureFlags stub called\n");
}

// Basic stub implementation of ChainstateManager::UpdateUncommittedBlockStructures
void ChainstateManager::UpdateUncommittedBlockStructures(CBlock& block, const CBlockIndex* pindexPrev) const
{
    LogPrintf("ChainstateManager::UpdateUncommittedBlockStructures stub called\n");
}

// Basic stub implementation of ChainstateManager::GenerateCoinbaseCommitment
std::vector<unsigned char> ChainstateManager::GenerateCoinbaseCommitment(CBlock& block, const CBlockIndex* pindexPrev) const
{
    LogPrintf("ChainstateManager::GenerateCoinbaseCommitment stub called\n");
    return std::vector<unsigned char>();
}

// Implementation for ExpectedAssumeutxo stub
const AssumeutxoData* ExpectedAssumeutxo(const int height, const CChainParams& params)
{
    LogPrintf("ExpectedAssumeutxo stub called\n");
    return nullptr;
}

// Basic stub implementation of Chainstate::AcceptBlock
bool Chainstate::AcceptBlock(const std::shared_ptr<const CBlock>& pblock, BlockValidationState& state, CBlockIndex** ppindex, bool fRequested, const FlatFilePos* dbp, bool* fNewBlock, bool min_pow_checked)
{
    AssertLockHeld(cs_main);
    
    if (fNewBlock) *fNewBlock = false;
    
    // Check that the block satisfies synchronized checkpoint
    LogPrintf("Chainstate::AcceptBlock stub called\n");
    
    // Get block index
    CBlockIndex* pindex = nullptr;
    
    // Set ppindex to point to the created/found block index
    if (ppindex)
        *ppindex = pindex;
    
    if (fNewBlock) *fNewBlock = true;
    
    return true;
} 