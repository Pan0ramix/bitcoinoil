// Copyright (c) 2023 The Bitcoinoil developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

// This file contains additional stub implementations needed for tests and fuzzing

#include <validation.h>
#include <node/utxo_snapshot.h>
#include <logging.h>
#include <chain.h>
#include <kernel/chainstatemanager_opts.h>
#include <primitives/block.h>

// Private member variable for storing the snapshot blockhash
namespace {
    std::optional<uint256> g_snapshot_blockhash;
}

// Stub implementation for ChainstateManager::ActivateSnapshot
__attribute__((weak)) bool ChainstateManager::ActivateSnapshot(AutoFile& coins_file, const node::SnapshotMetadata& metadata, bool in_memory) {
    LogPrintf("ChainstateManager::ActivateSnapshot stub called\n");
    
    // For testing purposes, pretend to create a snapshot chainstate
    if (!m_snapshot_chainstate) {
        m_snapshot_chainstate = std::make_unique<Chainstate>(nullptr, m_blockman, *this);
    }
    
    // Store the blockhash for SnapshotBlockhash()
    g_snapshot_blockhash = metadata.m_base_blockhash;
    
    return true;
}

// Stub implementation for ChainstateManager::SnapshotBlockhash
__attribute__((weak)) std::optional<uint256> ChainstateManager::SnapshotBlockhash() const {
    LogPrintf("ChainstateManager::SnapshotBlockhash stub called\n");
    
    // Return the stored snapshot blockhash (set during ActivateSnapshot)
    return g_snapshot_blockhash;
}

// Stub implementation for Chainstate::ResizeCoinsCaches
__attribute__((weak)) bool Chainstate::ResizeCoinsCaches(size_t coinstip_size, size_t coinsdb_size) {
    LogPrintf("Chainstate::ResizeCoinsCaches stub called with coinstip_size=%zu, coinsdb_size=%zu\n", 
             coinstip_size, coinsdb_size);
    
    // Just log the call, no actual implementation needed for tests
    return true;
}

// Stub implementation for ChainstateManager::ResetChainstates
__attribute__((weak)) void ChainstateManager::ResetChainstates() {
    LogPrintf("ChainstateManager::ResetChainstates stub called\n");
    
    // Reset the chainstates for testing purposes
    m_ibd_chainstate.reset();
    m_snapshot_chainstate.reset();
    g_snapshot_blockhash.reset();
}

// Stub implementation for ChainstateManager::ActivateExistingSnapshot
__attribute__((weak)) Chainstate& ChainstateManager::ActivateExistingSnapshot(CTxMemPool* mempool, uint256 base_blockhash) {
    LogPrintf("ChainstateManager::ActivateExistingSnapshot stub called\n");
    
    // For testing purposes, pretend to activate an existing snapshot
    if (!m_snapshot_chainstate) {
        m_snapshot_chainstate = std::make_unique<Chainstate>(mempool, m_blockman, *this);
    }
    
    // Store the blockhash for SnapshotBlockhash()
    g_snapshot_blockhash = base_blockhash;
    
    // Return the chainstate
    return *m_snapshot_chainstate;
} 