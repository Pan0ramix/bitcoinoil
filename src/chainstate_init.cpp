// Copyright (c) 2022-2023 The Bitcoinoil developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

// This file handles the initialization of ChainstateManager

// Define this to prevent duplicate definitions in chainstate_stubs.cpp
#define BITCOINOIL_CHAINSTATE_INIT_CPP

#include <validation.h>
#include <node/blockstorage.h>
#include <logging.h>
#include <kernel/chainstatemanager_opts.h>
#include <kernel/chain.h>
#include <chainparams.h>
#include <primitives/block.h>
#include <primitives/transaction.h>
#include <consensus/validation.h>
#include <util/time.h>
#include <pow.h>
#include <auxpow.h>

// Instead of static initialization with Params(), we'll lazy-initialize when needed
static const CChainParams* g_chainparams_ptr = nullptr;
static std::unique_ptr<kernel::ChainstateManagerOpts> g_chainstate_opts_ptr;
static kernel::BlockManagerOpts g_blockman_opts{
    .prune_target = 0,
};

// Forward declaration for ContextualCheckBlock (defined in validation.cpp)
static bool ContextualCheckBlock(const CBlock& block, BlockValidationState& state, const Consensus::Params& consensusParams, const CBlockIndex* pindexPrev);

// Forward declarations
int64_t GetAdjustedTime();

// Simple implementation of ContextualCheckBlock
static bool ContextualCheckBlock(const CBlock& block, BlockValidationState& state, const Consensus::Params& consensusParams, const CBlockIndex* pindexPrev)
{
    // For now, just return true as a stub implementation later
    return true;
}

// Check block header for proof of work
bool CheckBlockHeader(const CBlockHeader& block, BlockValidationState& state, const Consensus::Params& consensusParams, bool fCheckPOW)
{
    // Check proof of work matches claimed amount
    if (fCheckPOW && !CheckAuxPowProofOfWork(block, consensusParams))
        return state.Invalid(BlockValidationResult::BLOCK_INVALID_HEADER, "high-hash", "proof of work failed");
    
    return true;
}

// Define and initialize the ChainstateManager constructor with proper initialization
ChainstateManager::ChainstateManager(kernel::ChainstateManagerOpts chainstate_opts, kernel::BlockManagerOpts blockman_opts)
    : m_options(chainstate_opts), 
      m_blockman(blockman_opts)
{
    LogPrintf("ChainstateManager constructor initialized with stub implementation\n");
} 

// Stub implementations for Chainstate methods
void Chainstate::LoadMempool(const fs::path& load_path, fsbridge::FopenFn mockable_fopen_function) {
    LogPrintf("Chainstate::LoadMempool stub called\n");
}

bool Chainstate::LoadChainTip() {
    LogPrintf("Chainstate::LoadChainTip stub called\n");
    return true;
}

bool Chainstate::ReplayBlocks() {
    LogPrintf("Chainstate::ReplayBlocks stub called\n");
    return true;
}

void Chainstate::CheckBlockIndex() {
    LogPrintf("Chainstate::CheckBlockIndex stub called\n");
}

bool Chainstate::LoadGenesisBlock() {
    LogPrintf("Chainstate::LoadGenesisBlock stub called\n");
    return true;
}

void Chainstate::UnloadBlockIndex() {
    LogPrintf("Chainstate::UnloadBlockIndex stub called\n");
}

void Chainstate::LoadExternalBlockFile(FILE* fileIn, FlatFilePos* dbp, std::multimap<uint256, FlatFilePos>* blocks_with_unknown_parent) {
    LogPrintf("Chainstate::LoadExternalBlockFile stub called\n");
}

std::string Chainstate::ToString() {
    LogPrintf("Chainstate::ToString stub called\n");
    return "Stub Chainstate";
}

bool Chainstate::NeedsRedownload() const {
    LogPrintf("Chainstate::NeedsRedownload stub called\n");
    return false;
}

