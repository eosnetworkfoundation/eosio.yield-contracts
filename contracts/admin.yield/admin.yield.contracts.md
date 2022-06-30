<h1 class="contract">setmetakey</h1>

---
spec_version: "0.2.0"
title: Set metakey
summary: 'Set {{key}} metakey.'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This action can only be called by the Yield+ administrator self contract. It will set the metakey {{key}} with the type {{type}} as ({{description}}) with required={{required}}.


<h1 class="contract">setcategory</h1>

---
spec_version: "0.2.0"
title: Set category
summary: 'Set {{category}} category.'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This action can only be called by the Yield+ administrator self contract. It will set the {{category}} category with ({{description}}).


<h1 class="contract">delmetakey</h1>

---
spec_version: "0.2.0"
title: Delete metakey
summary: 'Delete {{key}} metakey.'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This action can only be called by the Yield+ administrator self contract. It will delete the {{key}} metakey and its data.


<h1 class="contract">delcategory</h1>

---
spec_version: "0.2.0"
title: Delete category
summary: 'Delete {{category}} category.'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This action can only be called by the Yield+ administrator self contract. It will delete {{category}} category and its data.


<h1 class="contract">cleartable</h1>

---
spec_version: "0.2.0"
title: Clear Table (debug)
summary: 'Clears all tables. Delete all contract data. For testing only.'
icon: https://gateway.pinata.cloud/ipfs/QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk#88016c23a1ed3af668f50353523ba29d086a8d3a460340b6e53add24588e5c5c
---

This option is for debug purposes and will be removed in production. Clears all tables from {{table_name}}
{{if_has_value scope}} with the scope {{scope}} {{/if_has_value}}
{{if_has_value max_rows}} with a maxiumum of {{max_rows}} rows{{/if_has_value}}.
