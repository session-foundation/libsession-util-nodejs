/// <reference path="../shared.d.ts" />

declare module 'libsession_util_nodejs' {
  /**
   *
   * User config wrapper logic
   *
   */

  type UserConfigWrapper = BaseConfigWrapper & {
    init: (secretKey: Uint8Array, dump: Uint8Array | null) => void;
    /** This function is used to free wrappers from memory only */
    free: () => void;
    getPriority: () => number;
    getName: () => string | null;
    getProfilePic: () => ProfilePicture;
    setPriority: (priority: number) => void;
    setName: (name: string) => void;
    setNameTruncated: (name: string) => void;
    /**
     *
     * Batch set user config fields.
     * Set a field to null to not change it
     * for example,
     * - set newProfilePic to null to not change the profile picture
     * - set newProfilePic to url: null, key: null to erase it
     *
     * `name` set to null means to not change it, but `name` set to `''` means to erase it
     */
    setUserConfig: (details: {
      priority: number | null;
      name: string | null;
      newProfilePic: ProfilePicture | null;
      reuploadProfilePic: ProfilePicture | null;
      enableBlindedMsgRequest: boolean | null;
      noteToSelfExpiry: number | null;
    }) => void;
    /**
     * Call this when uploading a new profile picture (i.e. not an auto reupload)
     */
    setNewProfilePic: (pic: ProfilePicture) => void;
    /**
     * Call this when reuploading a the previous profile picture
     */
    setReuploadProfilePic: (pic: ProfilePicture) => void;

    getProfileUpdatedSeconds: () => number;

    setEnableBlindedMsgRequest: (msgRequest: boolean) => void;
    getEnableBlindedMsgRequest: () => boolean | undefined;
    setNoteToSelfExpiry: (expirySeconds: number) => void;
    /**
     * @returns the expiry in seconds, 0 if off, undefined if not set
     */
    getNoteToSelfExpiry: () => number | undefined;
  };

  export type UserConfigWrapperActionsCalls = MakeWrapperActionCalls<UserConfigWrapper>;

  /**
   * To be used inside the web worker only (calls are synchronous and won't work asynchronously)
   */
  export class UserConfigWrapperNode extends BaseConfigWrapperNode {
    constructor(secretKey: Uint8Array, dump: Uint8Array | null);
    public getPriority: UserConfigWrapper['getPriority'];
    public getName: UserConfigWrapper['getName'];
    public getProfilePic: UserConfigWrapper['getProfilePic'];
    public setPriority: UserConfigWrapper['setPriority'];
    public setName: UserConfigWrapper['setName'];
    public setNameTruncated: UserConfigWrapper['setNameTruncated'];
    public setNewProfilePic: UserConfigWrapper['setNewProfilePic'];
    public setReuploadProfilePic: UserConfigWrapper['setReuploadProfilePic'];
    public setUserConfig: UserConfigWrapper['setUserConfig'];
    public getProfileUpdatedSeconds: UserConfigWrapper['getProfileUpdatedSeconds'];
    public getEnableBlindedMsgRequest: UserConfigWrapper['getEnableBlindedMsgRequest'];
    public setEnableBlindedMsgRequest: UserConfigWrapper['setEnableBlindedMsgRequest'];
    public getNoteToSelfExpiry: UserConfigWrapper['getNoteToSelfExpiry'];
    public setNoteToSelfExpiry: UserConfigWrapper['setNoteToSelfExpiry'];
  }

  /**
   * Those actions are used internally for the web worker communication.
   * You should never need to import them in Session directly
   * You will need to add an entry here if you add a new function
   */
  export type UserConfigActionsType =
    | ['init', Uint8Array, Uint8Array | null]
    | MakeActionCall<UserConfigWrapper, 'free'>
    | MakeActionCall<UserConfigWrapper, 'getPriority'>
    | MakeActionCall<UserConfigWrapper, 'getName'>
    | MakeActionCall<UserConfigWrapper, 'getProfilePic'>
    | MakeActionCall<UserConfigWrapper, 'setPriority'>
    | MakeActionCall<UserConfigWrapper, 'setName'>
    | MakeActionCall<UserConfigWrapper, 'setNameTruncated'>
    | MakeActionCall<UserConfigWrapper, 'setNewProfilePic'>
    | MakeActionCall<UserConfigWrapper, 'setReuploadProfilePic'>
    | MakeActionCall<UserConfigWrapper, 'setUserConfig'>
    | MakeActionCall<UserConfigWrapper, 'getProfileUpdatedSeconds'>
    | MakeActionCall<UserConfigWrapper, 'getEnableBlindedMsgRequest'>
    | MakeActionCall<UserConfigWrapper, 'setEnableBlindedMsgRequest'>
    | MakeActionCall<UserConfigWrapper, 'getNoteToSelfExpiry'>
    | MakeActionCall<UserConfigWrapper, 'setNoteToSelfExpiry'>;
}
