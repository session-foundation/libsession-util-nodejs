/// <reference path="../shared.d.ts" />;

declare module 'libsession_util_nodejs' {
  type WithProRotatingEd25519PrivKey = { proRotatingEd25519PrivKey: string | null };

  type WithProBackendPubkey = {
    /**
     * HexString
     */
    proBackendPubkeyHex: string;
  };

  type ProStatus = 'InvalidProBackendSig' | 'InvalidUserSig' | 'Valid';
  type WithProFeaturesBitset = { proFeaturesBitset: bigint };
  type WithGenIndexHash = { genIndexHashB64: string };

  type WithRequestVersion = { requestVersion: number };
  type WithTicket = { ticket: number };

  type WithUnixTsMs = {
    unixTsMs: number;
  };

  type ProProof = WithGenIndexHash & {
    version: number;
    /**
     * HexString, 64 chars
     */
    rotatingPubkeyHex: string;
    expiryMs: number;
    /**
     * signature of the pro proof provided by the backend (hex)
     * 64 bytes, 128 chars
     */
    signatureHex: string;
  };

  type WithRotatingPrivKeyHex = {
    /**
     * 64 bytes, 128 chars
     */
    rotatingPrivKeyHex: string;
  };

  type ProConfig = WithRotatingPrivKeyHex & {
    proProof: ProProof;
  };

  export type ProOriginatingPlatform = 'Nil' | 'Google' | 'iOS';

  export type ProBackendProviderConstantType = {
    device: string;
    store: string;
    store_other: string;
    platform: string;
    platform_account: string;
    refund_support_url: string;
    refund_platform_url: string;
    update_subscription_url: string;
    cancel_subscription_url: string;
  };

  export type ProBackendProviderConstantsType = Record<
    ProOriginatingPlatform,
    ProBackendProviderConstantType
  >;

  export type ProBackendUrlsType = {
    roadmap: string;
    privacy_policy: string;
    terms_of_service: string;
    pro_access_not_found: string;
    support_url: string;
  };

  type ProRevocationItem = WithGenIndexHash & {
    expiryUnixTsMs: number;
  };

  type WithMasterPrivKeyHex = { masterPrivKeyHex: string };

  type ProPaymentItem = {
    /**
     * Describes the current status of the consumption of the payment for Session Pro entitlement
     * The status should be used to determine which timestamps should be used.

     * For example, a payment can be in a redeemed state whilst also have a refunded timestamp set
     * if the payment was refunded and then the refund was reversed. We preserve all timestamps for
     * book-keeping purposes.
     */
    status: 'NIL' | 'UNREDEEMED' | 'REDEEMED' | 'EXPIRED' | 'REFUNDED';
    /**
     * Session Pro product/plan item that was purchased
     */
    plan: 'NIL' | 'ONE_MONTH' | 'THREE_MONTHS' | 'TWELVE_MONTHS';
    /**
     * Store front that this particular payment came from
     */
    paymentProvider: ProOriginatingPlatform;
    /**
     * Flag indicating whether or not this payment will automatically bill itself at the end of the
     billing cycle.
     */
    autoRenewing: boolean;
    /**
     * Unix timestamp of when the payment was witnessed by the Pro Backend. Always set
     */
    unredeemedTsMs: number;
    /**
     * Unix timestamp of when the payment was redeemed. 0 if not activated
     */
    redeemedTsMs: number;
    /**
     * Unix timestamp of when the payment was expiry. 0 if not activated
     */
    expiryTsMs: number;
    /**
     * Duration of the grace period, e.g. when the payment provider will start to attempt to renew
     * the Session Pro subscription. During the period between
     * [expiry_unix_ts, expiry_unix_ts + grace_period_duration_ms] the user continues to have
     * entitlement to Session Pro. This value is only applicable if `auto_renewing` is `true`.
     */
    gracePeriodDurationMs: number;
    /**
     *  Unix deadline timestamp of when the user is able to refund the subscription via the payment
     provider.
     * Thereafter the user must initiate a refund manually via Session support.
     */
    platformRefundExpiryTsMs: number;
    /**
     * Unix timestamp of when the payment was revoked or refunded. 0 if not applicable.
     */
    revokedTsMs: number;
    /**
     * When payment provider is set to Google Play Store, this is the platform-specific purchase
     token.
     * This information should be considered as confidential and stored appropriately.
     */
    googlePaymentToken: string | null;
    /**
     * When payment provider is set to iOS App Store, this is the platform-specific original
     transaction ID.
     * This information should be considered as confidential and stored appropriately.
     */
    appleOriginalTxId: string | null;
    /**
     * When payment provider is set to iOS App Store, this is the platform-specific transaction ID
     * This information should be considered as confidential and stored appropriately.
     */
    appleTxId: string | null;
    /**
     * When payment provider is set to iOS App Store, this is the platform-specific web line order
     *  ID.
     * This information should be considered as confidential and stored appropriately.
     */
    appleWebLineOrderId: string | null;
  };

  type ProWrapper = {
    proFeaturesForMessage: (
      args: WithProFeaturesBitset & {
        utf16: string;
      }
    ) => WithProFeaturesBitset & {
      status: 'SUCCESS' | 'UTF_DECODING_ERROR' | 'EXCEEDS_CHARACTER_LIMIT';
    };
    proProofRequestBody: (
      args: WithMasterPrivKeyHex & WithRequestVersion & WithUnixTsMs & WithRotatingPrivKeyHex
    ) => string;

    /**
     * @param version: Request version. The latest accepted version is 0
     * @param ticket: 4-byte monotonic integer for the caller's revocation list iteration. Set to 0 if unknown; otherwise, use the latest known `ticket` from a prior `GetProRevocationsResponse` to allow
     the Session Pro Backend to omit the revocation list if it has not changed.
     * @returns the stringified body to include in the request
     */
    proRevocationsRequestBody: (args: WithRequestVersion & WithTicket) => string;

    proStatusRequestBody: (
      args: WithMasterPrivKeyHex &
        WithRequestVersion &
        WithUnixTsMs & {
          count: number;
        }
    ) => string;
  };

  export type ProActionsCalls = MakeWrapperActionCalls<ProWrapper>;

  /**
   * To be used inside the web worker only (calls are synchronous and won't work asynchronously)
   */
  export class ProWrapperNode {
    public static proFeaturesForMessage: ProWrapper['proFeaturesForMessage'];
    public static proProofRequestBody: ProWrapper['proProofRequestBody'];
    public static proRevocationsRequestBody: ProWrapper['proRevocationsRequestBody'];
    public static proStatusRequestBody: ProWrapper['proStatusRequestBody'];
  }

  /**
   * Those actions are used internally for the web worker communication.
   * You should never need to import them in Session directly
   * You will need to add an entry here if you add a new function
   */
  export type ProActionsType =
    | MakeActionCall<ProWrapper, 'proFeaturesForMessage'>
    | MakeActionCall<ProWrapper, 'proProofRequestBody'>
    | MakeActionCall<ProWrapper, 'proRevocationsRequestBody'>
    | MakeActionCall<ProWrapper, 'proStatusRequestBody'>;
}
