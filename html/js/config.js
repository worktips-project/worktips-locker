var config = {
    apiUrl: "http://127.0.0.1:1984/",
    //apiUrl: "http://<SITE>:1984/",
    mainnetExplorerUrl: "https://wtip-explorer.gonspool.com/",
    testnetExplorerUrl: "",
    stagenetExplorerUrl: "",
    nettype: 1, /* 0 - MAINNET, 1 - TESTNET, 2 - STAGENET */
    coinUnitPlaces: 8,
    txMinConfirms: 10,         // corresponds to CRYPTONOTE_DEFAULT_TX_SPENDABLE_AGE in Worktips
    txCoinbaseMinConfirms: 30, // corresponds to CRYPTONOTE_MINED_MONEY_UNLOCK_WINDOW in Worktips
    coinSymbol: 'WTIP',
    openAliasPrefix: "wtip",
    coinName: 'Worktips',
    coinUriPrefix: 'wtips:',
    addressPrefix: 0x121bb2,
    integratedAddressPrefix: 0x121bb2,
    subAddressPrefix: 0x1a9bb2,
    addressPrefixTestnet: 0x1ddd32,
    integratedAddressPrefixTestnet: 0x221d32,
    subAddressPrefixTestnet: 0x269d32,
    addressPrefixStagenet: 0xdd32,
    integratedAddressPrefixStagenet: 0x4dd32,
    subAddressPrefixStagenet: 0x95d32,
    feePerKB: new JSBigInt('200'),//20^10 - not used anymore, as fee is dynamic.
    dustThreshold: new JSBigInt('1000000'),//10^10 used for choosing outputs/change - we decompose all the way down if the receiver wants now regardless of threshold
    txChargeRatio: 0.5,
    defaultMixin: 9,
    txChargeAddress: '',
    idleTimeout: 30,
    idleWarningDuration: 20,
    maxBlockNumber: 500000000,
    avgBlockTime: 120,
    debugMode: false
};
