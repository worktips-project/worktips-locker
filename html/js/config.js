var config = {
    apiUrl: "http://127.0.0.1:1984/",
    mainnetExplorerUrl: "https://lokiblocks.com/",
    testnetExplorerUrl: "https://testnet.xmrchain.com/",
    stagenetExplorerUrl: "http://162.210.173.150:8083/",
    nettype: 0, /* 0 - MAINNET, 1 - TESTNET, 2 - STAGENET */
    coinUnitPlaces: 12,
    txMinConfirms: 10,         // corresponds to CRYPTONOTE_DEFAULT_TX_SPENDABLE_AGE in Monero
    txCoinbaseMinConfirms: 30, // corresponds to CRYPTONOTE_MINED_MONEY_UNLOCK_WINDOW in Monero
    coinSymbol: 'LOKI',
    openAliasPrefix: "loki",
    coinName: 'Loki',
    coinUriPrefix: 'loki:',
    addressPrefix: 114,
    integratedAddressPrefix: 115,
    subAddressPrefix: 116,
    addressPrefixTestnet: 156,
    integratedAddressPrefixTestnet: 157,
    subAddressPrefixTestnet: 158,
    addressPrefixStagenet: 24,
    integratedAddressPrefixStagenet: 25,
    subAddressPrefixStagenet: 36,
    feePerKB: new JSBigInt('2000000000'),//20^10 - not used anymore, as fee is dynamic.
    dustThreshold: new JSBigInt('1000000000'),//10^10 used for choosing outputs/change - we decompose all the way down if the receiver wants now regardless of threshold
    txChargeRatio: 0.5,
    defaultMixin: 10, // minimum mixin for hardfork v7 is 6 (ring size 7)
    txChargeAddress: '',
    idleTimeout: 30,
    idleWarningDuration: 20,
    maxBlockNumber: 500000000,
    avgBlockTime: 120,
    debugMode: true
};