// Implementation of ProcessNewBlock
bool ChainstateManager::ProcessNewBlock(const std::shared_ptr<const CBlock>& block, bool force_processing, bool min_pow_checked, bool* new_block)
{
    LogPrintf("Processing new block %s\n", block->GetHash().ToString());
    
    AssertLockNotHeld(cs_main);
    
    if (new_block) *new_block = false;
    
    BlockValidationState state;
    
    // Check block header first
    {
        LOCK(cs_main);
        
        // Check if block has already been processed
        const CBlockIndex* pindex = m_blockman.LookupBlockIndex(block->GetHash());
        if (pindex) {
            LogPrint(BCLog::VALIDATION, "Block %s already known\n", block->GetHash().ToString());
            if (new_block) *new_block = false;
            return true;
        }
        
        // Check block header
        if (!CheckBlockHeader(*block, state, GetConsensus(), true)) {
            LogPrintf("Invalid block header %s: %s\n", block->GetHash().ToString(), state.ToString());
            return false;
        }
        
        // Get the previous block index
        CBlockIndex* pindexPrev = nullptr;
        if (!block->hashPrevBlock.IsNull()) {
            pindexPrev = m_blockman.LookupBlockIndex(block->hashPrevBlock);
            if (!pindexPrev) {
                LogPrintf("Block %s has unknown parent %s\n", 
                          block->GetHash().ToString(),
                          block->hashPrevBlock.ToString());
                if (new_block) *new_block = false;
                return false;
            }
        }
        
        // Perform context-dependent validation checks, including AUX-POW
        if (!ContextualCheckBlock(*block, state, GetConsensus(), pindexPrev)) {
            LogPrintf("Failed contextual check for block %s: %s\n", 
                      block->GetHash().ToString(), 
                      state.ToString());
            return false;
        }
        
        // Check full block validity
        if (!CheckBlock(*block, state, GetConsensus(), true, true)) {
            LogPrintf("Invalid block %s: %s\n", block->GetHash().ToString(), state.ToString());
            return false;
        }
        
        // Add block to index
        CBlockIndex* pindexNew = nullptr;
        bool accepted = AcceptBlockHeader(*block, state, GetParams(), &pindexNew);
        if (!accepted) {
            LogPrintf("Block header %s not accepted: %s\n", 
                      block->GetHash().ToString(), 
                      state.ToString());
            return false;
        }
        
        if (new_block) *new_block = true;
        
        // Record the new block for future reference
        LogPrintf("New block accepted: %s prev=%s height=%d\n",
                  block->GetHash().ToString(),
                  block->hashPrevBlock.ToString(),
                  pindexNew->nHeight);
    }
    
    // Note: We intentionally don't attempt to process and connect the block here
    // since this is a simplified implementation. In a complete implementation,
    // you would call ConnectBlock, UpdateTip, etc.
    
    return true;
} 

