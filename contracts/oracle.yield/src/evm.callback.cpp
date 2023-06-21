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
    oracle::evm_tokens_table _evm_tokens( get_self(), get_self().value );

    check(get_first_receiver() == get_self(), "callback must initially be called by this contract");
    const string context_str = silkworm::to_hex(*context, false);
    const bytes contract = *silkworm::from_hex(context_str.substr(0, 40));
    const bytes address = *silkworm::from_hex(context_str.substr(40, 40));

    // token amount from `balanceof` call
    const uint64_t token_id = get_account_id( contract );
    const uint64_t address_id = get_account_id( address );
    const auto token = _evm_tokens.find( token_id);
    check(token != _evm_tokens.end(), "oracle::callback: [token_id=" + to_string(token_id) + " & contract=" + silkworm::to_hex(contract, true) + "] token not found" );

    // token amount from `balanceof` call
    const uint8_t decimals = token->decimals - token->sym.precision();
    const int64_t amount = bytes_to_int64(data, decimals);

    // send inline action to update current balance
    test_action test{ get_self(), { get_self(), "active"_n } };
    test.send( contract, address, asset{amount, token->sym} );
}

[[eosio::action]]
void oracle::balanceof( const bytes contract, const bytes address )
{
    bytes padding = *silkworm::from_hex("000000000000000000000000");

    // balanceOf address
    bytes data;
    bytes method = *silkworm::from_hex("70a08231"); // sha3(balanceOf(address))[:4]
    data.insert(data.end(), method.begin(), method.end());
    data.insert(data.end(), padding.begin(), padding.end());
    data.insert(data.end(), address.begin(), address.end());

    // Inputs
    evm_contract::exec_input input;
    input.data = data;
    input.to = contract;

    // Context
    bytes context; // contract + address
    context.insert(context.end(), contract.begin(), contract.end());
    context.insert(context.end(), address.begin(), address.end());
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
void oracle::test( const bytes contract, const bytes address, const asset quantity )
{
    require_auth( get_self() );
}
