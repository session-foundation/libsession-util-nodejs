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

    setProConfig: (proConfig: ProConfigSet) => void;
    getProConfig: () => ProConfig | null;
    removeProConfig: () => boolean;

    setAnimatedAvatar: (enabled: boolean) => void;
    setProAccessExpiry: (expiryTsMs: number | null) => void;
    setProBadge: (enabled: boolean) => void;
    /**
     *
     * @returns 0 if no pro user features are enabled, the bitset of pro features enabled otherwise
     */
    getProProfileBitset: () => bigint;

    getProAccessExpiry: () => number | null;

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

    /**
     * Deterministically derive the rotating seed (and its ed25519 keypair) for `nowMs` from the Pro
     * master key (libsession owns the rotation schedule). Every device deriving from the same master
     * key + time converges on the same key, so concurrent proof (re)generations don't race. Feed the
     * priv key to a proof request and persist the seed via setProConfig once the backend returns a
     * signed proof.
     */
    deriveProRotatingKey: (args: { proMasterKeyHex: string; nowMs: number }) => {
      /** 32 bytes, 64 chars */
      rotatingSeedHex: string;
      /** 64 bytes, 128 chars */
      rotatingPrivKeyHex: string;
    };

    /** Refund-requested marker (config key R), in ms; null when unset or past the 1-week read gate. */
    getRefundRequested: () => number | null;
    setRefundRequested: (refundTsMs: number | null) => void;
    /** Pro-prepaid / purchase-in-flight marker (config key I), in ms; null when unset or gated. */
    getProPrepaid: () => number | null;
    setProPrepaid: (prepaidTsMs: number | null) => void;
    /**
     * Whether the subscription auto-renews (config key A), from get_pro_status.auto_renewing.
     *
     * Presence-only: core writes it with set_nonzero_int, so `setProAutoRenewing(false)` ERASES the
     * key rather than storing a false. The getter therefore returns false for all three of "not
     * auto-renewing", "never fetched yet" and "written by a client predating key A" — so treat
     * false as terminal/unknown, and never key a change check on whether the key is present.
     */
    getProAutoRenewing: () => boolean;
    setProAutoRenewing: (autoRenewing: boolean) => void;
    /**
     * The account's grace period (config key G), in MILLISECONDS, from the ACCOUNT-level
     * get_pro_status.grace_period_duration. 0 when unset.
     *
     * Synced alongside `E` so any linked device can derive when coverage ends:
     * `getProAccessExpiry() + getProGracePeriod()`. `E` is the account's true expiry — what has been
     * paid for, and the honest thing to show a user — and the backend keeps serving for `G` past it,
     * judging active/expired against that later instant. So `[E, E + G)` is expired-but-still-served.
     *
     * ⚠️ NOT the `gracePeriodDurationMs` on a get_pro_status response's `latestPayment`. That one is a
     * single store's raw declaration and is not gated on auto-renewal — a subscriber who cancels
     * mid-retry keeps a nonzero value in it, and treating it as the account's grace would place
     * coverage weeks past the truth. This key is "how much longer are we served"; the payment-level
     * field is "what did the store declare about one transaction".
     *
     * ⚠️ Only meaningful if `E` and `G` are written from the SAME get_pro_status response — they are
     * then consistent whatever grace was in force. Write them together; core clears `G` with `E`.
     *
     * Not optional, unlike the auto-renewing pair: the backend sends 0 when the subscription isn't
     * auto-renewing, and `E + 0 == E`, so an absent key and a stored zero describe the same account.
     * Milliseconds here (not seconds like setNoteToSelfExpiry) to match the rest of the Pro accessors
     * and the `gracePeriodDurationMs` field callers receive from get_pro_status; core stores seconds
     * and the conversion floors.
     */
    getProGracePeriod: () => number;
    setProGracePeriod: (graceMs: number) => void;
    /**
     * When to (re)request a proof given `nowMs`: nowMs (request now), a future ms (preemptive
     * renewal ~1h before expiry), or null (don't renew). Supersedes bespoke auto-renew logic.
     */
    getProRenewalTarget: (nowMs: number) => number | null;
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
    public getProAccessExpiry: UserConfigWrapper['getProAccessExpiry'];
    public setProAccessExpiry: UserConfigWrapper['setProAccessExpiry'];
    public getProProfileBitset: UserConfigWrapper['getProProfileBitset'];
    public setAnimatedAvatar: UserConfigWrapper['setAnimatedAvatar'];
    public setProBadge: UserConfigWrapper['setProBadge'];

    public generateProMasterKey: UserConfigWrapper['generateProMasterKey'];
    public generateRotatingPrivKeyHex: UserConfigWrapper['generateRotatingPrivKeyHex'];
    public deriveProRotatingKey: UserConfigWrapper['deriveProRotatingKey'];
    public getRefundRequested: UserConfigWrapper['getRefundRequested'];
    public setRefundRequested: UserConfigWrapper['setRefundRequested'];
    public getProPrepaid: UserConfigWrapper['getProPrepaid'];
    public setProPrepaid: UserConfigWrapper['setProPrepaid'];
    public getProAutoRenewing: UserConfigWrapper['getProAutoRenewing'];
    public setProAutoRenewing: UserConfigWrapper['setProAutoRenewing'];
    public getProGracePeriod: UserConfigWrapper['getProGracePeriod'];
    public setProGracePeriod: UserConfigWrapper['setProGracePeriod'];
    public getProRenewalTarget: UserConfigWrapper['getProRenewalTarget'];
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
    | MakeActionCall<UserConfigWrapper, 'setProAccessExpiry'>
    | MakeActionCall<UserConfigWrapper, 'getProAccessExpiry'>
    | MakeActionCall<UserConfigWrapper, 'setProBadge'>
    | MakeActionCall<UserConfigWrapper, 'generateProMasterKey'>
    | MakeActionCall<UserConfigWrapper, 'generateRotatingPrivKeyHex'>
    | MakeActionCall<UserConfigWrapper, 'deriveProRotatingKey'>
    | MakeActionCall<UserConfigWrapper, 'getRefundRequested'>
    | MakeActionCall<UserConfigWrapper, 'setRefundRequested'>
    | MakeActionCall<UserConfigWrapper, 'getProPrepaid'>
    | MakeActionCall<UserConfigWrapper, 'setProPrepaid'>
    | MakeActionCall<UserConfigWrapper, 'getProAutoRenewing'>
    | MakeActionCall<UserConfigWrapper, 'setProAutoRenewing'>
    | MakeActionCall<UserConfigWrapper, 'getProGracePeriod'>
    | MakeActionCall<UserConfigWrapper, 'setProGracePeriod'>
    | MakeActionCall<UserConfigWrapper, 'getProRenewalTarget'>;
}
