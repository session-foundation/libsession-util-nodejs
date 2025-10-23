/// <reference path="../shared.d.ts" />

declare module 'libsession_util_nodejs' {
  type WithProRotatingEd25519PrivKey = { proRotatingEd25519PrivKey: string | null };

  type WithProBackendPubkey = {
    /**
     * HexString
     */
    proBackendPubkeyHex: string;
  };

  type ProStatus = 'InvalidProBackendSig' | 'InvalidUserSig' | 'Valid' | 'Expired';
  type ProFeature = '10K_CHARACTER_LIMIT' | 'PRO_BADGE' | 'ANIMATED_AVATAR';
  type ProFeatures = Array<ProFeature>;
  type WithProFeatures = { proFeatures: ProFeatures };

  type ProProof = {
    version: number;
    genIndexHashB64: string;
    rotatingPubkeyHex: string;
    expiryMs: number;
  };

  type ProConfig = {
    /**
     * 64 bytes, 128 chars
     */
    rotatingPrivKeyHex: string;
    proProof: ProProof;
  };
}
