<h1 class="contract">claim</h1>

---
spec_version: "0.2.0"
title: Claim
summary: 'Claims the Yield+ rewards for the {{protocol}} protocol.'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This action can only be called by the {{protocol}} protocol account. It will claim all rewards earned by the {{protocol}} protocol.

{{#if_has_value receiver}}The rewards will be sent to the {{receiver}} account. {{/if_has_value}}



<h1 class="contract">init</h1>

---
spec_version: "0.2.0"
title: init
summary: 'Initialize the rewards contract'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This can only be called by the Yield+ contract permission. It initializes the contract with the {{oracle_contract}} oracle and the {{admin_contract}} administrator. It will use the {{rewards.sym}} token at the contract {{rewards.contract}} for rewards.


<h1 class="contract">setrate</h1>

---
spec_version: "0.2.0"
title: Set rate
summary: 'Set TVL rewards rate at {{annual_rate}} basis points.'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This can only be called by the contract permission. It will set the reward rate at {{annual_rate}} basis points with a minimum TVL of {{min_tvl_report}} and a maximum TVL of {{max_tvl_report}}.


<h1 class="contract">regprotocol</h1>

---
spec_version: "0.2.0"
title: Register Protocol
summary: 'Register the {{protocol}} protocol.'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This action can only be called by the {{protocol}} account or the administrator account. It will register the {{protocol}} protocol with the following metadata.

<h1 class="contract">setmetakey</h1>

---
spec_version: "0.2.0"
title: Set Meta Key
summary: 'Set the {{key}} metadata key to {{value}}.'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This action can only be called by the {{protocol}} account or the administrator account. It will set a metadata key-value pair for the {{protocol}} protocol with a key of {{key}} and a value of {{value}}.

<h1 class="contract">unregister</h1>

---
spec_version: "0.2.0"
title: Unregister
summary: 'Unregister the {{protocol}} protocol.'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This action can only be called by the {{protocol}} account. It will unregister the {{protocol}} protocol from the Yield+ Rewards program.

<h1 class="contract">setcontracts</h1>

---
spec_version: "0.2.0"
title: Set Contracts
summary: 'Sets the smart contracts for the {{protocol}} protocol.'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This action requires the permissions of either the administrator contract or both the {{protocol}} account and the {{contracts}} accounts. It will set the EOS contracts for the {{protocol}} protocol as {{contracts}}.


<h1 class="contract">setevm</h1>

---
spec_version: "0.2.0"
title: Set EVM
summary: 'Sets EVM contracts for the {{protocol}} protocol.'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This action requires the permissions of either the administrator contract or both the {{protocol}} account and the {{evm}} EVM contract accounts. It will set the following additional EVM contracts for the {{protocol}} protocol: {{evm}}.

<h1 class="contract">approve</h1>

---
spec_version: "0.2.0"
title: Approve
summary: 'Approves the {{protocol}} protocol for the Yield+ rewards program.'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This action can only be called by the administrator account. It will approve the {{protocol}} protocol for the Yield+ rewards program.

<h1 class="contract">setcategory</h1>

---
spec_version: "0.2.0"
title: Set Category
summary: 'Sets the category of the {{protocol}} protocol.'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This action can only be called by the administrator account. It will set the category of the {{protocol}} protocol to {{category}}.


<h1 class="contract">deny</h1>

---
spec_version: "0.2.0"
title: Deny
summary: 'Denies the {{protocol}} protocol for the Yield+ rewards program.'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This action can only be called by the administrator account. It will deny the {{protocol}} protocol for the Yield+ rewards program.


<h1 class="contract">claimlog</h1>

---
spec_version: "0.2.0"
title: Claim Log
summary: 'Generates a log each time Yield+ rewards are claimed.'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This action can only be called by the Yield+ contract self account. It will generate a log that the rewards for the {{protocol}} {{category}} protocol were claimed in the amount of {{claimed}}
{{#if_has_value receiver}} by the account {{receiver}}
{{/if_has_value}}.


<h1 class="contract">report</h1>

---
spec_version: "0.2.0"
title: Report
summary: 'Generates a report of the current TVL from the {{protocol}} protocol.'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This action can only be called by the oracle contract account. It will report the TVL of the {{protocol}} protocol at the time of {{period}}. Average TVL will be reported as {{tvl}} EOS and ${{usd}} USD.


<h1 class="contract">rewardslog</h1>

---
spec_version: "0.2.0"
title: Rewards Log
summary: 'Generates a log when rewards are generated from reports.'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This action can only be called by the Yield+ contract self account. It generates a log when rewards are allocated. It will record a reward of {{rewards}} based on an EOS TVL of {{tvl}} and a USD TVL of {{usd}} for the {{period_interval}}-second period ending at {{period}} for the {{protocol}} {{category}} protocol. The protocol's claimable balance is now {{balance}}.

<h1 class="contract">statuslog</h1>

---
spec_version: "0.2.0"
title: Status Log
summary: 'Generates a log when a protocol's status is modified.'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This action can only be called by the Yield+ contract self account. It generates a log when a protocol's status has been modified between pending, approved, or denied. It will report the {{protocol}} protocol status as {{status}}.


<h1 class="contract">contractslog</h1>

---
spec_version: "0.2.0"
title: Contracts Log
summary: 'Generates a log when a protocol's contracts are modified.'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This action can only be called by the Yield+ contract self account. It generates a log when a protocol's contracts have been modified. It will report the {{protocol}} protocol contracts as using the EOS contract(s) {{contracts.eos}} and the EVM contract(s) {{contracts.evm}}.

<h1 class="contract">categorylog</h1>

---
spec_version: "0.2.0"
title: Category Log
summary: 'Generates a log when a protocol's category is modified.'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This action can only be called by the Yield+ contract self account. It generates a log when a protocol's category is modified. It will report the {{protocol}} protocol as affiliated with the {{category}} category.

<h1 class="contract">createlog</h1>

---
spec_version: "0.2.0"
title: Create Log
summary: 'Generates a log when a protocol is created.'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This action can only be called by the Yield+ contract self account. It generates a log when a protocol is created. It will report the {{protocol}} protocol as created with the following metadata: {{metadata}}.


<h1 class="contract">eraselog</h1>

---
spec_version: "0.2.0"
title: Erase Log
summary: 'Generates a log when a protocol is erased.'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This action can only be called by the Yield+ contract self account. It generates a log when a protocol is erased. It will report that the {{protocol}} protocol has been erased.

<h1 class="contract">balancelog</h1>

---
spec_version: "0.2.0"
title: Balance Log
summary: 'Generates a log when a protocol's balance is updated.'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This action can only be called by the Yield+ contract self account. It generates a log when a protocol's balance changes. It will record the available balance of the {{protocol}} protocol as {{balance}}.

<h1 class="contract">metadatalog</h1>

---
spec_version: "0.2.0"
title: Metadata Log
summary: 'Generates a log when a protocol's metadata is modified.'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This action can only be called by the Yield+ contract self account. It generates a log when a protocol's metadata has been modified. It will record that the {{protocol}} protocol has the following metadata.

<h1 class="contract">setmetadata</h1>

---
spec_version: "0.2.0"
title: Set Metadata
summary: 'Set the metadata for the {{oracle}} oracle.'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This action can only be called by the {{oracle}} account or the Yield+ administrator account. It will set the following metadata for the {{protocol}} protocol.

<h1 class="contract">cleartable</h1>

---
spec_version: "0.2.0"
title: Clear Table (debug)
summary: 'Clears the smart contract table. Deletes all data.'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This function is used for debug purposes and will be removed in the final release.


<h1 class="contract">addbalance</h1>

---
spec_version: "0.2.0"
title: Add Balance (debug)
summary: 'Adds to a protocol's Yield+ rewards balance'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This function is used for debug purposes and will be removed in the final release.
