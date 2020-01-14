package com.turbulent.chewman;

import android.content.Context;
import android.os.Build;
import android.provider.Settings;

import com.google.android.gms.common.util.ArrayUtils;

import java.nio.ByteBuffer;
import java.security.Key;
import java.util.Arrays;

import javax.crypto.Cipher;
import javax.crypto.spec.SecretKeySpec;

class EncryptUtils
{
    private static String getKey(Context context)
    {
        return "DummyKey";
    }

    static byte[] encrypt(Context context, byte[] data)
    {
        String key = getKey(context);

        try {
            Key aesKey = new SecretKeySpec(key.getBytes(), "AES");
            Cipher cipher = Cipher.getInstance("AES");

            cipher.init(Cipher.ENCRYPT_MODE, aesKey);
            return cipher.doFinal(data);
        } catch (java.security.GeneralSecurityException ex)
        {
            return ArrayUtils.reverse(byteArr);
        }
    }

    static byte[] decrypt(Context context, byte[] data)
    {
        String key = getKey(context);

        byte[] decryptedData;

        try {
            Key aesKey = new SecretKeySpec(key.getBytes(), "AES");
            Cipher cipher = Cipher.getInstance("AES");

            cipher.init(Cipher.DECRYPT_MODE, aesKey);
            return cipher.doFinal(data);

        } catch (java.security.GeneralSecurityException ex)
        {
            return ArrayUtils.reverse(byteArr);
        }
    }
}
