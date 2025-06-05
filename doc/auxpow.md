# Auxiliary Proof-of-Work (AuxPow) Implementation

BitcoinOil supports auxiliary proof-of-work (AuxPow), also known as merged mining, which allows miners to mine multiple cryptocurrencies simultaneously without additional computational overhead.

## Overview

AuxPow enables a "parent" blockchain (like Bitcoin) to provide proof-of-work for a "child" blockchain (BitcoinOil) by embedding the child block's hash in the parent's coinbase transaction.

## Technical Details

### Chain ID
- BitcoinOil uses Chain ID: **16**
- Chain IDs are embedded in bits 16-23 of the block version field
- Version mask: `0x00FF0000`
- Version shift: `16`

### Activation
- AuxPow becomes active at block height: **5000**
- Before this height, only regular proof-of-work is accepted
- After this height, both regular PoW and AuxPow blocks are accepted

### Block Version Format

```
Bits 31-24: Reserved (must be 0)
Bits 23-16: Chain ID (16 for BitcoinOil)
Bits 15-9:  Reserved (must be 0)
Bit  8:     AuxPow flag (1 if this is an AuxPow block)
Bits 7-0:   Standard version bits
```

### AuxPow Structure

An AuxPow block contains:
1. **Coinbase Transaction**: The parent chain's coinbase transaction
2. **Chain Merkle Branch**: Proof linking the auxiliary block to the coinbase
3. **Chain Index**: Position in the chain merkle tree
4. **Merkle Branch**: Proof linking the coinbase to the parent block
5. **Parent Block Header**: The parent blockchain's block header

## Validation Process

1. **Version Check**: Verify the AuxPow flag is set and chain ID matches
2. **Height Check**: Ensure AuxPow is active (height >= 5000)
3. **Chain ID Verification**: Extract and validate the chain ID from block version
4. **AuxPow Validation**: Verify the cryptographic proofs in the AuxPow structure
5. **Parent PoW Check**: Verify the parent block meets its proof-of-work requirement

## RPC Commands

### `getauxpowinfo`
Returns general information about AuxPow configuration:
```json
{
  "chain_id": 16,
  "start_height": 5000,
  "auxpow_flag": "0x00000100",
  "chain_id_mask": "0x00ff0000",
  "chain_id_shift": 16,
  "auxpow_enabled": true
}
```

### `getblockauxpow "blockhash"`
Returns AuxPow-specific information for a block:
```json
{
  "is_auxpow": true,
  "chain_id": 16,
  "version": "0x00100100",
  "auxpow": {
    "has_data": true,
    "parent_block_hash": "00000000...",
    "chain_index": 0,
    "chain_merkle_branch": ["hash1", "hash2"],
    "merkle_branch": ["hash3", "hash4"],
    "coinbase_txid": "abcdef..."
  }
}
```

## Implementation Files

- `src/auxpow.h/cpp`: Core AuxPow class implementation
- `src/auxpow_constants.h`: Version bit constants and masks
- `src/auxpow_serialization.h`: Serialization templates
- `src/pow.cpp`: AuxPow validation functions
- `src/primitives/block.h`: Block header AuxPow integration
- `src/rpc/auxpow.cpp`: RPC command implementations
- `src/test/auxpow_tests.cpp`: Unit tests

## Security Considerations

1. **Chain ID Uniqueness**: Each merged-mined chain must have a unique chain ID
2. **Parent Chain Security**: Security depends on the parent blockchain's hash rate
3. **Validation Ordering**: AuxPow validation occurs before regular PoW validation
4. **Merkle Proof Integrity**: All merkle branches must be cryptographically valid

## Testing

Use the test suite in `src/test/auxpow_tests.cpp` to verify implementation:
```bash
./src/test/test_bitcoinoil --run_test=auxpow_tests
```

For regtest integration testing, use the scripts in the `regtest/` directory.

## Compatibility

This implementation is compatible with:
- Namecoin-style AuxPow (the de facto standard)
- Dogecoin AuxPow implementation
- Other Bitcoin-derived coins using similar AuxPow schemes

The chain ID (16) is chosen to avoid conflicts with other known merged-mined cryptocurrencies. 