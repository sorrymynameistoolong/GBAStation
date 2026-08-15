package com.beiklive.gbastation;

import android.content.ClipData;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.provider.OpenableColumns;
import android.widget.Toast;

import org.libsdl.app.SDLActivity;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;

public class GBAStationActivity extends SDLActivity {
    private static final int REQUEST_IMPORT_ROMS = 0x4742;
    private static final int COPY_BUFFER_BYTES = 64 * 1024;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    /**
     * Called by native code when the user chooses the Android ROM import action.
     * ACTION_OPEN_DOCUMENT delegates all non-app storage access to the system
     * picker; no broad media or storage permission is requested.
     */
    public void importRomsFromSystemPicker() {
        runOnUiThread(() -> {
            Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
            intent.addCategory(Intent.CATEGORY_OPENABLE);
            intent.setType("*/*");
            intent.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, true);
            intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
            startActivityForResult(intent, REQUEST_IMPORT_ROMS);
        });
    }

    @Override
    @SuppressWarnings("deprecation")
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode != REQUEST_IMPORT_ROMS || resultCode != RESULT_OK || data == null) {
            return;
        }

        List<Uri> uris = new ArrayList<>();
        ClipData selection = data.getClipData();
        if (selection != null) {
            for (int index = 0; index < selection.getItemCount(); ++index) {
                Uri uri = selection.getItemAt(index).getUri();
                if (uri != null) {
                    uris.add(uri);
                }
            }
        } else if (data.getData() != null) {
            uris.add(data.getData());
        }

        if (uris.isEmpty()) {
            Toast.makeText(this, "未选择可导入的 ROM 文件", Toast.LENGTH_SHORT).show();
            return;
        }

        int imported = 0;
        int failed = 0;
        for (Uri uri : uris) {
            try {
                copyImportedRom(uri);
                ++imported;
            } catch (IOException | SecurityException exception) {
                ++failed;
            }
        }

        final String message;
        if (failed == 0) {
            message = "已导入 " + imported + " 个 ROM；请在游戏库中扫描 roms 目录";
        } else {
            message = "已导入 " + imported + " 个 ROM，" + failed + " 个文件导入失败";
        }
        Toast.makeText(this, message, Toast.LENGTH_LONG).show();
    }

    private void copyImportedRom(Uri uri) throws IOException {
        File destinationDirectory = getRomImportDirectory();
        if (!destinationDirectory.isDirectory() && !destinationDirectory.mkdirs()) {
            throw new IOException("Unable to create the application ROM directory");
        }

        File destination = uniqueDestination(destinationDirectory, safeDisplayName(uri));
        try (InputStream input = getContentResolver().openInputStream(uri)) {
            if (input == null) {
                throw new IOException("Unable to read selected document");
            }
            try (OutputStream output = new FileOutputStream(destination)) {
                byte[] buffer = new byte[COPY_BUFFER_BYTES];
                int count;
                while ((count = input.read(buffer)) != -1) {
                    output.write(buffer, 0, count);
                }
                output.flush();
            }
        } catch (IOException | SecurityException exception) {
            // Do not leave a partial ROM in the library if the provider stream
            // fails, permission changes, or the destination fills up.
            if (destination.exists() && !destination.delete()) {
                // A future import with the same name will receive a safe suffix.
            }
            throw exception;
        }
    }

    private File getRomImportDirectory() {
        File baseDirectory = getExternalFilesDir(null);
        if (baseDirectory == null) {
            baseDirectory = getFilesDir();
        }
        return new File(new File(baseDirectory, "GBAStation"), "roms");
    }

    private String safeDisplayName(Uri uri) {
        String name = null;
        try (Cursor cursor = getContentResolver().query(
                uri,
                new String[] {OpenableColumns.DISPLAY_NAME},
                null,
                null,
                null)) {
            if (cursor != null && cursor.moveToFirst()) {
                int index = cursor.getColumnIndex(OpenableColumns.DISPLAY_NAME);
                if (index >= 0 && !cursor.isNull(index)) {
                    name = cursor.getString(index);
                }
            }
        }

        if (name == null || name.trim().isEmpty()) {
            name = "imported-rom-" + System.currentTimeMillis();
        }
        name = new File(name).getName().replaceAll("[^A-Za-z0-9._() -]", "_");
        return name.isEmpty() || ".".equals(name) || "..".equals(name)
                ? "imported-rom-" + System.currentTimeMillis()
                : name;
    }

    private File uniqueDestination(File directory, String filename) {
        File candidate = new File(directory, filename);
        if (!candidate.exists()) {
            return candidate;
        }

        int extensionIndex = filename.lastIndexOf('.');
        String stem = extensionIndex > 0 ? filename.substring(0, extensionIndex) : filename;
        String extension = extensionIndex > 0 ? filename.substring(extensionIndex) : "";
        int suffix = 1;
        do {
            candidate = new File(directory, stem + " (" + suffix + ")" + extension);
            ++suffix;
        } while (candidate.exists());
        return candidate;
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        System.exit(0);
    }

    @Override
    protected String[] getLibraries() {
        return new String[] {
            "SDL2",
            "GBAStation"
        };
    }
}
