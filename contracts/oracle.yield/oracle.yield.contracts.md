<h1 class="contract">addtoken</h1>

---
spec_version: "0.2.0"
title: Add Token
summary: 'Add {{sym}} token as supported asset.'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This action can only be called by the Yield+ oracle contract self permission. It will add the {{symcode}}@{{contract}} token as a supported asset using {{defibox_oracle_id}} Defibox Oracle ID & {{delphi_oracle_id}} Delphi Oracle ID.

<h1 class="contract">init</h1>

---
spec_version: "0.2.0"
title: Init
summary: 'Initializes the Yield+ oracle contract'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This action can only be called by the Yield+ oracle contract's self permission. It will initialize the Yield+ oracle contract for the {{yield_contract}} contract with the administrator {{admin_contract}} using the token {{rewards}}

<h1 class="contract">deltoken</h1>

---
spec_version: "0.2.0"
title: Delete Token
summary: 'deltoken'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This action can only be called by the Yield+ oracle contract's self permission. It will remove the {{symcode}} token as supported asset.


<h1 class="contract">setreward</h1>

---
spec_version: "0.2.0"
title: Set Reward
summary: 'Set oracle rewards'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This action can only be called by the Yield+ oracle contract's self permission. It will set oracle rewards at {{reward_per_update}} per update.




<h1 class="contract">regoracle</h1>

---
spec_version: "0.2.0"
title: Register Oracle
summary: 'Registers the {{oracle}} oracle with the Yield+ oracle contract'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This action can only be called by the specific {{oracle}} oracle's account. It will register {{oracle}} with the following metadata: {{metadata}}.



<h1 class="contract">unregister</h1>

---
spec_version: "0.2.0"
title: Unregister Oracle
summary: 'Unregisters the {{oracle}} oracle from the Yield+ oracle contract'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This action can only be called by the specific {{oracle}} oracle's account. It will unregister {{oracle}} from the Yield+ oracle contract.



<h1 class="contract">setmetadata</h1>

---
spec_version: "0.2.0"
title: Set Metadata
summary: 'Set metadata for the {{oracle}} oracle'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This action can be called by the specific {{oracle}} oracle's account or the Yield+ administrator account. It will set the {{oracle}} oracle with the following metadata: {{metadata}}.



<h1 class="contract">setmetakey</h1>

---
spec_version: "0.2.0"
title: Set Meta Key
summary: 'Set specific metadata key-value pairs'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This action can be called by the specific {{oracle}} oracle's account or the Yield+ administrator account. It will set the {{oracle}} oracle's {{key}} key with the {{value}} value.



<h1 class="contract">approve</h1>

---
spec_version: "0.2.0"
title: Approve
summary: 'Approve the {{oracle}} oracle for Yield+ rewards'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This action can only be called by the Yield+ administrator contract. It will approve the {{oracle}} oracle for for Yield+ oracle rewards.


<h1 class="contract">deny</h1>

---
spec_version: "0.2.0"
title: Deny
summary: 'Deny the {{oracle}} oracle for Yield+ rewards'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This action can only be called by the Yield+ administrator contract. It will deny the {{oracle}} oracle for for Yield+ oracle rewards.



<h1 class="contract">update</h1>

---
spec_version: "0.2.0"
title: Update
summary: 'Update TVL for a specific protocol'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This action can only be called by an approved oracle account. It will update the TVL for the {{protocol}} protocol using the {{oracle}} oracle.



<h1 class="contract">updateall</h1>

---
spec_version: "0.2.0"
title: Update All
summary: 'Update the TVL for all protocols'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This action can only be called by an approved oracle account. It will update the TVL for all protocols using the {{oracle}} oracle.


<h1 class="contract">updatelog</h1>

---
spec_version: "0.2.0"
title: Update Log
summary: 'Generates a log when an oracle updates its smart contracts'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This action can only be called by the Yield+ oracle contract's self permission. It will record that, for the time period ending at {{period}}, the {{oracle}} oracle updated the {{protocol}} {{category}} protocol, which is using the EOS contract(s) {{contracts}}
{{#if_has_value evm}} and the EVM contract(s) {{evm}}{{#/if_has_value}}. The updated TVL is {{tvl}} EOS and ${{usd}} USD.


<h1 class="contract">claim</h1>

---
spec_version: "0.2.0"
title: Claim Rewards
summary: 'Claims Yield+ rewards for an oracle'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This action can only be called by the {{oracle}} oracle contract. It will claim rewards for the {{oracle}} oracle.



<h1 class="contract">claimlog</h1>

---
spec_version: "0.2.0"
title: Claim Log
summary: 'Generates a log when Yield+ rewards are claimed.'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This action can only be called by the Yield+ oracle contract's self permission. It will record that {{oracle}} claimed {{claimed.quantity}}.

<h1 class="contract">statuslog</h1>

---
spec_version: "0.2.0"
title: Status Log
summary: 'Generates a log when oracle status is modified.'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This action can only be called by the Yield+ oracle contract self permission. It will record that {{oracle}} status has been updated to {{status}}.



<h1 class="contract">createlog</h1>

---
spec_version: "0.2.0"
title: Create Log
summary: 'Generates a log when an oracle is created in the Yield+ oracle contract.'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This action can only be called by the Yield+ oracle contract self permission. It will record that {{oracle}} oracle has been registered with the Yield+ contract with the following metadata: {{metadata}}



<h1 class="contract">eraselog</h1>

---
spec_version: "0.2.0"
title: Erase Log
summary: 'Generates a log when an oracle is erased from the Yield+ oracle contract.'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This action can only be called by the Yield+ oracle contract self permission. It will record that {{oracle}} status has been erased from the Yield+ contract.



<h1 class="contract">balancelog</h1>

---
spec_version: "0.2.0"
title: Balance Log
summary: 'Generates a log when an oracle balance is modified.'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This action can only be called by the Yield+ oracle contract self permission. It will record that the {{oracle}} Yield+ rewards balance has been updated to {{balance.quantity}}.




<h1 class="contract">metadatalog</h1>

---
spec_version: "0.2.0"
title: Metadata Log
summary: 'Generates a log when oracle metadata is modified.'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This action can only be called by the Yield+ oracle contract self permission. It will record that {{oracle}} metadata has been modified to: {{metadata}}.



<h1 class="contract">cleartable</h1>

---
spec_version: "0.2.0"
title: Clear Table
summary: 'Clears all tables. Delete all contract data. For testing only.'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This option is for debug purposes and will be removed in production. Clears all tables from {{table_name}}
{{if_has_value scope}} with the scope {{scope}} {{/if_has_value}}
{{if_has_value max_rows}} with a maxiumum of {{max_rows}} rows{{/if_has_value}}.