// Implementation of AcceptBlockHeader
bool ChainstateManager::AcceptBlockHeader(
    const CBlockHeader& block,
    BlockValidationState& state,
    const CChainParams& chainparams,
    CBlockIndex** ppindex)
{
    AssertLockHeld(cs_main);
    
    // Check for duplicate
    uint256 hash = block.GetHash();
    CBlockIndex* pindex = m_blockman.LookupBlockIndex(hash);
    
    if (pindex) {
        if (ppindex) 
            *ppindex = pindex;
        if (pindex->nStatus & BLOCK_FAILED_MASK) {
            LogPrintf("Block %s is marked invalid\n", hash.ToString());
            return state.Invalid(BlockValidationResult::BLOCK_INVALID_PREV, "duplicate-invalid");
        }
        return true;
    }
    
    // Check proof of work 
    if (!CheckProofOfWork(hash, block.nBits, GetConsensus())) {
        return state.Invalid(BlockValidationResult::BLOCK_INVALID_HEADER, "high-hash", "proof of work failed");
    }
    
    // Get the previous block index
    CBlockIndex* pindexPrev = nullptr;
    if (!block.hashPrevBlock.IsNull()) {
        pindexPrev = m_blockman.LookupBlockIndex(block.hashPrevBlock);
        if (!pindexPrev) {
            LogPrintf("Block %s has unknown parent %s\n", 
                      hash.ToString(),
                      block.hashPrevBlock.ToString());
            return state.Invalid(BlockValidationResult::BLOCK_MISSING_PREV, "prev-blk-not-found", "previous block not found");
        }
        if (pindexPrev->nStatus & BLOCK_FAILED_MASK) {
            LogPrintf("Block %s has invalid parent %s\n", 
                      hash.ToString(),
                      block.hashPrevBlock.ToString());
            return state.Invalid(BlockValidationResult::BLOCK_INVALID_PREV, "bad-prevblk", "previous block invalid");
        }
        if (!ContextualCheckBlockHeader(block, state, GetConsensus(), pindexPrev, GetAdjustedTime())) {
            LogPrintf("Block %s failed contextual check\n", hash.ToString());
            return false;
        }
    }
    
    // Check for AUX-POW blocks before the activation height
    if (block.IsAuxPow()) {
        if (pindexPrev && (pindexPrev->nHeight + 1 < GetConsensus().nAuxpowStartHeight)) {
            return state.Invalid(BlockValidationResult::BLOCK_CONSENSUS, "auxpow-not-yet-active",
                                strprintf("AuxPow blocks are not allowed before height %d (current height %d, block version 0x%08x)", 
                                          GetConsensus().nAuxpowStartHeight, pindexPrev->nHeight + 1, block.nVersion));
        }
        
        // Ensure chain ID matches expected value
        if (block.GetChainID() != GetConsensus().nAuxpowChainId) {
            return state.Invalid(BlockValidationResult::BLOCK_CONSENSUS, "wrong-chain-id",
                                strprintf("Block has wrong chain ID %d (expected %d), block version 0x%08x, block hash %s", 
                                          block.GetChainID(), GetConsensus().nAuxpowChainId, block.nVersion, block.GetHash().ToString()));
        }
    }
    
    // Construct new block index object
    CBlockIndex* best_header = nullptr;
    pindex = m_blockman.AddToBlockIndex(block, best_header);
    if (ppindex)
        *ppindex = pindex;
    
    return true;
}

// Helper function for contextual block header checks
bool ChainstateManager::ContextualCheckBlockHeader(const CBlockHeader& block, BlockValidationState& state, const Consensus::Params& params, const CBlockIndex* pindexPrev, int64_t nAdjustedTime) const
{
    // Check timestamp against prev
    if (block.GetBlockTime() <= pindexPrev->GetMedianTimePast())
        return state.Invalid(BlockValidationResult::BLOCK_INVALID_HEADER, "time-too-old", "block timestamp too old");
    
    // Check timestamp
    if (block.GetBlockTime() > nAdjustedTime + 2 * 60 * 60)
        return state.Invalid(BlockValidationResult::BLOCK_TIME_FUTURE, "time-too-new", "block timestamp too far in the future");
    
    // Check version format based on AuxPow activation status
    int nHeight = pindexPrev ? pindexPrev->nHeight + 1 : 0;
    
    // Check if AuxPow is active at this height
    bool auxpow_active = nHeight >= params.nAuxpowStartHeight;
    
    // After AuxPow activates, check that blocks have the proper chain ID in the version bits
    if (auxpow_active && block.IsAuxPow()) {
        if (block.GetChainID() != params.nAuxpowChainId) {
            return state.Invalid(BlockValidationResult::BLOCK_CONSENSUS, "bad-version-chainid",
                                 strprintf("block's chain ID %d doesn't match expected chain ID %d", 
                                           block.GetChainID(), params.nAuxpowChainId));
        }
    }
    
    return true;
}

// Helper function to get consensus parameters

// Helper function to get chain parameters
