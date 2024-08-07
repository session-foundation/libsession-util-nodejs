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

  export type GroupMemberGet = GroupMemberShared & {
    /**  Default state, before we try sending the invite */
    inviteNotSent: boolean;
    /** We did send the invite to the user */
    invitePending: boolean;
    /** The invite was accepted by the user */
    inviteAccepted: boolean;

    /** We failed to send the invite to the user */
    inviteFailed: boolean;

    /** Default state, before we try sending the promotion */
    promotionNotSent: boolean;
    /** We did send the promotion, not accepted yet */
    promotionPending: boolean;
    /** We tried to send the promotion but failed */
    promotionFailed: boolean;
    /** The user is already an admin *or* has a pending promotion */
    promoted: boolean;

    /** True if the user should be removed from the group */
    isRemoved: boolean;
    /** True if the user and his messages should be removed from the group */
    shouldRemoveMessages: boolean;
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

    /** A member invite states defaults to invite-not-sent. Use this function to mark that you've sent one, or at least tried (failed: boolean)*/
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
