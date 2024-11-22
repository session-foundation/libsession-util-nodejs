/// <reference path="../shared.d.ts" />

declare module 'libsession_util_nodejs' {
  /**
   *
   * GroupMembers wrapper logic
   *
   */
  type GroupMemberShared = {
    pubkeyHex: PubkeyType;
    name: string | null;
    profilePicture: ProfilePicture | null;
  };

  type MemberStateGroupV2 =
    | 'INVITE_NOT_SENT' // as soon as we've scheduled that guy to be invited, but before we've tried sending the invite message
    | 'INVITE_FAILED'
    | 'INVITE_SENT'
    | 'INVITE_ACCEPTED' // regular member
    | 'PROMOTION_NOT_SENT' // as soon as we've scheduled that guy to be an admin, but before we've tried sending the promotion message
    | 'PROMOTION_FAILED'
    | 'PROMOTION_SENT'
    | 'PROMOTION_ACCEPTED' // regular admin
    | 'UNKNOWN';

  export type GroupMemberGet = GroupMemberShared & {
    memberStatus: MemberStateGroupV2;

    /**
     * NOT_REMOVED = 0:
     * REMOVED_MEMBER = 1,
     * REMOVED_MEMBER_AND_MESSAGES = 2;
     */
    removedStatus: 'NOT_REMOVED' | 'REMOVED_MEMBER' | 'REMOVED_MEMBER_AND_MESSAGES' | 'UNKNOWN';
    /**
     * True if the member is scheduled to get the keys (.admin field of libsession).
     * This is equivalent of memberStatus being one of:
     *  - PROMOTION_NOT_SENT
     *  - PROMOTION_FAILED
     *  - PROMOTION_SENT
     *  - PROMOTION_ACCEPTED
     */
    nominatedAdmin: boolean;
  };

  type GroupMemberWrapper = {
    // GroupMember related methods
    memberGet: (pubkeyHex: PubkeyType) => GroupMemberGet | null;
    memberGetOrConstruct: (pubkeyHex: PubkeyType) => GroupMemberGet;
    memberConstructAndSet: (pubkeyHex: PubkeyType) => void;

    memberGetAll: () => Array<GroupMemberGet>;
    memberGetAllPendingRemovals: () => Array<GroupMemberGet>;

    // setters
    memberSetNameTruncated: (pubkeyHex: PubkeyType, newName: string) => void;

    /** A member's invite state defaults to invite-not-sent. Use this function to mark that you've sent one, or at least tried (failed: boolean)*/
    memberSetInvited: (pubkeyHex: PubkeyType, failed: boolean) => void;
    /** User has accepted an invitation and is now a regular member of the group */
    memberSetAccepted: (pubkeyHex: PubkeyType) => void;

    /** Mark the member as waiting a promotion to be sent to them */
    memberSetPromoted: (pubkeyHex: PubkeyType) => void;
    /** Called when we did send the promotion to the member */
    memberSetPromotionSent: (pubkeyHex: PubkeyType) => void;
    /** Called when we did send the promotion to the member, but failed */
    memberSetPromotionFailed: (pubkeyHex: PubkeyType) => void;
    /** Called when the member accepted the promotion */
    memberSetPromotionAccepted: (pubkeyHex: PubkeyType) => void;

    memberSetProfilePicture: (pubkeyHex: PubkeyType, profilePicture: ProfilePicture) => void;
    membersMarkPendingRemoval: (members: Array<PubkeyType>, withMessages: boolean) => void;

    // eraser
    memberEraseAndRekey: (members: Array<PubkeyType>) => boolean;
  };
}
