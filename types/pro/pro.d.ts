declare module 'libsession_util_nodejs' {
  // Must match session-desktop
  export enum ProOriginatingPlatform {
    Nil = "Nil",
    GooglePlayStore = "Google",
    iOSAppStore = "iOS",
  }

  export type ProBackendProviderConstantType = {
    device: string;
    store: string;
    store_other: string;
    platform: string;
    platform_account: string;
    refund_url: string;
    refund_after_platform_deadline_url: string;
    update_subscription_url: string;
    cancel_subscription_url: string;
  }

  export type ProBackendProviderConstantsType = Record<ProOriginatingPlatform, ProBackendProviderConstantType>

  export type ProBackendUrlsType = {
    roadmap: string;
    privacy_policy: string;
    terms_of_service: string;
    pro_access_not_found: string;
    support_url: string;
  }
}
