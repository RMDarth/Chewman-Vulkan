package com.turbulent.chewman;

import android.content.Context;

import java.security.Key;

import javax.crypto.Cipher;
import javax.crypto.spec.SecretKeySpec;

class EncryptUtils
{
    private static byte[] reverse(byte[] data)
    {
        byte[] newData = new byte[data.length];
        for(int i=0; i<data.length; i++){
            newData[i] = data[data.length - i - 1];
        }
        return newData;
    }

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
            return reverse(data);
        }
    }

    static byte[] decrypt(Context context, byte[] data)
    {
        String key = getKey(context);

        try {
            Key aesKey = new SecretKeySpec(key.getBytes(), "AES");
            Cipher cipher = Cipher.getInstance("AES");

            cipher.init(Cipher.DECRYPT_MODE, aesKey);
            return cipher.doFinal(data);

        } catch (java.security.GeneralSecurityException ex)
        {
            return reverse(data);
        }
    }
}
