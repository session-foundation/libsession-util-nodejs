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

  type ProProof = {
    version: number;
    genIndexHashB64: string;
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
      requestVersion: string,
      masterPrivkey: Uint8Array,
      rotatingPrivkey: Uint8Array,
      unixTsMs: number,
    }
    ) => string;
  };

  export type ProActionsCalls = MakeWrapperActionCalls<ProWrapper>;


  /**
   * To be used inside the web worker only (calls are synchronous and won't work asynchronously)
   */
  export class ProWrapperNode {
    public static proFeaturesForMessage: ProWrapper['proFeaturesForMessage'];
  }

  /**
   * Those actions are used internally for the web worker communication.
   * You should never need to import them in Session directly
   * You will need to add an entry here if you add a new function
   */
  export type ProActionsType = MakeActionCall<ProWrapper, 'proFeaturesForMessage'>;
}
