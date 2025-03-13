// Copyright (c) 2022-2023 The Bitcoinoil developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

// This file contains stub implementations for CVerifyDB and ChainstateManager methods

#include <validation.h>
#include <logging.h>
#include <chain.h>
#include <kernel/chainstatemanager_opts.h>
#include <node/blockstorage.h>

// Stub implementations for CVerifyDB methods
CVerifyDB::CVerifyDB() { 
    LogPrintf("CVerifyDB constructor stub called\n"); 
}

CVerifyDB::~CVerifyDB() { 
    LogPrintf("CVerifyDB destructor stub called\n"); 
}

VerifyDBResult CVerifyDB::VerifyDB(Chainstate& chainstate, const Consensus::Params& consensus_params, CCoinsView& coinsview, int nCheckLevel, int nCheckDepth) {
    LogPrintf("CVerifyDB::VerifyDB stub called\n");
    return VerifyDBResult::SUCCESS;
}

// Stub implementations for ChainstateManager methods
#ifndef BITCOINOIL_CHAINSTATE_INIT_CPP
ChainstateManager::ChainstateManager(kernel::ChainstateManagerOpts chainstate_opts, node::BlockManager::Options blockman_opts)
    : m_options(chainstate_opts), 
      m_blockman(blockman_opts)
{
    LogPrintf("ChainstateManager constructor stub called\n");
    // We don't create a chainstate here, it will be created when needed
}
#endif

ChainstateManager::~ChainstateManager() {
    LogPrintf("ChainstateManager destructor stub called\n");
}

bool ChainstateManager::LoadBlockIndex() { 
    LogPrintf("ChainstateManager::LoadBlockIndex stub called\n");
    return true; 
}

// ProcessNewBlock implementation has been moved to chainstate_init.cpp

MempoolAcceptResult ChainstateManager::ProcessTransaction(const CTransactionRef& tx, bool test_accept) { 
    LogPrintf("ChainstateManager::ProcessTransaction stub called\n");
    return MempoolAcceptResult::Failure(TxValidationState()); 
}

Chainstate& ChainstateManager::InitializeChainstate(CTxMemPool* mempool) { 
    LogPrintf("ChainstateManager::InitializeChainstate stub called\n");
    if (!m_ibd_chainstate) {
        // Create a chainstate on first use
        m_ibd_chainstate = std::make_unique<Chainstate>(nullptr, m_blockman, *this);
    }
    return *m_ibd_chainstate; 
}

void ChainstateManager::MaybeRebalanceCaches() {
    LogPrintf("ChainstateManager::MaybeRebalanceCaches stub called\n");
}

void ChainstateManager::ReportHeadersPresync(const arith_uint256& work, int64_t height, int64_t timestamp) {
    LogPrintf("ChainstateManager::ReportHeadersPresync stub called\n");
}

bool ChainstateManager::ProcessNewBlockHeaders(const std::vector<CBlockHeader>& headers, bool min_pow_checked, BlockValidationState& state, const CBlockIndex** ppindex) { 
    LogPrintf("ChainstateManager::ProcessNewBlockHeaders stub called\n");
    if (ppindex) *ppindex = nullptr;
    return true; 
}

bool ChainstateManager::DetectSnapshotChainstate(CTxMemPool* mempool) { 
    LogPrintf("ChainstateManager::DetectSnapshotChainstate stub called\n");
    return false; 
}

bool ChainstateManager::ValidatedSnapshotCleanup() { 
    LogPrintf("ChainstateManager::ValidatedSnapshotCleanup stub called\n");
    return true; 
}

SnapshotCompletionResult ChainstateManager::MaybeCompleteSnapshotValidation(std::function<void (bilingual_str)> completion_callback) { 
    LogPrintf("ChainstateManager::MaybeCompleteSnapshotValidation stub called\n");
    return SnapshotCompletionResult::SUCCESS; 
}

std::vector<Chainstate*> ChainstateManager::GetAll() { 
    LogPrintf("ChainstateManager::GetAll stub called\n");
    std::vector<Chainstate*> result;
    if (m_ibd_chainstate) result.push_back(m_ibd_chainstate.get());
    if (m_snapshot_chainstate) result.push_back(m_snapshot_chainstate.get());
    return result; 
}

Chainstate& ChainstateManager::ActiveChainstate() const { 
    LogPrintf("ChainstateManager::ActiveChainstate stub called\n");
    if (!m_ibd_chainstate) {
        // This is a const method, so we can't create the chainstate here
        // This is a hack to make it work in a stub implementation
        const_cast<ChainstateManager*>(this)->m_ibd_chainstate = 
            std::make_unique<Chainstate>(nullptr, const_cast<node::BlockManager&>(m_blockman), const_cast<ChainstateManager&>(*this));
    }
    return *m_ibd_chainstate; 
}

bool ChainstateManager::IsSnapshotActive() const { 
    LogPrintf("ChainstateManager::IsSnapshotActive stub called\n");
    return false; 
}
