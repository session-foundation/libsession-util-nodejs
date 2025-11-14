/// <reference path="../shared.d.ts" />
/// <reference path="../pro/pro.d.ts" />

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
  type WithEd25519PrivateKeyHex = {
    /**
     * This is the current user identity private key, HexString
     */
    ed25519PrivateKeyHex: string;
  };
  type WithEd25519GroupPubkeyHex = {
    /**
     * This is the group identity pubkey, HexString
     */
    ed25519GroupPubkeyHex: string;
  };
  type WithGroupEncryptionKeys = {
    /**
     * This should be what is returned by the 03-group `MetaGroupWrapper::keyGetAll()`
     */
    groupEncKeys: Array<Uint8Array>;
  };

  type WithContentOrEnvelope = { contentOrEnvelope: Uint8Array };
  type WithEnvelopePayload = { envelopePayload: Uint8Array };
  type WithContentPlaintext = {
    contentPlaintextUnpadded: Uint8Array;
  };
  type WithServerId = {
    serverId: number;
  };
  type WithMessageHash = {
    /**
     * Base64 string
     */
    messageHash: string;
  };
  type WithNowMs = { nowMs: number };

  type DecodedPro = WithProFeaturesBitset & {
    proStatus: ProStatus;
    proProof: ProProof;
  };

  type WithDecodedPro = {
    decodedPro: DecodedPro | null;
  };
  type WithProSigHex = {
    /**
     * HexString
     */
    proSigHex: string | null;
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

  type WithEnvelope = {
    envelope: Envelope | null;
  };

  type WithNonNullableEnvelope = {
    envelope: Envelope;
  };

  type WithDecodedEnvelope = {
    decodedEnvelope:
      | WithNonNullableEnvelope &
          WithContentPlaintext &
          WithDecodedPro & {
            /**
             * HexString with 05 prefix of the author of that message
             */
            sessionId: string;
          };
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
      first: Array<WithContentOrEnvelope & WithServerId>,
      second: WithNowMs & WithProBackendPubkey
    ) => Array<WithDecodedPro & WithProSigHex & WithEnvelope & WithContentPlaintext & WithServerId>;

    decryptFor1o1: (
      first: Array<WithEnvelopePayload & WithMessageHash>,
      second: WithNowMs & WithProBackendPubkey & WithEd25519PrivateKeyHex
    ) => Array<WithDecodedEnvelope & WithMessageHash>;

    decryptForGroup: (
      first: Array<WithEnvelopePayload & WithMessageHash>,
      second: WithNowMs & WithProBackendPubkey & WithEd25519GroupPubkeyHex & WithGroupEncryptionKeys
    ) => Array<WithDecodedEnvelope & WithMessageHash>;
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
    public static decryptFor1o1: MultiEncryptWrapper['decryptFor1o1'];
    public static decryptForGroup: MultiEncryptWrapper['decryptForGroup'];
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
    | MakeActionCall<MultiEncryptWrapper, 'decryptForCommunity'>
    | MakeActionCall<MultiEncryptWrapper, 'decryptFor1o1'>
    | MakeActionCall<MultiEncryptWrapper, 'decryptForGroup'>;
}
