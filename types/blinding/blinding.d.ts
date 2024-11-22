/// <reference path="../../shared.d.ts" />

declare module 'libsession_util_nodejs' {
  type BlindingWrapper = {
    blindVersionPubkey: (opts: {
      /**
       * len 64: ed25519 secretKey with pubkey
       */
      ed25519SecretKey: Uint8Array;
    }) => string;
    blindVersionSignRequest: (opts: {
      /**
       * len 64: ed25519 secretKey with pubkey
       */
      ed25519SecretKey: Uint8Array;
      sigTimestampSeconds: number;
      sigMethod: string;
      sigPath: string;
      sigBody: Uint8Array | null;
    }) => Uint8Array;
    blindVersionSign: (opts: {
      /**
       * len 64: ed25519 secretKey with pubkey
       */
      ed25519SecretKey: Uint8Array;
      sigTimestampSeconds: number;
    }) => Uint8Array;
  };

  export type BlindingActionsCalls = MakeWrapperActionCalls<BlindingWrapper>;

  /**
   * To be used inside the web worker only (calls are synchronous and won't work asynchronously)
   */
  export class BlindingWrapperNode {
    public static blindVersionPubkey: BlindingWrapper['blindVersionPubkey'];
    public static blindVersionSignRequest: BlindingWrapper['blindVersionSignRequest'];
    public static blindVersionSign: BlindingWrapper['blindVersionSign'];
  }

  /**
   * Those actions are used internally for the web worker communication.
   * You should never need to import them in Session directly
   * You will need to add an entry here if you add a new function
   */
  export type BlindingActionsType =
    | MakeActionCall<BlindingWrapper, 'blindVersionPubkey'>
    | MakeActionCall<BlindingWrapper, 'blindVersionSignRequest'>
    | MakeActionCall<BlindingWrapper, 'blindVersionSign'>;
}
