/// <reference path="../shared.d.ts" />

declare module 'libsession_util_nodejs' {
  type WithEncryptedData = { encryptedData: Uint8Array };
  type WithPlaintext = { plaintext: Uint8Array };
  type WithSentTimestampMs = {
    /** in milliseconds */
    sentTimestampMs: number;
  };
  type WithSenderEd25519Seed = {
    /**
     * 32 bytes
     */
    senderEd25519Seed: Uint8Array;
  };
  type WithRecipientPubkey = { recipientPubkey: string };
  type WithCommunityPubkey = { communityPubkey: string };
  type WithGroupEd25519Pubkey = { groupEd25519Pubkey: string };
  type WithGroupEncKey = { groupEncKey: string };
  type WithProRotatingEd25519PrivKey = { proRotatingEd25519PrivKey: string | null };

  type WithContentOrEnvelope = { contentOrEnvelope: Uint8Array };
  type WithContentPlaintext = {
    contentPlaintextUnpadded: Uint8Array;
  };
  type WithNowMs = { nowMs: number };
  type WithProBackendPubkey = {
    /**
     * HexString
     */
    proBackendPubkeyHex: string;
  };

  type ProProof = {
    version: number;
    genIndexHashB64: string;
    rotatingPubkeyHex: string;
    expiryMs: number;
  };

  type WithProProof = {
    proProof: ProProof;
  };
  type Envelope = {
    timestampMs: number;
    /**
     * HexString, 33 bytes, 66 chars
     */
    source: string | null;
    /**
     * HexString
     */
    proSigHex: string | null;
  };

  type WithDecryptedEnvelope = {
    envelope: Envelope | null;
  };

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
    }) => WithEncryptedData & { encryptionKey: Uint8Array };
    /**
     *
     * Throws if the decryption fails
     */
    attachmentDecrypt: (opts: WithEncryptedData & { decryptionKey: Uint8Array }) => {
      decryptedData: Uint8Array;
    };

    encryptFor1o1: (
      opts: Array<
        WithPlaintext &
          WithSentTimestampMs &
          WithSenderEd25519Seed &
          WithRecipientPubkey &
          WithProRotatingEd25519PrivKey
      >
    ) => { encryptedData: Array<Uint8Array> };

    encryptForCommunityInbox: (
      opts: Array<
        WithPlaintext &
          WithSentTimestampMs &
          WithSenderEd25519Seed &
          WithRecipientPubkey &
          WithCommunityPubkey &
          WithProRotatingEd25519PrivKey
      >
    ) => { encryptedData: Array<Uint8Array> };

    encryptForCommunity: (opts: Array<WithPlaintext & WithProRotatingEd25519PrivKey>) => {
      encryptedData: Array<Uint8Array>;
    };

    encryptForGroup: (
      opts: Array<
        WithPlaintext &
          WithSenderEd25519Seed &
          WithSentTimestampMs &
          WithGroupEd25519Pubkey &
          WithGroupEncKey &
          WithProRotatingEd25519PrivKey
      >
    ) => { encryptedData: Array<Uint8Array> };

    decryptForCommunity: (
      first: Array<WithContentOrEnvelope>,
      second: WithNowMs & WithProBackendPubkey
    ) => Array<WithProProof & WithDecryptedEnvelope & WithContentPlaintext>;
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
    public static encryptForCommunityInbox: MultiEncryptWrapper['encryptForCommunityInbox'];
    public static encryptForCommunity: MultiEncryptWrapper['encryptForCommunity'];
    public static encryptForGroup: MultiEncryptWrapper['encryptForGroup'];

    public static decryptForCommunity: MultiEncryptWrapper['decryptForCommunity'];
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
    | MakeActionCall<MultiEncryptWrapper, 'encryptFor1o1'>
    | MakeActionCall<MultiEncryptWrapper, 'encryptForCommunityInbox'>
    | MakeActionCall<MultiEncryptWrapper, 'encryptForCommunity'>
    | MakeActionCall<MultiEncryptWrapper, 'encryptForGroup'>
    | MakeActionCall<MultiEncryptWrapper, 'decryptForCommunity'>;
}
