rm -f  ../build/oracleyield/oracleyield.wasm
rm -f  ../build/oracleyield/oracleyield.abi

if [[ $* == --debug ]]
then
    echo "Compiling in debug mode..."
    eosio-cpp -I ../include/ -O 3 ../src/oracleyield.cpp -abigen -abigen_output ../build/oracleyield/oracleyield.abi -o ../build/oracleyield/oracleyield.wasm -D=DEBUG
else
    cd ..
    cd build
    cmake ..
    make
    echo "Successfully built. Output in ./build/oracleyield"
fi
