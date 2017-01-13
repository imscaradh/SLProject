package ch.bfh.ar;

import android.app.Application;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLContext;

public class GLES3Lib {

    static {
        System.loadLibrary("native-lib");
    }

    public static Application App = null;
    public static String FilesPath = null;
    public static GLES3View view;
    public static int dpi;
    public static boolean RTIsRunning = false;

    public static native void onInit(int width, int height, int dotsPerInch, String FilePath);

    public static native boolean onUpdateAndPaint();

    public static native void onResize(int width, int height);

    public static native void onMenuButton();

    public static native boolean onMouseDown(int button, int x, int y);

    public static native boolean onMouseUp(int button, int x, int y);

    public static native boolean onMouseMove(int x, int y);

    //public static native boolean onMouseWheel(int pos);

    public static native boolean onTouch2Down(int x1, int y1, int x2, int y2);

    public static native boolean onTouch2Up(int x1, int y1, int x2, int y2);

    public static native boolean onTouch2Move(int x1, int y1, int x2, int y2);

    public static native boolean onDoubleClick(int button, int x, int y);

    public static native void onRotationPYR(float pitchRAD, float yawRAD, float rollRAD);

    public static native void onRotationQUAT(float quatX, float quatY, float quatZ, float quatW);

    //public static native void onStopRT();

    public static native void onClose();

    public static native boolean shouldClose();

    public static native boolean usesRotation();

    public static native boolean usesVideoImage();

    /**
     * The RaytracingCallback function is used to repaint the ray tracing image during the
     * ray tracing process. Only the GUI bound OpenGL context can call the swap the buffer
     * for the OpenGL display. This is an example for a native C++ callback into managed
     * Java. See also the Java_renderRaytracingCallback in SLInterface that calls this
     * function.
     */
    public static boolean RaytracingCallback() {
        // calls the OpenGL rendering to display the RT image on a simple rectangle
        boolean stopSignal = GLES3Lib.onUpdateAndPaint();

        // Do the OpenGL back to front buffer swap
        EGL10 mEgl = (EGL10) EGLContext.getEGL();
        mEgl.eglSwapBuffers(mEgl.eglGetCurrentDisplay(), mEgl.eglGetCurrentSurface(EGL10.EGL_READ));
        return RTIsRunning;
    }

    /**
     * Extracts the relevant folders from the assets in our private storage on the device
     * internal storage. We extract all files from the folders texutures, models & shaders
     * into the corresponding folders. This has to be done because most files in the apk/assets
     * folder are compressed and can not be read with standard C-file IO.
     */
    public static void extractAPK() throws IOException {
        FilesPath = App.getApplicationContext().getFilesDir().getAbsolutePath();
        Log.i("SLProject", "Destination: " + FilesPath);
        extractAPKFolder(FilesPath, "textures");
        extractAPKFolder(FilesPath, "models");
        extractAPKFolder(FilesPath, "shaders");
        extractAPKFolder(FilesPath, "calibrations");
        extractAPKFolder(FilesPath, "config");
    }

    /**
     * Extract a folder inside the APK File. If we have a subfolder we just skip it.
     * If a file already exists we skip it => we don't update it!
     *
     * @param FilesPath where we want to store the path. Usually this is some writable path on the device. No / at the end
     * @param AssetPath path inside the asset folder. No leading or closing /
     * @throws IOException
     */
    public static void extractAPKFolder(String FilesPath, String AssetPath) throws IOException {
        String[] files = App.getAssets().list(AssetPath);

        for (String file : files) {
            if (!file.contains("."))
                continue;

            File f = new File(FilesPath + "/" + AssetPath + "/" + file);
            if (f.exists())
                continue;

            createDir(FilesPath + "/" + AssetPath);
            copyFile(App.getAssets().open(AssetPath + "/" + file), new FileOutputStream(FilesPath + "/" + AssetPath + "/" + file));
            Log.i("SLProject", "File: " + FilesPath + "/" + AssetPath + "/" + file + "\r\n");
        }
    }

    /**
     * Create the full directory tree up to our path
     *
     * @param Path to create
     * @throws IOException
     */
    public static void createDir(String Path) throws IOException {
        File directory = new File(Path);
        if (directory.exists())
            return;
        if (!directory.mkdirs())
            throw new IOException("Directory couldn't be created: " + Path);
    }

    /**
     * Copy from an Input to an Output stream. We use a buffer of 1024 and close both streams at the end
     *
     * @param is
     * @param os
     * @throws IOException
     */
    public static void copyFile(InputStream is, OutputStream os) throws IOException {
        byte[] buffer = new byte[1024];

        int length;
        while ((length = is.read(buffer)) > 0) {
            os.write(buffer, 0, length);
        }
        os.flush();
        os.close();
        is.close();
    }
}
