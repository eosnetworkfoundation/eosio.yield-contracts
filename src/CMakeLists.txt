project(eosioyield)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt)

add_contract( eosioyield eosioyield eosioyield.cpp )
target_include_directories( eosioyield PUBLIC ${CMAKE_SOURCE_DIR}/../include )
target_ricardian_directory( eosioyield ${CMAKE_SOURCE_DIR}/../ricardian )