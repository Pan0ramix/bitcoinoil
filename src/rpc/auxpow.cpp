// Copyright (c) 2023 The Bitcoinoil developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <rpc/server.h>
#include <rpc/util.h>
#include <rpc/blockchain.h>
#include <rpc/server_util.h>

#include <auxpow.h>
#include <auxpow_constants.h>
#include <primitives/block.h>
#include <validation.h>
#include <node/context.h>
#include <node/blockstorage.h>
#include <chainparams.h>
#include <consensus/validation.h>
#include <univalue.h>

using node::BlockManager;
using node::NodeContext;

static RPCHelpMan getauxpowinfo()
{
    return RPCHelpMan{"getauxpowinfo",
        "\nReturns information about auxiliary proof-of-work configuration.\n",
        {},
        RPCResult{
            RPCResult::Type::OBJ, "", "",
            {
                {RPCResult::Type::NUM, "chain_id", "The chain ID for AuxPow"},
                {RPCResult::Type::NUM, "start_height", "The block height at which AuxPow becomes active"},
                {RPCResult::Type::STR_HEX, "auxpow_flag", "The version bit flag for AuxPow blocks"},
                {RPCResult::Type::STR_HEX, "chain_id_mask", "The bit mask for extracting chain ID"},
                {RPCResult::Type::NUM, "chain_id_shift", "The bit shift for chain ID"},
                {RPCResult::Type::BOOL, "auxpow_enabled", "Whether AuxPow is currently enabled (past activation height)"},
            }
        },
        RPCExamples{
            HelpExampleCli("getauxpowinfo", "")
            + HelpExampleRpc("getauxpowinfo", "")
        },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
{
    const ChainstateManager& chainman = EnsureAnyChainman(request.context);
    const Consensus::Params& params = chainman.GetParams().GetConsensus();
    
    LOCK(cs_main);
    const CBlockIndex* tip = chainman.ActiveChain().Tip();
    bool auxpow_enabled = tip && (tip->nHeight >= params.nAuxpowStartHeight);
    
    UniValue result(UniValue::VOBJ);
    result.pushKV("chain_id", params.nAuxpowChainId);
    result.pushKV("start_height", params.nAuxpowStartHeight);
    result.pushKV("auxpow_flag", strprintf("0x%08x", BLOCK_VERSION_AUXPOW));
    result.pushKV("chain_id_mask", strprintf("0x%08x", BLOCK_VERSION_CHAIN_ID_MASK));
    result.pushKV("chain_id_shift", BLOCK_VERSION_CHAIN_ID_SHIFT);
    result.pushKV("auxpow_enabled", auxpow_enabled);
    
    return result;
},
    };
}

static RPCHelpMan getblockauxpow()
{
    return RPCHelpMan{"getblockauxpow",
        "\nReturns auxiliary proof-of-work information for a specific block.\n",
        {
            {"blockhash", RPCArg::Type::STR_HEX, RPCArg::Optional::NO, "The block hash"},
        },
        RPCResult{
            RPCResult::Type::OBJ, "", "",
            {
                {RPCResult::Type::BOOL, "is_auxpow", "Whether this block uses AuxPow"},
                {RPCResult::Type::NUM, "chain_id", "The chain ID extracted from block version"},
                {RPCResult::Type::STR_HEX, "version", "The block version"},
                {RPCResult::Type::OBJ, "auxpow", "AuxPow data (if present)",
                {
                    {RPCResult::Type::BOOL, "has_data", "Whether auxpow data is present"},
                    {RPCResult::Type::STR_HEX, "parent_block_hash", "Hash of the parent block (if auxpow data present)"},
                    {RPCResult::Type::NUM, "chain_index", "Chain index in merkle tree (if auxpow data present)"},
                    {RPCResult::Type::ARR, "chain_merkle_branch", "Chain merkle branch (if auxpow data present)",
                    {
                        {RPCResult::Type::STR_HEX, "", "merkle branch hash"}
                    }},
                    {RPCResult::Type::ARR, "merkle_branch", "Merkle branch (if auxpow data present)",
                    {
                        {RPCResult::Type::STR_HEX, "", "merkle branch hash"}
                    }},
                }},
            }
        },
        RPCExamples{
            HelpExampleCli("getblockauxpow", "\"00000000c937983704a73af28acdec37b049d214adbda81d7e2a3dd146f6ed09\"")
            + HelpExampleRpc("getblockauxpow", "\"00000000c937983704a73af28acdec37b049d214adbda81d7e2a3dd146f6ed09\"")
        },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
{
    const ChainstateManager& chainman = EnsureAnyChainman(request.context);

    uint256 hash(ParseHashV(request.params[0], "blockhash"));

    const CBlockIndex* pblockindex;
    {
        LOCK(cs_main);
        pblockindex = chainman.m_blockman.LookupBlockIndex(hash);
        if (!pblockindex) {
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Block not found");
        }
    }

    CBlock block;
    if (!node::ReadBlockFromDisk(block, pblockindex, chainman.GetConsensus())) {
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Can't read block from disk");
    }

    UniValue result(UniValue::VOBJ);
    result.pushKV("is_auxpow", block.IsAuxPow());
    result.pushKV("chain_id", block.GetChainID());
    result.pushKV("version", strprintf("0x%08x", block.nVersion));

    UniValue auxpow_obj(UniValue::VOBJ);
    if (block.IsAuxPow() && block.auxpow) {
        auxpow_obj.pushKV("has_data", true);
        
        if (block.auxpow->GetParentBlockHeader()) {
            auxpow_obj.pushKV("parent_block_hash", block.auxpow->GetParentBlockHeader()->GetHash().GetHex());
        }
        
        auxpow_obj.pushKV("chain_index", block.auxpow->GetChainIndex());
        
        UniValue chain_branch(UniValue::VARR);
        for (const uint256& hash : block.auxpow->GetChainMerkleBranch()) {
            chain_branch.push_back(hash.GetHex());
        }
        auxpow_obj.pushKV("chain_merkle_branch", chain_branch);
        
        UniValue merkle_branch(UniValue::VARR);
        for (const uint256& hash : block.auxpow->GetMerkleBranch()) {
            merkle_branch.push_back(hash.GetHex());
        }
        auxpow_obj.pushKV("merkle_branch", merkle_branch);
        
        if (block.auxpow->GetCoinbaseTx()) {
            auxpow_obj.pushKV("coinbase_txid", block.auxpow->GetCoinbaseTx()->GetHash().GetHex());
        }
    } else {
        auxpow_obj.pushKV("has_data", false);
    }
    
    result.pushKV("auxpow", auxpow_obj);
    return result;
},
    };
}

void RegisterAuxPowRPCCommands(CRPCTable& t)
{
    static const CRPCCommand commands[]{
        {"auxpow", &getauxpowinfo},
        {"auxpow", &getblockauxpow},
    };
    for (const auto& c : commands) {
        t.appendCommand(c.name, &c);
    }
} 