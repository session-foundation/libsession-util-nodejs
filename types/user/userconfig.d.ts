/// <reference path="../shared.d.ts" />
/// <reference path="../pro/pro.d.ts" />

declare module 'libsession_util_nodejs' {
  /**
   *
   * User config wrapper logic
   *
   */

  type UserConfigWrapper = BaseConfigWrapper & {
    init: (secretKey: Uint8Array, dump: Uint8Array | null) => void;
    /** This function is used to free wrappers from memory only */
    free: () => void;
    getPriority: () => number;
    getName: () => string | null;
    getProfilePic: () => ProfilePicture;
    setPriority: (priority: number) => void;
    setName: (name: string) => void;
    setNameTruncated: (name: string) => void;

    /**
     * Call this when uploading a new profile picture (i.e. not an auto reupload)
     */
    setNewProfilePic: (pic: ProfilePicture) => void;
    /**
     * Call this when reuploading a the previous profile picture
     */
    setReuploadProfilePic: (pic: ProfilePicture) => void;

    getProfileUpdatedSeconds: () => number;

    setEnableBlindedMsgRequest: (msgRequest: boolean) => void;
    getEnableBlindedMsgRequest: () => boolean | undefined;
    setNoteToSelfExpiry: (expirySeconds: number) => void;
    /**
     * @returns the expiry in seconds, 0 if off, undefined if not set
     */
    getNoteToSelfExpiry: () => number | undefined;

    setProConfig: (proConfig: ProConfig) => void;
    getProConfig: () => ProConfig | null;
    removeProConfig: () => boolean;

    setAnimatedAvatar: (enabled: boolean) => void;
    setProBadge: (enabled: boolean) => void;
    /**
     *
     * @returns 0 if no pro user features are enabled, the bitset of pro features enabled otherwise
     */
    getProProfileBitset: () => bigint;

    generateProMasterKey: ({
      ed25519SeedHex,
    }: {
      /**
       * HexString, 64 chars
       */
      ed25519SeedHex: string;
    }) => {
      /**
       * 64 bytes, 128 chars
       */
      proMasterKeyHex: string;
    };

    /**
     * Generates a new rotating private key for the user.
     * Note: this should only be done once per device, and saved to the DB or the extra_data of `UserProfile`.
     */
    generateRotatingPrivKeyHex: () => WithRotatingPrivKeyHex;
  };

  export type UserConfigWrapperActionsCalls = MakeWrapperActionCalls<UserConfigWrapper>;

  /**
   * To be used inside the web worker only (calls are synchronous and won't work asynchronously)
   */
  export class UserConfigWrapperNode extends BaseConfigWrapperNode {
    constructor(secretKey: Uint8Array, dump: Uint8Array | null);
    public getPriority: UserConfigWrapper['getPriority'];
    public getName: UserConfigWrapper['getName'];
    public getProfilePic: UserConfigWrapper['getProfilePic'];
    public setPriority: UserConfigWrapper['setPriority'];
    public setName: UserConfigWrapper['setName'];
    public setNameTruncated: UserConfigWrapper['setNameTruncated'];
    public setNewProfilePic: UserConfigWrapper['setNewProfilePic'];
    public setReuploadProfilePic: UserConfigWrapper['setReuploadProfilePic'];
    public getProfileUpdatedSeconds: UserConfigWrapper['getProfileUpdatedSeconds'];
    public getEnableBlindedMsgRequest: UserConfigWrapper['getEnableBlindedMsgRequest'];
    public setEnableBlindedMsgRequest: UserConfigWrapper['setEnableBlindedMsgRequest'];
    public getNoteToSelfExpiry: UserConfigWrapper['getNoteToSelfExpiry'];
    public setNoteToSelfExpiry: UserConfigWrapper['setNoteToSelfExpiry'];
    public getProConfig: UserConfigWrapper['getProConfig'];
    public setProConfig: UserConfigWrapper['setProConfig'];
    public removeProConfig: UserConfigWrapper['removeProConfig'];
    public getProProfileBitset: UserConfigWrapper['getProProfileBitset'];
    public setAnimatedAvatar: UserConfigWrapper['setAnimatedAvatar'];
    public setProBadge: UserConfigWrapper['setProBadge'];

    public generateProMasterKey: UserConfigWrapper['generateProMasterKey'];
    public generateRotatingPrivKeyHex: UserConfigWrapper['generateRotatingPrivKeyHex'];
  }

  /**
   * Those actions are used internally for the web worker communication.
   * You should never need to import them in Session directly
   * You will need to add an entry here if you add a new function
   */
  export type UserConfigActionsType =
    | ['init', Uint8Array, Uint8Array | null]
    | MakeActionCall<UserConfigWrapper, 'free'>
    | MakeActionCall<UserConfigWrapper, 'getPriority'>
    | MakeActionCall<UserConfigWrapper, 'getName'>
    | MakeActionCall<UserConfigWrapper, 'getProfilePic'>
    | MakeActionCall<UserConfigWrapper, 'setPriority'>
    | MakeActionCall<UserConfigWrapper, 'setName'>
    | MakeActionCall<UserConfigWrapper, 'setNameTruncated'>
    | MakeActionCall<UserConfigWrapper, 'setNewProfilePic'>
    | MakeActionCall<UserConfigWrapper, 'setReuploadProfilePic'>
    | MakeActionCall<UserConfigWrapper, 'getProfileUpdatedSeconds'>
    | MakeActionCall<UserConfigWrapper, 'getEnableBlindedMsgRequest'>
    | MakeActionCall<UserConfigWrapper, 'setEnableBlindedMsgRequest'>
    | MakeActionCall<UserConfigWrapper, 'getNoteToSelfExpiry'>
    | MakeActionCall<UserConfigWrapper, 'setNoteToSelfExpiry'>
    | MakeActionCall<UserConfigWrapper, 'getProConfig'>
    | MakeActionCall<UserConfigWrapper, 'setProConfig'>
    | MakeActionCall<UserConfigWrapper, 'removeProConfig'>
    | MakeActionCall<UserConfigWrapper, 'getProProfileBitset'>
    | MakeActionCall<UserConfigWrapper, 'setAnimatedAvatar'>
    | MakeActionCall<UserConfigWrapper, 'setProBadge'>
    | MakeActionCall<UserConfigWrapper, 'generateProMasterKey'>
    | MakeActionCall<UserConfigWrapper, 'generateRotatingPrivKeyHex'>;
}
