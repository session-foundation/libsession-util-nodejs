/// <reference path="../shared.d.ts" />
/// <reference path="./usergroups.d.ts" />

declare module 'libsession_util_nodejs' {
  export type ConvoVolatileType = '1o1' | UserGroupsType;

  export type BaseConvoInfoVolatile = {
    lastReadTsMs: number; // defaults to 0, unixTimestamp in ms
    forcedUnread: boolean; // defaults to false
  };

  type ConvoVolatile1o1GetExtra = {
    pubkeyHex: string;
  } & { proExpiryTsMs: number | null; genIndexHashB64: string | null };

  type ConvoVolatile1o1SetExtra = {
    /**
     * The timestamp of the proof expiry in milliseconds.
     * If null, no changes will be made.
     * To force the field to be reset, you need to provide 0 here
     */
    proExpiryTsMs: number | null;
    /**
     * The base64 encoded `genIndexHash` of the proof (32 bytes)
     * If null, no changes will be made.
     * To force the field to be reset, you need to provide an empty string here
     */
    proGenIndexHashB64: string | null;
  };

  type ConvoInfoVolatileGet1o1 = BaseConvoInfoVolatile & ConvoVolatile1o1GetExtra;
  type ConvoInfoVolatileGetLegacyGroup = BaseConvoInfoVolatile & { pubkeyHex: string };
  type ConvoInfoVolatileGetGroup = BaseConvoInfoVolatile & { pubkeyHex: GroupPubkeyType };
  type ConvoInfoVolatileGetCommunity = BaseConvoInfoVolatile & CommunityDetails;

  type ConvoInfoVolatileWrapper = BaseConfigWrapper & {
    init: (secretKey: Uint8Array, dump: Uint8Array | null) => void;
    /** This function is used to free wrappers from memory only */
    free: () => void;

    // 1o1 related methods
    get1o1: (pubkeyHex: string) => ConvoInfoVolatileGet1o1 | null;
    getAll1o1: () => Array<ConvoInfoVolatileGet1o1>;
    set1o1: (pubkeyHex: string, args: BaseConvoInfoVolatile & ConvoVolatile1o1SetExtra) => void;
    erase1o1: (pubkeyHex: string) => void;

    // legacy group related methods
    getLegacyGroup: (pubkeyHex: string) => ConvoInfoVolatileGetLegacyGroup | null;
    getAllLegacyGroups: () => Array<ConvoInfoVolatileGetLegacyGroup>;
    setLegacyGroup: (pubkeyHex: string, args: BaseConvoInfoVolatile) => void;
    eraseLegacyGroup: (pubkeyHex: string) => boolean;

    // group related methods
    getGroup: (pubkeyHex: GroupPubkeyType) => ConvoInfoVolatileGetGroup | null;
    getAllGroups: () => Array<ConvoInfoVolatileGetGroup>;
    setGroup: (pubkeyHex: GroupPubkeyType, args: BaseConvoInfoVolatile) => void;
    eraseGroup: (pubkeyHex: GroupPubkeyType) => boolean;

    // communities related methods
    getCommunity: (communityFullUrl: string) => ConvoInfoVolatileGetCommunity | null; // pubkey not required
    getAllCommunities: () => Array<ConvoInfoVolatileGetCommunity>;
    setCommunityByFullUrl: (fullUrlWithPubkey: string, args: BaseConvoInfoVolatile) => void;
    eraseCommunityByFullUrl: (fullUrlWithOrWithoutPubkey: string) => void;
  };

  export type ConvoInfoVolatileWrapperActionsCalls =
    MakeWrapperActionCalls<ConvoInfoVolatileWrapper>;

  export class ConvoInfoVolatileWrapperNode extends BaseConfigWrapperNode {
    constructor(secretKey: Uint8Array, dump: Uint8Array | null);
    // 1o1 related methods
    public get1o1: ConvoInfoVolatileWrapper['get1o1'];
    public getAll1o1: ConvoInfoVolatileWrapper['getAll1o1'];
    public set1o1: ConvoInfoVolatileWrapper['set1o1'];
    public erase1o1: ConvoInfoVolatileWrapper['eraseLegacyGroup'];

    // legacy-groups related methods
    public getLegacyGroup: ConvoInfoVolatileWrapper['getLegacyGroup'];
    public getAllLegacyGroups: ConvoInfoVolatileWrapper['getAllLegacyGroups'];
    public setLegacyGroup: ConvoInfoVolatileWrapper['setLegacyGroup'];
    public eraseLegacyGroup: ConvoInfoVolatileWrapper['eraseLegacyGroup'];

    // communities related methods
    public getCommunity: ConvoInfoVolatileWrapper['getCommunity'];
    public setCommunityByFullUrl: ConvoInfoVolatileWrapper['setCommunityByFullUrl'];
    public getAllCommunities: ConvoInfoVolatileWrapper['getAllCommunities'];
    public eraseCommunityByFullUrl: ConvoInfoVolatileWrapper['eraseCommunityByFullUrl'];
  }

  export type ConvoInfoVolatileConfigActionsType =
    | ['init', Uint8Array, Uint8Array | null]
    | MakeActionCall<ConvoInfoVolatileWrapper, 'free'>
    | MakeActionCall<ConvoInfoVolatileWrapper, 'get1o1'>
    | MakeActionCall<ConvoInfoVolatileWrapper, 'getAll1o1'>
    | MakeActionCall<ConvoInfoVolatileWrapper, 'set1o1'>
    | MakeActionCall<ConvoInfoVolatileWrapper, 'erase1o1'>
    | MakeActionCall<ConvoInfoVolatileWrapper, 'getLegacyGroup'>
    | MakeActionCall<ConvoInfoVolatileWrapper, 'getAllLegacyGroups'>
    | MakeActionCall<ConvoInfoVolatileWrapper, 'setLegacyGroup'>
    | MakeActionCall<ConvoInfoVolatileWrapper, 'eraseLegacyGroup'>
    | MakeActionCall<ConvoInfoVolatileWrapper, 'getGroup'>
    | MakeActionCall<ConvoInfoVolatileWrapper, 'getAllGroups'>
    | MakeActionCall<ConvoInfoVolatileWrapper, 'setGroup'>
    | MakeActionCall<ConvoInfoVolatileWrapper, 'eraseGroup'>
    | MakeActionCall<ConvoInfoVolatileWrapper, 'getCommunity'>
    | MakeActionCall<ConvoInfoVolatileWrapper, 'setCommunityByFullUrl'>
    | MakeActionCall<ConvoInfoVolatileWrapper, 'getAllCommunities'>
    | MakeActionCall<ConvoInfoVolatileWrapper, 'eraseCommunityByFullUrl'>;
}
