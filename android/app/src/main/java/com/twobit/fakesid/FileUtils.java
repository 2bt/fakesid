package com.twobit.fakesid;

import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.OpenableColumns;
import android.util.Log;

import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class FileUtils {
    private static final String TAG = "FileUtils";

    static public void copyFileToUri(Context context, String path, Uri uri) {
        try {
            InputStream inputStream = new FileInputStream(path);
            OutputStream outputStream = context.getContentResolver().openOutputStream(uri);
            if (outputStream == null) {
                Log.e(TAG, "copyFileToUri: failed to open output stream for URI " + uri);
                return;
            }
            byte[] buffer = new byte[4096];
            int bytesRead;
            while ((bytesRead = inputStream.read(buffer)) != -1) {
                outputStream.write(buffer, 0, bytesRead);
            }
            inputStream.close();
            outputStream.close();
        } catch (IOException e) {
            Log.e(TAG, "copyFileToUri: " + e.getMessage());
        }
    }
    static public void copyUriToFile(Context context, Uri uri, String path) {
        try {
            InputStream inputStream = context.getContentResolver().openInputStream(uri);
            if (inputStream == null) {
                Log.e(TAG, "copyUriToFile: failed to get input stream for URI " + uri);
                return;
            }
            OutputStream outputStream = new FileOutputStream(path);
            byte[] buffer = new byte[4096];
            int bytesRead;
            while ((bytesRead = inputStream.read(buffer)) != -1) {
                outputStream.write(buffer, 0, bytesRead);
            }
            inputStream.close();
            outputStream.close();
        } catch (IOException e) {
            Log.e(TAG, "copyUriToFile: " + e.getMessage());
        }
    }


    public static String getFileName(Context context, Uri uri) {
        // Query the ContentResolver for DISPLAY_NAME
        Cursor cursor = context.getContentResolver().query(uri, null, null, null, null);
        if (cursor != null) {
            try {
                if (cursor.moveToFirst()) {
                    int nameIndex = cursor.getColumnIndex(OpenableColumns.DISPLAY_NAME);
                    if (nameIndex != -1) {
                        return cursor.getString(nameIndex);
                    }
                }
            } finally {
                cursor.close();
            }
        }
        // extract from the Uri path as a fallback
        String uriPath = uri.getPath();
        if (uriPath != null) {
            int lastSlashIndex = uriPath.lastIndexOf('/');
            if (lastSlashIndex != -1) {
                return uriPath.substring(lastSlashIndex + 1);
            }
        }
        return null;
    }

}
