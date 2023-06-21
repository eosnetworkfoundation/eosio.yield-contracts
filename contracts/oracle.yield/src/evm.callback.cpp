int64_t oracle::bytes_to_int64( const bytes data, const uint8_t decimals )
{
    eosio::check(data.size() == 32, "bytes_to_int64: wrong length");
    auto v = intx::be::unsafe::load<intx::uint256>(data.data()) / pow(10, decimals); // reduce precision
    eosio::check(v <= numeric_limits<int64_t>::max(), "bytes_to_int64: out of range");
    return int64_t(v);
}

[[eosio::action]]
void oracle::callback( const int32_t status, const bytes data, const std::optional<bytes> context )
{
    check(get_first_receiver() == get_self(), "callback must initially be called by this contract");
    const int64_t amount = bytes_to_int64(data, 14); // TO-DO: change decimals to dynamic value
    const string context_str = silkworm::to_hex(*context, false);
    const string contract = context_str.substr(0, 40);
    const string address = context_str.substr(40, 40);

    // Push transaction
    test_action test{ get_self(), { get_self(), "active"_n } };
    test.send( contract, address, amount );
}

[[eosio::action]]
void oracle::balanceof( const string contract, const string address )
{
    // Convert to Bytes
    bytes contract_bytes = *silkworm::from_hex(contract);
    bytes address_bytes = *silkworm::from_hex(address);
    bytes address_bytes_left_padding = *silkworm::from_hex("000000000000000000000000" + address);

    // balanceOf address
    bytes data;
    bytes balance_of = *silkworm::from_hex("70a08231"); // sha3(balanceOf(address))[:4]
    data.insert(data.end(), balance_of.begin(), balance_of.end());
    data.insert(data.end(), address_bytes_left_padding.begin(), address_bytes_left_padding.end());

    // Inputs
    evm_contract::exec_input input;
    input.data = data;
    input.to = contract_bytes;

    // Context
    bytes context; // contract + address
    context.insert(context.end(), contract_bytes.begin(), contract_bytes.end());
    context.insert(context.end(), address_bytes.begin(), address_bytes.end());
    input.context = context;

    // Callback
    evm_contract::exec_callback callback;
    callback.contract = get_self();
    callback.action = "callback"_n;

    // Push transaction
    evm_contract::exec_action exec{ "eosio.evm"_n, { get_self(), "active"_n } };
    exec.send( input, callback );
}

[[eosio::action]]
void oracle::test( const string contract, const string address, const int64_t amount )
{
    require_auth( get_self() );
}