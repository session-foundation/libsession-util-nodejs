/// <reference path="../shared.d.ts" />
/// <reference path="./groupmembers.d.ts" />
/// <reference path="./groupinfo.d.ts" />
/// <reference path="./groupkeys.d.ts" />

declare module 'libsession_util_nodejs' {
  export type ConfirmKeysPush = [data: Uint8Array, hash: string, timestampMs: number];

  export type GroupWrapperConstructor = {
    /**
     * The user's ed25519 secret key, length 64.
     */
    userEd25519Secretkey: Uint8Array;
    /**
     *  The group ed25519 pubkey without the 03 prefix, length 32.
     */
    groupEd25519Pubkey: Uint8Array;
    /**
     * The group ed25519 priv key if we have it (len 64). Having this means we have admin rights in the group.
     * This usually comes from the user group wrapper if we have it.
     */
    groupEd25519Secretkey: Uint8Array | null;
    /**
     * The unified dumps (as saved in the db) for this group. i.e. Keys, Members and Info concatenated, see MetaGroupWrapper::metaDump for details.
     */
    metaDumped: Uint8Array | null;
  };

  type MetaGroupWrapper = GroupInfoWrapper &
    GroupMemberWrapper &
    GroupKeysWrapper & {
      // shared actions
      init: (options: GroupWrapperConstructor) => void;
      free: () => void;
      needsPush: () => boolean;
      push: () => {
        groupInfo: PushConfigResult | null;
        groupMember: PushConfigResult | null;
        // groupKeys is very custom, and not an array of Uint8Array
        groupKeys: {
          data: Uint8Array;
          namespace: number;
        } | null;
      };
      /**
       * The current bytes of groupInfo and groupMember whether or not they need pushing, for
       * putting a config back on the swarm after it expired from there.
       *
       * Use `push()` for syncing — this one deliberately ignores `needsPush()`, so using it to
       * sync would send configs that haven't changed. `hashes` still carries the config's
       * obsolete hashes and is still handed over only once, so the caller must issue the delete
       * for them (expect an empty list on a read-only config — a group member — which is normal).
       *
       * groupKeys is absent because a stored keys message cannot be re-emitted at all; only an
       * admin rekey can replace one.
       */
      pushForRecovery: () => {
        groupInfo: PushConfigResult;
        groupMember: PushConfigResult;
      };
      /**
       * The same hashes as `activeHashes()`, kept separate per sub-config.
       *
       * `activeHashes()` merges all three, which is enough to bump TTLs but not to act on a
       * specific hash: a missing groupInfo/groupMember hash can be re-stored, a missing groupKeys
       * hash can only be replaced by an admin rekey.
       */
      activeHashesByConfig: () => {
        groupInfo: Array<string>;
        groupMember: Array<string>;
        groupKeys: Array<string>;
      };
      needsDump: () => boolean;
      metaDump: () => Uint8Array;
      metaMakeDump: () => Uint8Array;
      metaConfirmPushed: ({
        groupInfo,
        groupMember,
      }: {
        groupInfo: ConfirmPush | null;
        groupMember: ConfirmPush | null;
      }) => void;
      /**
       * @returns how many of the messages handed in were merged. Compare it against what you
       * passed: libSession skips a config message it cannot take and carries on without erroring,
       * so a partial merge is invisible to a caller that only checks for a throw.
       *
       * Note the keys count is optimistic — a keys message is counted even when it turns out not
       * to be encrypted to us, which is not a failure. So this detects a lossy groupInfo or
       * groupMember merge, which is what matters.
       */
      metaMerge: ({
        groupInfo,
        groupKeys,
        groupMember,
      }: {
        groupInfo: Array<MergeSingle> | null;
        groupMember: Array<MergeSingle> | null;
        groupKeys: Array<MergeSingle & { timestampMs: number }> | null;
      }) => number;
    };

  // this just adds an argument of type GroupPubkeyType in front of the parameters of that function
  type AddGroupPkToFunction<T extends (...args: any) => any> = (
    ...args: [GroupPubkeyType, ...Parameters<T>]
  ) => ReturnType<T>;

