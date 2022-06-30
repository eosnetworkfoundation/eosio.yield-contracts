// admin
export const metakeys = [
  {required: true,  type: "url", key: "website", description: "Protocol website"},
  {required: true,  type: "string", key: "name", description: "Protocol name"},
  {required: false, type: "string", key: "dappradar", description: "DappRadar identifier"},
]

export const categories = [
  {category: "dexes", description: "Protocols where you can swap/trade cryptocurrency"},
  {category: "lending", description: "Protocols that allow users to borrow and lend assets"},
]

// yield
export const category = categories[0].category;
export const category1 = categories[1].category;
export const eos_contracts = ["myprotocol", "myvault"];
export const evm_contracts = ["0x2f9ec37d6ccfff1cab21733bdadede11c823ccb0"];
export const metadata_yield = [
  {key: "name", value: "My Protocol"},
  {key: "website", value: "https://myprotocol.com"}
];


// oracle
export const metadata_oracle = [
  {key: "name", value: "My Oracle"},
  {key: "website", value: "https://myoracle.com"}
];