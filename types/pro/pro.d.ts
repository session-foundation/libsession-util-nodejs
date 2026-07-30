/// <reference path="../shared.d.ts" />;

declare module 'libsession_util_nodejs' {
  type WithProRotatingEd25519PrivKey = { proRotatingEd25519PrivKey: string | null };

  type WithProBackendPubkey = {
    /**
     * HexString
     */
    proBackendPubkeyHex: string;
  };

  type ProStatus = 'ValidOrExpired' | 'Invalid';
  type WithProProfileBitset = { proProfileBitset: bigint };
  type WithProMessageBitset = { proMessageBitset: bigint };
  /**
   * base64 of the proof's revocation tag (historically the "gen index hash")
   */
  type WithRevocationTag = { revocationTagB64: string };

  type WithTicket = { ticket: number };

  type WithUnixTsMs = {
    unixTsMs: number;
  };

  type ProProof = WithRevocationTag & {
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

  type WithRotatingSeedHex = {
    /**
     * 32 bytes, 64 chars
     */
    rotatingSeedHex: string;
  };

  type ProConfig = WithRotatingPrivKeyHex & {
    proProof: ProProof;
  };

  type ProConfigSet = WithRotatingSeedHex & {
    proProof: Omit<ProProof, 'rotatingPubkeyHex'>;
  };

  /**
   * General (non per-provider) Pro URLs. These are libsession-owned constants (not translation data)
   * still surfaced via LIBSESSION_PRO_URLS. Per-provider URLs are fetched via ProWrapper.providerUrls().
   */
  export type ProBackendUrlsType = {
    roadmap: string;
    privacy_policy: string;
    terms_of_service: string;
    pro_access_not_found: string;
    support_url: string;
  };

  /**
   * Per-provider support/management URLs, looked up by provider slug via ProWrapper.providerUrls().
   * `null` for a provider with no applicable URLs (unknown slug, or e.g. rangeproof).
   */
  type ProviderUrls = {
    refundPlatformUrl: string;
    refundSupportUrl: string;
    refundStatusUrl: string;
    updateSubscriptionUrl: string;
    cancelSubscriptionUrl: string;
  };

  type WithMasterPrivKeyHex = { masterPrivKeyHex: string };

  /**
   * A request to POST to the Session Pro backend. libsession owns the endpoint<->body pairing.
   */
  type ProRequest = {
    /**
     * Endpoint path relative to the backend base URL, e.g. "generate_pro_proof".
     */
    endpoint: string;
    /**
     * The value to send as the request's `Content-Type` header — relay verbatim; do not assume a format.
     */
    contentType: string;
    /**
     * The opaque request payload to POST. Relay it untouched — do not parse, inspect, or modify it.
     */
    body: string;
  };

  /**
   * A parsed backend response (§5). Check `status === 'ok'` before using the typed payload.
   */
  type WithProResponseHeader = {
    /**
     * Outcome category (closed set): 'ok' = success; 'fail' = client input / precondition rejected;
     * 'error' = backend fault (retryable).
     */
    status: 'ok' | 'fail' | 'error';
    /**
     * On non-'ok', a stable machine slug (spec §5.1) — map known ones to a localized string, fall back
     * to `error` for an unrecognized slug. null on success.
     */
    errorCode: string | null;
    /**
     * On non-'ok', an English diagnostic — NOT user-facing (show only when the slug has no i18n entry);
     * always safe to log. null on success.
     */
    error: string | null;
  };

  type GenerateProProofResponse = WithProResponseHeader & {
    proof: ProProof;
  };

  type ProRevocationItem = WithRevocationTag & {
    /**
     * A matching proof is revoked once the client clock reaches this unix instant (milliseconds).
     */
    effectiveMs: number;
  };

  type GetProRevocationsResponse = WithProResponseHeader & {
    ticket: number;
    /**
     * Absolute unix instant (ms) at which to next poll the revocation list — already `now + retry_in`
     * (libsession clamps the delay to a sane range), so callers can feed it straight to a next-run
     * scheduler without any arithmetic.
     */
    retryAtMs: number;
    /**
     * Duration (ms) to retain each item after first seeing it — applied per item as `seenAt + retainForMs`
     * (memory-only aging); stays a duration since each item is seen at a different time.
     */
    retainForMs: number;
    items: Array<ProRevocationItem>;
  };

  /**
   * A single Pro payment item. `status` is the numeric payment-status enum
   * (0=Nil,1=Unredeemed,2=Redeemed,3=Expired,4=Revoked). provider/plan are opaque wire slugs.
   */
  type ProPaymentItem = {
    /**
     * Opaque payment-status code slug: "unredeemed"/"redeemed"/"expired"/"revoked" (unknowns pass through).
     */
    status: string;
    /**
     * Parsed billing period (libsession parses the closed `plan` grammar, §1). `planUnit` is
     * one of "second"/"day"/"week"/"month"/"year"/"lifetime" (preserved as transmitted, not
     * canonicalized). `planCount` is >= 1 for periodic units and 0 for "lifetime". Client localizes.
     */
    planCount: number;
    planUnit: string;
    /**
     * Provider slug, e.g. "google_play"/"app_store" (opaque).
     */
    paymentProvider: string;
    autoRenewing: boolean;
    purchasedTsMs: number;
    revokedTsMs: number;
    redeemedTsMs: number;
    expiryTsMs: number;
    gracePeriodDurationMs: number;
    platformRefundExpiryTsMs: number;
    /**
     * Opaque payment identifier (confidential).
     */
    paymentId: string;
  };

  // NOTE: session-desktop persists this shape verbatim to local storage and reads it back cast to this
  // type (getProStatusFromStorage). The glue and desktop ship in lockstep so a single build can't
  // drift, but an older build's cache can. If you add a REQUIRED field here, the desktop read side needs
  // a transition (drop the stale cache, or treat the field as optional there) — see the reminder at
  // getProStatusFromStorage in session-desktop. Adding OPTIONAL fields is always safe.
  type GetProStatusResponse = WithProResponseHeader & {
    /**
     * Opaque account-status code slug: "never"/"active"/"expired" (unknowns pass through).
     */
    userStatus: string;
    /**
     * numeric error-report enum (0=Success,1=GenericError)
     */
    errorReport: number;
    autoRenewing: boolean;
    expiryMs: number;
    gracePeriodDurationMs: number;
    /** The single most-recent payment, or null when the account has no payments. (Full history is a
     * separate library-only query — not wired.) */
    latestPayment: ProPaymentItem | null;
  };

  type ProWrapper = {
    proFeaturesForMessage: (args: { codepointCount: number }) => WithProMessageBitset & {
      status: 'SUCCESS' | 'EXCEEDS_CHARACTER_LIMIT';
    };

    proProofRequest: (
      args: WithMasterPrivKeyHex & WithRotatingPrivKeyHex & WithUnixTsMs
    ) => ProRequest;

    /**
     * @param ticket: 64-bit monotonic revocation-list iteration. Set to 0 if unknown; otherwise use
     * the latest known `ticket` from a prior GetProRevocationsResponse so the backend may omit an
     * unchanged list.
     */
    proRevocationsRequest: (args: WithTicket) => ProRequest;

    proStatusRequest: (args: WithMasterPrivKeyHex & WithUnixTsMs) => ProRequest;

    /**
     * Parse a backend reply. The `body` is the RAW response bytes relayed from the network — the wire
     * format is a libsession<->backend contract, so the client never parses it; libsession does, and
     * returns these typed structs. (parseProProofResponse covers add-payment + generate-proof.)
     */
    parseProProofResponse: (args: { body: Uint8Array }) => GenerateProProofResponse;
    parseRevocationsResponse: (args: { body: Uint8Array }) => GetProRevocationsResponse;
    parseProStatusResponse: (args: { body: Uint8Array }) => GetProStatusResponse;

    /**
     * Support/management URLs for a provider slug, or null if none apply.
     */
    providerUrls: (args: { code: string }) => ProviderUrls | null;

    /**
     * The purchasable payment-provider slugs to surface to users (single source of truth in
     * libsession; excludes non-purchasable providers like rangeproof). Order is not significant — the
     * caller applies its own ordering and skips slugs it has no display translation for.
     */
    visiblePlatforms: () => Array<string>;
  };

  export type ProActionsCalls = MakeWrapperActionCalls<ProWrapper>;

  /**
   * To be used inside the web worker only (calls are synchronous and won't work asynchronously)
   */
  export class ProWrapperNode {
    public static proFeaturesForMessage: ProWrapper['proFeaturesForMessage'];
    public static proProofRequest: ProWrapper['proProofRequest'];
    public static proRevocationsRequest: ProWrapper['proRevocationsRequest'];
    public static proStatusRequest: ProWrapper['proStatusRequest'];
    public static parseProProofResponse: ProWrapper['parseProProofResponse'];
    public static parseRevocationsResponse: ProWrapper['parseRevocationsResponse'];
    public static parseProStatusResponse: ProWrapper['parseProStatusResponse'];
    public static providerUrls: ProWrapper['providerUrls'];
    public static visiblePlatforms: ProWrapper['visiblePlatforms'];
  }

  /**
   * Those actions are used internally for the web worker communication.
   * You should never need to import them in Session directly
   * You will need to add an entry here if you add a new function
   */
  export type ProActionsType =
    | MakeActionCall<ProWrapper, 'proFeaturesForMessage'>
    | MakeActionCall<ProWrapper, 'proProofRequest'>
    | MakeActionCall<ProWrapper, 'proRevocationsRequest'>
    | MakeActionCall<ProWrapper, 'proStatusRequest'>
    | MakeActionCall<ProWrapper, 'parseProProofResponse'>
    | MakeActionCall<ProWrapper, 'parseRevocationsResponse'>
    | MakeActionCall<ProWrapper, 'parseProStatusResponse'>
    | MakeActionCall<ProWrapper, 'providerUrls'>
    | MakeActionCall<ProWrapper, 'visiblePlatforms'>;
}
