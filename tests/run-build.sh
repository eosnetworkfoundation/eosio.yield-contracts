rm -f  ../build/eosioyield/eosioyield.wasm
rm -f  ../build/eosioyield/eosioyield.abi

if [[ $* == --debug ]]
then
    echo "Compiling in debug mode..."
    eosio-cpp -I ../include/ -O 3 ../src/eosioyield.cpp -abigen -abigen_output ../build/oracleyield/eosioyield.abi -o ../build/eosioyield/eosioyield.wasm -D=DEBUG
else
    cd ..
    cd build
    cmake ..
    make
    echo "Successfully built. Output in ./build/eosioyield"
fi
