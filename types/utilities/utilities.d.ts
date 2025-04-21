/// <reference path="../shared.d.ts" />

declare module 'libsession_util_nodejs' {
  type UtilitiesWrapper = {
    freeAllWrappers: () => void;
  };

  /**
   * To be used inside the web worker only (calls are synchronous and won't work asynchronously)
   */
  export class UtilitiesWrapperNode {
    public static freeAllWrappers: UtilitiesWrapper['freeAllWrappers'];
  }

  export type UtilitiesWrapperActionsCalls = MakeWrapperActionCalls<UtilitiesWrapper>;

  /**
   * Those actions are used internally for the web worker communication.
   * You should never need to import them in Session directly
   * You will need to add an entry here if you add a new function
   */
  export type UtilitiesActionsType = MakeActionCall<UtilitiesWrapper, 'freeAllWrappers'>;
}
