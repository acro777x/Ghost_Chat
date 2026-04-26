package com.ghost.app

import android.util.Base64
import javax.crypto.Cipher
import javax.crypto.SecretKeyFactory
import javax.crypto.spec.GCMParameterSpec
import javax.crypto.spec.PBEKeySpec
import javax.crypto.spec.SecretKeySpec
import java.security.SecureRandom

/**
 * Ghost Protocol Cryptography Engine — Military-Grade
 *
 * Two encryption modes:
 * 1. Wire-format XOR+Base64 (legacy ESP32 compatibility)
 * 2. AES-256-GCM with PBKDF2 key derivation (E2E encryption)
 *
 * The ESP32 is a BLIND ROUTER. It only sees AES-encrypted Base64 blobs.
 */
object GhostCrypto {

    private const val SALT = "ghost_salt_v5"
    const val MSG_PREFIX = "GHO:"

    // AES-256-GCM constants
    private const val AES_KEY_SIZE = 256
    private const val GCM_IV_LENGTH = 12
    private const val GCM_TAG_LENGTH = 128
    private const val PBKDF2_ITERATIONS = 100_000
    private const val PBKDF2_ALGO = "PBKDF2WithHmacSHA256"

    // ── Room Hash (ESP32 Login — unchanged) ──────────────────────────

    fun genHash(pwd: String): String {
        val s = pwd + SALT
        var h1 = 0xdeadbeef.toInt()
        var h2 = 0x41c6ce57
        for (ch in s) {
            val c = ch.code
            h1 = imul(h1 xor c, 0x9e3779b1.toInt())
            h2 = imul(h2 xor c, 0x5f356495)
        }
        h1 = imul(h1 xor (h1 ushr 16), 0x85ebca6b.toInt()) xor imul(h2 xor (h2 ushr 13), 0xc2b2ae35.toInt())
        h2 = imul(h2 xor (h2 ushr 16), 0x85ebca6b.toInt()) xor imul(h1 xor (h1 ushr 13), 0xc2b2ae35.toInt())
        val hex1 = (h1.toLong() and 0xFFFFFFFFL).toString(16).padStart(8, '0')
        val hex2 = (h2.toLong() and 0xFFFFFFFFL).toString(16).padStart(8, '0')
        return (hex1 + hex2).substring(0, 8)
    }

    private fun imul(a: Int, b: Int): Int {
        val ah = (a ushr 16) and 0xffff
        val al = a and 0xffff
        val bh = (b ushr 16) and 0xffff
        val bl = b and 0xffff
        return (al * bl) + (((ah * bl + al * bh) shl 16) or 0)
    }

    // ── AES-256-GCM End-to-End Encryption ────────────────────────────

    /** Derive a 256-bit AES key from the room password using PBKDF2 */
    fun deriveKey(password: String): SecretKeySpec {
        val salt = (password + SALT).toByteArray(Charsets.UTF_8)
        val spec = PBEKeySpec(password.toCharArray(), salt, PBKDF2_ITERATIONS, AES_KEY_SIZE)
        val factory = SecretKeyFactory.getInstance(PBKDF2_ALGO)
        val keyBytes = factory.generateSecret(spec).encoded
        return SecretKeySpec(keyBytes, "AES")
    }

    /** AES-256-GCM encrypt → Base64 encoded string (IV prepended) */
    fun encryptAES(plaintext: String, key: SecretKeySpec): String {
        val iv = ByteArray(GCM_IV_LENGTH).also { SecureRandom().nextBytes(it) }
        val cipher = Cipher.getInstance("AES/GCM/NoPadding")
        cipher.init(Cipher.ENCRYPT_MODE, key, GCMParameterSpec(GCM_TAG_LENGTH, iv))
        val encrypted = cipher.doFinal(plaintext.toByteArray(Charsets.UTF_8))
        val combined = iv + encrypted // IV || ciphertext+tag
        return Base64.encodeToString(combined, Base64.NO_WRAP)
    }

    /** Base64 decode → AES-256-GCM decrypt (IV extracted from prefix) */
    fun decryptAES(encoded: String, key: SecretKeySpec): String {
        return try {
            val combined = Base64.decode(encoded, Base64.DEFAULT)
            val iv = combined.copyOfRange(0, GCM_IV_LENGTH)
            val ciphertext = combined.copyOfRange(GCM_IV_LENGTH, combined.size)
            val cipher = Cipher.getInstance("AES/GCM/NoPadding")
            cipher.init(Cipher.DECRYPT_MODE, key, GCMParameterSpec(GCM_TAG_LENGTH, iv))
            String(cipher.doFinal(ciphertext), Charsets.UTF_8)
        } catch (e: Exception) {
            "⚠ Decryption failed"
        }
    }

    // ── Utility ──────────────────────────────────────────────────────

    /** Generate a deterministic avatar color from username */
    fun avatarColor(name: String): Int {
        var h = 0
        for (c in name) {
            h = ((h shl 5) - h) + c.code
            h = h or 0
        }
        // Ensure vibrant colors by keeping saturation high
        val hue = (h and 0x7FFFFFFF) % 360
        return android.graphics.Color.HSVToColor(floatArrayOf(hue.toFloat(), 0.7f, 0.9f))
    }

    /** Get current time as HH:MM */
    fun timeNow(): String {
        val c = java.util.Calendar.getInstance()
        return "%02d:%02d".format(c.get(java.util.Calendar.HOUR_OF_DAY), c.get(java.util.Calendar.MINUTE))
    }

    /** Build a Ghost protocol message string */
    fun buildMsg(sender: String, content: String): String {
        return "$MSG_PREFIX|$sender|${timeNow()}|$content"
    }

    /** Build a system message */
    fun buildSysMsg(content: String): String {
        return "$MSG_PREFIX|SYS|${timeNow()}|$content"
    }
}
