/// <reference path="../shared.d.ts" />

/**
 *
 * Contacts wrapper logic
 *
 */
declare module 'libsession_util_nodejs' {
  type ContactsWrapper = BaseConfigWrapper & {
    init: (secretKey: Uint8Array, dump: Uint8Array | null) => void;
    /** This function is used to free wrappers from memory only */
    free: () => void;
    get: (pubkeyHex: string) => ContactInfoGet | null;
    set: (contact: ContactInfoSet) => void;
    getAll: () => Array<ContactInfoGet>;
    erase: (pubkeyHex: string) => void;
  };

  export type ContactsWrapperActionsCalls = MakeWrapperActionCalls<ContactsWrapper>;

  // NOTE must match DisappearingMessageConversationModeType in session-desktop
  export type DisappearingMessageConversationModeType =
    | 'off'
    | 'deleteAfterRead'
    | 'deleteAfterSend';

  type ContactInfoShared = WithPriority & {
    id: string;
    nickname?: string;
    /**
     * Can only be set the first time a contact is created, a new change won't override the value in the wrapper.
     */
    createdAtSeconds: number;
    expirationMode?: DisappearingMessageConversationModeType;
    expirationTimerSeconds?: number;
    /**
     * A name & profile pic change won't be applied unless this value is more recent than the currently saved one.
     */
    profileUpdatedSeconds: number;
    /**
     * see `profileUpdatedSeconds` for more info.
     */
    name?: string;
    /**
     * see `profileUpdatedSeconds` for more info.
     */
    profilePicture?: ProfilePicture;
  };

  export type ContactInfoSet = ContactInfoShared & {
    approved?: boolean;
    approvedMe?: boolean;
    blocked?: boolean;
  };

  export type ContactInfoGet = ContactInfoShared & {
    approved: boolean;
    approvedMe: boolean;
    blocked: boolean;
  };

  export class ContactsConfigWrapperNode extends BaseConfigWrapperNode {
    constructor(secretKey: Uint8Array, dump: Uint8Array | null);
    public get: ContactsWrapper['get'];
    public set: ContactsWrapper['set'];
    public getAll: ContactsWrapper['getAll'];
    public erase: ContactsWrapper['erase'];
  }

  export type ContactsConfigActionsType =
    | ['init', Uint8Array, Uint8Array | null]
    | MakeActionCall<ContactsWrapper, 'free'>
    | MakeActionCall<ContactsWrapper, 'get'>
    | MakeActionCall<ContactsWrapper, 'set'>
    | MakeActionCall<ContactsWrapper, 'getAll'>
    | MakeActionCall<ContactsWrapper, 'erase'>;
}