  export type MetaGroupWrapperActionsCalls = MakeWrapperActionCalls<{
    [key in keyof MetaGroupWrapper]: AddGroupPkToFunction<MetaGroupWrapper[key]>;
  }>;

  export class MetaGroupWrapperNode {
    constructor(options: GroupWrapperConstructor);

    // shared actions
    public needsPush: MetaGroupWrapper['needsPush'];
    public push: MetaGroupWrapper['push'];
    public pushForRecovery: MetaGroupWrapper['pushForRecovery'];
    public needsDump: MetaGroupWrapper['needsDump'];
    public metaDump: MetaGroupWrapper['metaDump'];
    public metaMakeDump: MetaGroupWrapper['metaMakeDump'];
    public metaConfirmPushed: MetaGroupWrapper['metaConfirmPushed'];
    public metaMerge: MetaGroupWrapper['metaMerge'];
    public activeHashes: MetaGroupWrapper['activeHashes'];
    public activeHashesByConfig: MetaGroupWrapper['activeHashesByConfig'];

    // info
    public infoGet: MetaGroupWrapper['infoGet'];
    public infoSet: MetaGroupWrapper['infoSet'];
    public infoDestroy: MetaGroupWrapper['infoDestroy'];

    // members
    public memberGet: MetaGroupWrapper['memberGet'];
    public memberGetOrConstruct: MetaGroupWrapper['memberGetOrConstruct'];
    public memberConstructAndSet: MetaGroupWrapper['memberConstructAndSet'];
    public memberGetAll: MetaGroupWrapper['memberGetAll'];
    public memberGetAllPendingRemovals: MetaGroupWrapper['memberGetAllPendingRemovals'];
    public memberSetInviteAccepted: MetaGroupWrapper['memberSetInviteAccepted'];
    public memberSetPromoted: MetaGroupWrapper['memberSetPromoted'];
    public memberSetPromotionAccepted: MetaGroupWrapper['memberSetPromotionAccepted'];
    public memberSetPromotionSent: MetaGroupWrapper['memberSetPromotionSent'];
    public memberSetPromotionFailed: MetaGroupWrapper['memberSetPromotionFailed'];
    public memberSetInviteSent: MetaGroupWrapper['memberSetInviteSent'];
    public memberSetInviteNotSent: MetaGroupWrapper['memberSetInviteNotSent'];
    public memberSetInviteFailed: MetaGroupWrapper['memberSetInviteFailed'];
    public memberEraseAndRekey: MetaGroupWrapper['memberEraseAndRekey'];
    public membersMarkPendingRemoval: MetaGroupWrapper['membersMarkPendingRemoval'];
    public memberSetProfileDetails: MetaGroupWrapper['memberSetProfileDetails'];
    public memberResetAllSendingState: MetaGroupWrapper['memberResetAllSendingState'];
    public memberSetSupplement: MetaGroupWrapper['memberSetSupplement'];

    // keys
    public keysNeedsRekey: MetaGroupWrapper['keysNeedsRekey'];
    public keyRekey: MetaGroupWrapper['keyRekey'];
    public loadKeyMessage: MetaGroupWrapper['loadKeyMessage'];
    public keysAdmin: MetaGroupWrapper['keysAdmin'];
    public keyGetCurrentGen: MetaGroupWrapper['keyGetCurrentGen'];
    public encryptMessages: MetaGroupWrapper['encryptMessages'];
    public decryptMessage: MetaGroupWrapper['decryptMessage'];
    public makeSwarmSubAccount: MetaGroupWrapper['makeSwarmSubAccount'];
    public swarmSubaccountSign: MetaGroupWrapper['swarmSubaccountSign'];
  }

  export type MetaGroupActionsType =
    | ['init', GroupWrapperConstructor]

