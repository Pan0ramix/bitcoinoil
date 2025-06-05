#!/bin/bash

# Kill any existing processes
pkill -f bitcoinoild || true
sleep 2

# Clean start
rm -rf ./regtest_data_test
mkdir -p ./regtest_data_test

# Create minimal config
cat > ./regtest_data_test/bitcoinoil.conf << EOF
regtest=1
server=1

[regtest]
rpcuser=testuser
rpcpassword=testpass123
rpcallowip=127.0.0.1
rpcbind=127.0.0.1
rpcport=19543
port=19544
EOF

echo "Starting daemon..."
./src/bitcoinoild -regtest -datadir=./regtest_data_test -daemon

echo "Waiting for startup..."
sleep 15

echo "Testing connection..."
./src/bitcoinoil-cli -regtest -datadir=./regtest_data_test -rpcport=19543 -rpcuser=testuser -rpcpassword=testpass123 getblockchaininfo

echo "Testing AuxPow info..."
./src/bitcoinoil-cli -regtest -datadir=./regtest_data_test -rpcport=19543 -rpcuser=testuser -rpcpassword=testpass123 getauxpowinfo

echo "Done." 