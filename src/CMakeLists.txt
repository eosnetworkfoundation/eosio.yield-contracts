project(oracleyield)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt)

add_contract( oracleyield oracleyield oracleyield.cpp )
target_include_directories( oracleyield PUBLIC ${CMAKE_SOURCE_DIR}/../include )
target_ricardian_directory( oracleyield ${CMAKE_SOURCE_DIR}/../ricardian )