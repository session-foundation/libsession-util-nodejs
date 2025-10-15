/// <reference path="../shared.d.ts" />

declare module 'libsession_util_nodejs' {
  type MultiEncryptWrapper = {
    multiEncrypt: (opts: {
      /**
       * len 64: ed25519 secretKey with pubkey
       */
      ed25519SecretKey: Uint8ArrayLen64;
      domain: EncryptionDomain;
      messages: Array<Uint8Array>;
      recipients: Array<Uint8Array>;
    }) => Uint8Array;
    multiDecryptEd25519: (opts: {
      encoded: Uint8Array;
      /**
       * len 64: ed25519 secretKey with pubkey
       */
      userEd25519SecretKey: Uint8ArrayLen64;
      senderEd25519Pubkey: Uint8Array;
      domain: EncryptionDomain;
    }) => Uint8Array | null;
    /**
     * Throws if the encryption fails
     */
    attachmentEncrypt: (opts: {
      seed: Uint8Array;
      data: Uint8Array;
      domain: 'attachment' | 'profilePic';
      allowLarge: boolean;
    }) => { encryptedData: Uint8Array; encryptionKey: Uint8Array };
    /**
     *
     * Throws if the decryption fails
     */
    attachmentDecrypt: (opts: { encryptedData: Uint8Array; decryptionKey: Uint8Array }) => {
      decryptedData: Uint8Array;
    };

    encryptFor1o1: (
      opts: Array<{
        plaintext: Uint8Array;
        sentTimestampMs: number;
        ed25519Privkey: Uint8Array;
        recipientPubkey: Uint8Array;
        proRotatingEd25519Privkey?: Uint8Array;
      }>
    ) => Array<Uint8Array>;
  };

  export type MultiEncryptActionsCalls = MakeWrapperActionCalls<MultiEncryptWrapper>;

  /**
   * To be used inside the web worker only (calls are synchronous and won't work asynchronously)
   */
  export class MultiEncryptWrapperNode {
    public static multiEncrypt: MultiEncryptWrapper['multiEncrypt'];
    public static multiDecryptEd25519: MultiEncryptWrapper['multiDecryptEd25519'];
    public static attachmentDecrypt: MultiEncryptWrapper['attachmentDecrypt'];
    public static attachmentEncrypt: MultiEncryptWrapper['attachmentEncrypt'];
    public static encryptFor1o1: MultiEncryptWrapper['encryptFor1o1'];
  }

  /**
   * Those actions are used internally for the web worker communication.
   * You should never need to import them in Session directly
   * You will need to add an entry here if you add a new function
   */
  export type MultiEncryptActionsType =
    | MakeActionCall<MultiEncryptWrapper, 'multiEncrypt'>
    | MakeActionCall<MultiEncryptWrapper, 'multiDecryptEd25519'>
    | MakeActionCall<MultiEncryptWrapper, 'attachmentDecrypt'>
    | MakeActionCall<MultiEncryptWrapper, 'attachmentEncrypt'>
    | MakeActionCall<MultiEncryptWrapper, 'encryptFor1o1'>;
}
