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

  /**
   * Unified status that a member can have based on the various libsession-util values.
   *
   * **Invite statuses**
   * - INVITE_UNKNOWN: fallback invite case
   * - INVITE_SENDING : when we are actively sending the invite (not synced)
   * - INVITE_NOT_SENT: as soon as we've scheduled that member to be invited, but before we've tried sending the invite message
   * - INVITE_FAILED: we know the invite failed to be sent to the member
   * - INVITE_SENT: we know the invite has been sent to the member
   * - INVITE_ACCEPTED: regular member
   *
   * **Promotion statuses**
   * - PROMOTION_UNKNOWN: promotion fallback case
   * - PROMOTION_SENDING : when we are actively sending the promotion (not synced)
   * - PROMOTION_NOT_SENT: as soon as we've scheduled that guy to be an admin, but before we've tried sending the promotion message
   * - PROMOTION_FAILED: we know the promotion failed to be sent to the member
   * - PROMOTION_SENT: we know the promotion message was sent to the member
   * - PROMOTION_ACCEPTED: regular admin
   *
   * **Removed statuses**
   * - REMOVED_MEMBER: the member is pending removal from the group
   * - REMOVED_MEMBER_AND_MESSAGES: the member and his messages are pending removal from the group
   * - REMOVED_UNKNOWN: the member is pending removal, fallback case;
   */
  type MemberStateGroupV2 =
    | 'INVITE_UNKNOWN'
    | 'INVITE_SENDING'
    | 'INVITE_NOT_SENT'
    | 'INVITE_FAILED'
    | 'INVITE_SENT'
    | 'INVITE_ACCEPTED'
    | 'PROMOTION_UNKNOWN'
    | 'PROMOTION_SENDING'
    | 'PROMOTION_NOT_SENT'
    | 'PROMOTION_FAILED'
    | 'PROMOTION_SENT'
    | 'PROMOTION_ACCEPTED'
    | 'REMOVED_MEMBER'
    | 'REMOVED_MEMBER_AND_MESSAGES'
    | 'REMOVED_UNKNOWN';

  export type GroupMemberGet = GroupMemberShared & {
    memberStatus: MemberStateGroupV2;
    /**
     * True if the member was invited with a supplemental key (i.e. sharing message history).
     * On invite resend, we should check this field to know if we should again, generate a supplemental key for that user.
     */
    supplement: boolean;

    profileUpdatedSeconds: number;

    /**
     * True if the member is scheduled to get the keys (`.admin` field of libsession).
     * This is equivalent of memberStatus being one of:
     *  - PROMOTION_UNKNOWN
     *  - PROMOTION_NOT_SENT
     *  - PROMOTION_FAILED
     *  - PROMOTION_SENT
     *  - PROMOTION_ACCEPTED
     *
     * We display the "crown" on top of the member's avatar when this field is true
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

    /**
     * A member's invite state defaults to invite-not-sent.
     * Use this function to mark that you've failed to send one successfully.
     **/
    memberSetInviteFailed: (pubkeyHex: PubkeyType) => void;
    /**
     * A member's invite state defaults to invite-not-sent.
     * Use this function to mark that you've sent one successfully.
     **/
    memberSetInviteSent: (pubkeyHex: PubkeyType) => void;

    /**
     * Reset a member's invite state to not_sent. This will mark it as "sending" on the current device
     * and "not sent" on any others.
     */
    memberSetInviteNotSent: (pubkeyHex: PubkeyType) => void;
    /** User has accepted an invitation and is now a regular member of the group */
    memberSetInviteAccepted: (pubkeyHex: PubkeyType) => void;

    /** Mark the member as waiting a promotion to be sent to them */
    memberSetPromoted: (pubkeyHex: PubkeyType) => void;
    /** Called when we did send the promotion to the member */
    memberSetPromotionSent: (pubkeyHex: PubkeyType) => void;
    /** Called when we did send the promotion to the member, but failed */
    memberSetPromotionFailed: (pubkeyHex: PubkeyType) => void;
    /** Called when the member accepted the promotion */
    memberSetPromotionAccepted: (pubkeyHex: PubkeyType) => void;

    memberSetProfileDetails: (
      pubkeyHex: PubkeyType,
      profileDetails: {
        profileUpdatedSeconds: number;
        name: string;
        profilePicture: ProfilePicture;
      }
    ) => void;
    memberResetAllSendingState: () => boolean;
    memberSetSupplement: (pubkeyHex: PubkeyType) => void;
    membersMarkPendingRemoval: (members: Array<PubkeyType>, withMessages: boolean) => void;

    // eraser
    memberEraseAndRekey: (members: Array<PubkeyType>) => boolean;
  };
}