    // shared actions
    | MakeActionCall<MetaGroupWrapper, 'needsPush'>
    | MakeActionCall<MetaGroupWrapper, 'push'>
    | MakeActionCall<MetaGroupWrapper, 'pushForRecovery'>
    | MakeActionCall<MetaGroupWrapper, 'needsDump'>
    | MakeActionCall<MetaGroupWrapper, 'metaDump'>
    | MakeActionCall<MetaGroupWrapper, 'metaMakeDump'>
    | MakeActionCall<MetaGroupWrapper, 'metaConfirmPushed'>
    | MakeActionCall<MetaGroupWrapper, 'metaMerge'>
    | MakeActionCall<MetaGroupWrapper, 'free'>

    // info actions
    | MakeActionCall<MetaGroupWrapper, 'infoGet'>
    | MakeActionCall<MetaGroupWrapper, 'infoSet'>
    | MakeActionCall<MetaGroupWrapper, 'infoDestroy'>

    // member actions
    | MakeActionCall<MetaGroupWrapper, 'memberGet'>
    | MakeActionCall<MetaGroupWrapper, 'memberGetOrConstruct'>
    | MakeActionCall<MetaGroupWrapper, 'memberConstructAndSet'>
    | MakeActionCall<MetaGroupWrapper, 'memberGetAll'>
    | MakeActionCall<MetaGroupWrapper, 'memberGetAllPendingRemovals'>
    | MakeActionCall<MetaGroupWrapper, 'memberSetInviteAccepted'>
    | MakeActionCall<MetaGroupWrapper, 'memberSetPromoted'>
    | MakeActionCall<MetaGroupWrapper, 'memberSetPromotionFailed'>
    | MakeActionCall<MetaGroupWrapper, 'memberSetPromotionSent'>
    | MakeActionCall<MetaGroupWrapper, 'memberSetPromotionAccepted'>
    | MakeActionCall<MetaGroupWrapper, 'memberSetInviteSent'>
    | MakeActionCall<MetaGroupWrapper, 'memberSetInviteNotSent'>
    | MakeActionCall<MetaGroupWrapper, 'memberSetInviteFailed'>
    | MakeActionCall<MetaGroupWrapper, 'memberEraseAndRekey'>
    | MakeActionCall<MetaGroupWrapper, 'membersMarkPendingRemoval'>
    | MakeActionCall<MetaGroupWrapper, 'memberSetProfileDetails'>
    | MakeActionCall<MetaGroupWrapper, 'memberResetAllSendingState'>
    | MakeActionCall<MetaGroupWrapper, 'memberSetSupplement'>

    // keys actions
    | MakeActionCall<MetaGroupWrapper, 'keysNeedsRekey'>
    | MakeActionCall<MetaGroupWrapper, 'keyRekey'>
    | MakeActionCall<MetaGroupWrapper, 'keyGetAll'>
    | MakeActionCall<MetaGroupWrapper, 'keyGetEncryptionKeyHex'>
    | MakeActionCall<MetaGroupWrapper, 'loadKeyMessage'>
    | MakeActionCall<MetaGroupWrapper, 'keysAdmin'>
    | MakeActionCall<MetaGroupWrapper, 'keyGetCurrentGen'>
    | MakeActionCall<MetaGroupWrapper, 'activeHashes'>
    | MakeActionCall<MetaGroupWrapper, 'activeHashesByConfig'>
    | MakeActionCall<MetaGroupWrapper, 'encryptMessages'>
    | MakeActionCall<MetaGroupWrapper, 'decryptMessage'>
    | MakeActionCall<MetaGroupWrapper, 'makeSwarmSubAccount'>
    | MakeActionCall<MetaGroupWrapper, 'swarmSubaccountSign'>
    | MakeActionCall<MetaGroupWrapper, 'generateSupplementKeys'>
    | MakeActionCall<MetaGroupWrapper, 'swarmSubAccountToken'>
    | MakeActionCall<MetaGroupWrapper, 'swarmVerifySubAccount'>
    | MakeActionCall<MetaGroupWrapper, 'loadAdminKeys'>;
}
