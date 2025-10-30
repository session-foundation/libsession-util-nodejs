/// <reference path="../shared.d.ts" />

declare module 'libsession_util_nodejs' {
  type WithProRotatingEd25519PrivKey = { proRotatingEd25519PrivKey: string | null };

  type WithProBackendPubkey = {
    /**
     * HexString
     */
    proBackendPubkeyHex: string;
  };

  type ProStatus = 'InvalidProBackendSig' | 'InvalidUserSig' | 'Valid' | 'Expired';
  type ProFeature = '10K_CHARACTER_LIMIT' | 'PRO_BADGE' | 'ANIMATED_AVATAR';
  type ProFeatures = Array<ProFeature>;
  type WithProFeatures = { proFeatures: ProFeatures };
  type WithGenIndexHash = { genIndexHashB64: string };

  type ProProof = WithGenIndexHash & {
    version: number;
    /**
     * HexString, 64 chars
     */
    rotatingPubkeyHex: string;
    expiryMs: number;
  };

  type ProConfig = {
    /**
     * 64 bytes, 128 chars
     */
    rotatingPrivKeyHex: string;
    proProof: ProProof;
  };

  type WithProBackendResponse = {
    status: number;
    errors: Array<string>;
  };

  // Must match session-desktop
  export enum ProOriginatingPlatform {
    Nil = 'Nil',
    GooglePlayStore = 'Google',
    iOSAppStore = 'iOS',
  }

  export type ProBackendProviderConstantType = {
    device: string;
    store: string;
    store_other: string;
    platform: string;
    platform_account: string;
    refund_url: string;
    refund_after_platform_deadline_url: string;
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

  type ProWrapper = {
    proFeaturesForMessage: (args: {
      utf16: string;
      /**
       * If the utf16 requires 10K_CHARACTER_LIMIT to be set, it will be set in the return.
       * If provided (here) as an input, it will be ignored.
       */
      proFeatures: ProFeatures;
    }) => WithProFeatures & { success: boolean; error: string | null; codepointCount: number };
    proProofRequestBody: (args: {
      requestVersion: number;
      masterPrivkey: Uint8Array;
      rotatingPrivkey: Uint8Array;
      unixTsMs: number;
    }) => string;

    proProofParseResponse: (args: {
      json: string;
    }) => WithProBackendResponse & { proof: ProProof | null };

    /**
     * @param version: Request version. The latest accepted version is 0
     * @param ticket: 4-byte monotonic integer for the caller's revocation list iteration. Set to 0 if unknown; otherwise, use the latest known `ticket` from a prior `GetProRevocationsResponse` to allow
    /// the Session Pro Backend to omit the revocation list if it has not changed.
     * @returns the stringified body to include in the request
     */
    proRevocationsRequestBody: (args: { requestVersion: number; ticket: number }) => string;

    proRevocationsParseResponse: (args: { json: string }) => WithProBackendResponse & {
      ticket: number | null;
      items: Array<ProRevocationItem>;
    };

    proStatusRequestBody: (args: {
      requestVersion: number;
      masterPrivkey: Uint8Array;
      unixTsMs: number;
      withPaymentHistory: boolean;
    }) => string;

    proStatusParseResponse: (args: { json: string }) => WithProBackendResponse & {
      ticket: number | null;
      items: Array<ProPaymentItem>;
      userStatus: number;
      errorReport: number;
      autoRenewing: boolean;
      expiryUnixTsMs: number;
      gracePeriodDurationMs: number;
    };
  };

  export type ProActionsCalls = MakeWrapperActionCalls<ProWrapper>;

  /**
   * To be used inside the web worker only (calls are synchronous and won't work asynchronously)
   */
  export class ProWrapperNode {
    public static proFeaturesForMessage: ProWrapper['proFeaturesForMessage'];
    public static proProofRequestBody: ProWrapper['proProofRequestBody'];
    public static proProofParseResponse: ProWrapper['proProofParseResponse'];
    public static proRevocationRequestBody: ProWrapper['proRevocationsRequestBody'];
    public static proRevocationParseResponse: ProWrapper['proRevocationsParseResponse'];
    public static proStatusRequestBody: ProWrapper['proStatusRequestBody'];
    public static proStatusParseResponse: ProWrapper['proStatusParseResponse'];
  }

  /**
   * Those actions are used internally for the web worker communication.
   * You should never need to import them in Session directly
   * You will need to add an entry here if you add a new function
   */
  export type ProActionsType =
    | MakeActionCall<ProWrapper, 'proFeaturesForMessage'>
    | MakeActionCall<ProWrapper, 'proProofRequestBody'>
    | MakeActionCall<ProWrapper, 'proProofParseResponse'>
    | MakeActionCall<ProWrapper, 'proRevocationsRequestBody'>
    | MakeActionCall<ProWrapper, 'proRevocationsParseResponse'>
    | MakeActionCall<ProWrapper, 'proStatusRequestBody'>
    | MakeActionCall<ProWrapper, 'proStatusParseResponse'>;
}
