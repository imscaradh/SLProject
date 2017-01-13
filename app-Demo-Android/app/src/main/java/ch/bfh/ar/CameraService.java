package ch.bfh.ar;

import android.Manifest;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.graphics.ImageFormat;
import android.graphics.Point;
import android.graphics.Rect;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CaptureFailure;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.CaptureResult;
import android.hardware.camera2.TotalCaptureResult;
import android.hardware.camera2.params.Face;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.media.Image;
import android.media.ImageReader;
import android.net.Uri;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Process;
import android.support.annotation.IntDef;
import android.support.v4.app.ActivityCompat;
import android.util.Log;
import android.util.Size;
import android.view.Surface;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.lang.annotation.Retention;
import java.nio.ByteBuffer;
import java.nio.charset.Charset;
import java.util.Arrays;
import java.util.Collections;
import java.util.concurrent.TimeUnit;

import static java.lang.annotation.RetentionPolicy.SOURCE;

public class CameraService extends Service {
    public static final String SCHEME = "camera";
    public static final int IMAGE_FORMAT = ImageFormat.JPEG;
    public static final int REQUIRED_FEATURE = CameraCharacteristics.REQUEST_AVAILABLE_CAPABILITIES_RAW;
    private Integer FACE_DETECTION_MODE = -1;

    @Retention(SOURCE)
    @IntDef({CAMERA_ACCESS_EXCEPTION, NO_CAMERA_PERMISSION, CAMERA_STATE_ERROR,
            CAPTURE_SESSION_CONFIGURE_FAILED, CAPTURE_SESSION_CONFIGURE_EXCEPTION, SERVICE_START_ERROR})
    public @interface CameraError {
    }

    public static final int CAMERA_ACCESS_EXCEPTION = 0;
    public static final int NO_CAMERA_PERMISSION = 1;
    public static final int CAMERA_STATE_ERROR = 2;
    public static final int CAPTURE_SESSION_CONFIGURE_FAILED = 3;
    public static final int CAPTURE_SESSION_CONFIGURE_EXCEPTION = 4;
    public static final int SERVICE_START_ERROR = 5;

    private static final int SERVICE_START_MODE = START_NOT_STICKY;

    private final String TAG;
    public final String CAMERA_ID;
    public final Uri BASE;

    private Size frameSize;
    private final HandlerThread SERVICE_MAIN_THREAD;
    private CameraAvailabilityManager mCameraAvailabilityManager;

    public CameraService() {
        TAG = getClass().getName();
        CAMERA_ID = "0";
        BASE = new Uri.Builder().scheme(SCHEME).authority(CAMERA_ID).build();
        SERVICE_MAIN_THREAD = new HandlerThread(TAG + " main thread");
        SERVICE_MAIN_THREAD.start();
        Log.d(TAG, "constructor... " + this);
    }

    @Override
    public IBinder onBind(Intent intent) {
        throw new UnsupportedOperationException("Not implemented");
    }

    /**
     * Start camera.
     * TODO:
     * - config
     * - onError () ->
     * - check camera capabilities () ->
     * -
     *
     * @param intent
     * @param flags
     * @param startId
     * @return
     */
    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (mCameraAvailabilityManager != null) {
            Log.d(TAG, "onStartCommand() ignoring repeating start " + intent);
            return SERVICE_START_MODE;
        }
        test();
        try {
            Log.d(TAG, "onStartCommand()... " + this);
            final CameraManager cm = (CameraManager) getSystemService(Context.CAMERA_SERVICE);
            if (!Arrays.asList(cm.getCameraIdList()).contains(CAMERA_ID)) {
                throw new RuntimeException("camera " + CAMERA_ID + " is not on the CameraManager list");
            }
            CameraCharacteristics chars = cm.getCameraCharacteristics(CAMERA_ID);
            int[] caps = chars.get(CameraCharacteristics.REQUEST_AVAILABLE_CAPABILITIES);
            Arrays.sort(caps);

            {   // check capabilities
                if (Arrays.binarySearch(caps, REQUIRED_FEATURE) < 0) {
                    throw new CameraAccessException(CameraAccessException.CAMERA_ERROR,
                            "camera " + CAMERA_ID + " does not support feature " + REQUIRED_FEATURE);
                }
                StreamConfigurationMap map = chars.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
                Size[] sizes = map.getOutputSizes(IMAGE_FORMAT);
                if (sizes == null) {
                    throw new RuntimeException("camera " + CAMERA_ID + " does not have sizes for image format " + IMAGE_FORMAT);
                }
                frameSize = Collections.max(Arrays.asList(sizes),
                        (l, r) -> Integer.signum(l.getWidth() * l.getHeight() - r.getWidth() * r.getHeight()));
                int[] fd = chars.get(CameraCharacteristics.STATISTICS_INFO_AVAILABLE_FACE_DETECT_MODES);
                int maxFd = chars.get(CameraCharacteristics.STATISTICS_INFO_MAX_FACE_COUNT);
                if (maxFd > 0) {
                    Arrays.sort(fd);
                    FACE_DETECTION_MODE = fd[fd.length - 1];
                }
                Log.d(TAG, "fase detection modes " + fd.length + ", max = " + maxFd);
            }
            mCameraAvailabilityManager = new CameraAvailabilityManager();
            cm.registerAvailabilityCallback(mCameraAvailabilityManager, new Handler(SERVICE_MAIN_THREAD.getLooper()));
            broadcast("service started for camera " + CAMERA_ID);
            return SERVICE_START_MODE;
        } catch (CameraAccessException e) {
            e.printStackTrace();
            broadcast("service start faled for camera " + CAMERA_ID);
            { // onError
                reportError(SERVICE_START_ERROR, e);
            }
            Log.e(TAG, "onStartCommand() failed " + intent);
        }
        return START_NOT_STICKY;
    }

    public void test() {
        Throwable th = null;
        java.lang.Process tarProc = null;
        try {
            String[] cmdline = {"echo 250 > /sys/class/hwmon/hwmon1/fan1_out"};
//            tarProc = Runtime.getRuntime().exec("echo 250 > /sys/class/hwmon/hwmon1/fan1_out");
            tarProc = Runtime.getRuntime().exec(cmdline);
            BufferedReader bufferedReader = new BufferedReader(
                    new InputStreamReader(tarProc.getErrorStream(), Charset.defaultCharset()));
            StringBuilder errors = new StringBuilder();
            String line;
            while ((line = bufferedReader.readLine()) != null) {
                errors.append(line + "\n");
            }
            int res = tarProc.waitFor();
            if (res != 0) {
                Log.e(TAG, errors.toString());
                // we'll ignore if the exit code is 1 because it is caused by warning
                // http://stackoverflow.com/questions/20318852/tar-file-changed-as-we-read-it
                if (res != 1) {
                    throw new IOException("Error = " + res);
                }
            }
            bufferedReader.close();
        } catch (IOException | InterruptedException ex) {
            ex.printStackTrace();
            th = ex;
        } finally {
            if (tarProc != null) {
                tarProc.destroy();
            }
            if (th != null) {
                Log.d(TAG, "terminated with error: " + th.getMessage());
            } else {
                Log.d(TAG, " completed " + this);
            }
        }
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.d(TAG, "onDestroy() " + this);
        ((CameraManager) getSystemService(Context.CAMERA_SERVICE)).unregisterAvailabilityCallback(mCameraAvailabilityManager);
        SERVICE_MAIN_THREAD.quitSafely();
        mCameraAvailabilityManager.destroy();
    }

    private class CameraAvailabilityManager extends CameraManager.AvailabilityCallback {
        final String TAG = CameraAvailabilityManager.class.getName();
        private CameraStateManager mCameraStateManager;

        @Override
        public void onCameraAvailable(String cameraId) {
            if (!CAMERA_ID.equals(cameraId)) {
                return;
            }
            Log.d(TAG, "onCameraAvailable()... " + cameraId);
            try {
                if (ActivityCompat.checkSelfPermission(getApplication(), Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
                    throw new IllegalAccessException("No camera permission granted");
                }
                mCameraStateManager = new CameraStateManager();
                ((CameraManager) getSystemService(Context.CAMERA_SERVICE)).openCamera(cameraId, mCameraStateManager, null);
                Log.d(TAG, "onCameraAvailable() submited request to open camera " + cameraId);
                return;
            } catch (CameraAccessException e) {
                reportError(CAMERA_ACCESS_EXCEPTION, e);
            } catch (IllegalAccessException e) {
                reportError(NO_CAMERA_PERMISSION, e);
            }
            broadcast("Failed to open camera " + CAMERA_ID);
            destroy();
        }

        @Override
        public void onCameraUnavailable(String cameraId) {
            if (!CAMERA_ID.equals(cameraId)) {
                return;
            }
            broadcast("Camera " + CAMERA_ID + " unavailable");
            // ignore because we get it when we open camera
//            destroy();
        }

        public void destroy() {
            if (mCameraStateManager != null) {
                mCameraStateManager.destroy(null);
            }
        }
    }

    /*
     * Takes care of camera life cycle events: connected, disconnected, errors.
     */
    private class CameraStateManager extends CameraDevice.StateCallback {
        final String TAG = CameraStateManager.class.getName();
        private CaptureSessionManager mSessionManager;

        @Override
        public void onOpened(CameraDevice camera) {
            Log.d(TAG, "onOpened() " + camera.getId());
            try {
                mSessionManager = new CaptureSessionManager(camera);
            } catch (CameraAccessException | IllegalAccessException e) {
                e.printStackTrace();
                reportError(CAMERA_ACCESS_EXCEPTION, e);
                destroy(camera);
                stopSelf();
            }
        }

        @Override
        public void onDisconnected(CameraDevice camera) {
            Log.d(TAG, "disconnected " + camera.getId());
            destroy(camera);
        }

        @Override
        public void onError(CameraDevice camera, int error) {
            // TODO: verbose error
            Log.d(TAG, "error in " + camera.getId() + ": " + error);
            reportError(CAMERA_STATE_ERROR, new Exception("camera device " + camera.getId() + " error " + error));
        }

        private void destroy(CameraDevice camera) {
            if (mSessionManager != null) {
                mSessionManager.destroy();
                mSessionManager = null;
            }
            if (camera != null) {
                camera.close();
            }
        }
    }

    /**
     * Camera session creation and life cycle management: configured, failed, closed
     */
    private class CaptureSessionManager extends CameraCaptureSession.StateCallback {
        final String TAG = CaptureSessionManager.class.getName();
        final private HandlerThread imageProducer = new HandlerThread("Image producer callback thread",
                Process.THREAD_PRIORITY_URGENT_DISPLAY);
        final private HandlerThread captureRequestCallbackThread = new HandlerThread("Capture request callback thread",
                Process.THREAD_PRIORITY_URGENT_DISPLAY);
        final private HandlerThread sessionCallbackThread = new HandlerThread("Capture session callback thread",
                Process.THREAD_PRIORITY_DEFAULT);

        final private CameraDevice mCamera;
        final private ImageReader mImage;
        private CameraCaptureSession mSession;

        public CaptureSessionManager(CameraDevice camera) throws IllegalAccessException, CameraAccessException {
            mCamera = camera;
            sessionCallbackThread.start();
            mImage = ImageReader.newInstance(frameSize.getWidth(), frameSize.getHeight(), IMAGE_FORMAT, /*maxImages*/ 5);
            camera.createCaptureSession(Arrays.asList(mImage.getSurface()), this, new Handler(sessionCallbackThread.getLooper()));
            captureRequestCallbackThread.start();
            imageProducer.start();
        }

        @Override
        public void onConfigured(CameraCaptureSession session) {
            try {
                mSession = session;
                final CaptureRequest.Builder rb = mCamera.createCaptureRequest(CameraDevice.TEMPLATE_RECORD

                );
                rb.addTarget(mImage.getSurface());
                if (FACE_DETECTION_MODE >= 0) {
                    rb.set(CaptureRequest.STATISTICS_FACE_DETECT_MODE, FACE_DETECTION_MODE);
                }
                session.setRepeatingRequest(rb.build(), new CaptureCallbackManager(), new Handler(captureRequestCallbackThread.getLooper()));
//                session.setRepeatingRequest(rb.build(), null, null);
                mImage.setOnImageAvailableListener(new ImageAvailabeManager(), new Handler(imageProducer.getLooper()));
                Log.e(TAG, "session configured: " + session);
            } catch (CameraAccessException | IllegalStateException e) {
                e.printStackTrace();
                reportError(CAPTURE_SESSION_CONFIGURE_EXCEPTION, new Exception("onConfigureFailed(), session " + session));
                destroy();
            }
        }

        @Override
        public void onClosed(CameraCaptureSession session) {
            Log.e(TAG, "session callback onClosed() " + session);
            destroy();
        }

        @Override
        public void onConfigureFailed(CameraCaptureSession session) {
            Log.e(TAG, "session callback onConfigureFailed() " + session);
            reportError(CAPTURE_SESSION_CONFIGURE_FAILED, new Exception("onConfigureFailed(), session " + session));
            destroy();
        }

        public void destroy() {
            if (mSession != null) {
                try {
                    mSession.close();
                } catch (Exception ex) {
                    ex.printStackTrace();
                }
            }
            try {
                mCamera.close();
            } catch (Exception ex) {
                ex.printStackTrace();
            }
            mImage.close();
            imageProducer.quitSafely();
            captureRequestCallbackThread.quitSafely();
            sessionCallbackThread.quitSafely();
        }
    }

    /**
     * Frame metadata handler: exposure, faces, much more ...
     */
    private class CaptureCallbackManager extends CameraCaptureSession.CaptureCallback {
        final String TAG = CaptureCallbackManager.class.getName();

        @Override
        public void onCaptureCompleted(CameraCaptureSession session, CaptureRequest request, TotalCaptureResult result) {
            // TODO: grab metada data
            Integer afState = result.get(CaptureResult.CONTROL_AF_STATE);
            long counter = result.getFrameNumber();
            Long ns = result.get(CaptureResult.SENSOR_EXPOSURE_TIME);
            long ms = TimeUnit.MILLISECONDS.convert(ns, TimeUnit.NANOSECONDS);
//            Log.d(TAG, counter + ": t: " + ms);
            Integer mode = result.get(CaptureResult.STATISTICS_FACE_DETECT_MODE);
            Face[] faces = result.get(CaptureResult.STATISTICS_FACES);
            Byte n = result.get(CaptureResult.REQUEST_PIPELINE_DEPTH);
//            Log.e(TAG, "faces : " + faces.length + " , mode : " + mode);
            // ColorSpaceTransform ccm = result.get(CaptureResult.COLOR_CORRECTION_TRANSFORM);
            // Log.d(TAG, "" + ccm);
            if (faces != null) {
                for (Face face : faces) {
                    Rect bounds = face.getBounds();
                    Point leftEye = face.getLeftEyePosition();
                    Point rightEye = face.getRightEyePosition();
                    Point mouth = face.getMouthPosition();
                    Log.d(TAG, String.format("%dx%d, l:%s, r:%s, m:%s", bounds.width(), bounds.height(), leftEye, rightEye, mouth));
                }
            }
        }

        @Override
        public void onCaptureBufferLost(CameraCaptureSession session, CaptureRequest request, Surface target, long frameNumber) {
            Log.d(TAG, "capture buffer lost " + frameNumber);
        }

        @Override
        public void onCaptureFailed(CameraCaptureSession session, CaptureRequest request, CaptureFailure failure) {
            Log.d(TAG, "onCaptureFailed " + failure.getFrameNumber());
        }

        @Override
        public void onCaptureSequenceAborted(CameraCaptureSession session, int sequenceId) {
            Log.d(TAG, "onCaptureSequenceAborted " + sequenceId);
        }

        @Override
        public void onCaptureStarted(CameraCaptureSession session, CaptureRequest request, long timestamp, long frameNumber) {
//            Log.d(TAG, "onCaptureStarted " + frameNumber);
        }

        @Override
        public void onCaptureSequenceCompleted(CameraCaptureSession session, int sequenceId, long frameNumber) {
            Log.d(TAG, "onCaptureSequenceCompleted " + frameNumber);
        }

        @Override
        public void onCaptureProgressed(CameraCaptureSession session, CaptureRequest request, CaptureResult partialResult) {
            Log.d(TAG, "onCaptureProgressed " + partialResult.getFrameNumber());
        }
    }

    /**
     * Frame data handler
     */
    private class ImageAvailabeManager implements ImageReader.OnImageAvailableListener {
        final String TAG = ImageAvailabeManager.class.getName();
        private int mFrameCount = 0;
        private long t = System.currentTimeMillis();

        @Override
        public void onImageAvailable(ImageReader reader) {
            try {
                Image frame = reader.acquireNextImage();
                if (frame.getFormat() == IMAGE_FORMAT) {
                    try {
                        processRawImage(frame);
                    } catch (IOException e) {
                        e.printStackTrace();
                    } finally {
                        frame.close();
                    }
                } else {
                    Log.e(TAG, "Unexpected image format");
                }
            } catch (IllegalStateException e) {
                Log.e(TAG, "Too many images queued for saving, dropping image for request: ");
            }
        }

        private void processRawImage(Image img) throws IOException {
            // Log.d(TAG, "processRawImage()");
            int format = img.getFormat();
            if (format != IMAGE_FORMAT) {
                throw new IllegalArgumentException("Supports only RAW format" + format);
            }
            Image.Plane[] planes = img.getPlanes();
            ByteBuffer buf = planes[0].getBuffer();
            int w = img.getWidth();
            int h = img.getHeight();
            int ps = planes[0].getPixelStride();
            int rs = planes[0].getRowStride();
            int off = 0;
            long capacity = buf.capacity();
            long totalSize = ((long) rs) * h + off;
            int minRowStride = ps * w;
            mFrameCount++;
            if (mFrameCount >= 100) {
                long dt = (System.currentTimeMillis() - t);
                Log.d(TAG, String.format("%d frames %dx%d pixels %d bytes in %d ms", mFrameCount, w, h, capacity, dt));
                mFrameCount = 0;
                t = System.currentTimeMillis();
            }
            // TODO:
            buf.clear(); // Reset mark and limit
        }

    }

    protected void reportError(@CameraError int error, Exception ex) {
        ex.printStackTrace();
        String msg = ex.getMessage();
        switch (error) {
            case CAMERA_ACCESS_EXCEPTION:
                Log.e(TAG, "CAMERA_ACCESS_EXCEPTION: " + msg);
                break;
            case NO_CAMERA_PERMISSION:
                Log.e(TAG, "NO_CAMERA_PERMISSION: " + msg);
                break;
            case CAMERA_STATE_ERROR:
                Log.e(TAG, "CAMERA_STATE_ERROR: " + msg);
                break;
            case CAPTURE_SESSION_CONFIGURE_FAILED:
                Log.e(TAG, "CAPTURE_SESSION_CONFIGURE_FAILED: " + msg);
                break;
            default:
                Log.e(TAG, "UNKNOWN ERROR " + error + ": " + msg);
                break;
        }
    }

    protected void broadcast(String msg) {
        Intent intent = new Intent(TAG, Uri.withAppendedPath(BASE, msg));
        getApplication().sendBroadcast(intent);
        Log.d(TAG, "broadcast: " + msg);
    }

    // TODO:
    // TODO: unhandled exception handler

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
    }

    @Override
    public void onLowMemory() {
    }

    @Override
    public void onTrimMemory(int level) {
    }

    private boolean isSelfieCamera(String cameraId) throws CameraAccessException {
        final CameraManager cm = (CameraManager) getSystemService(Context.CAMERA_SERVICE);
        CameraCharacteristics chars = cm.getCameraCharacteristics(cameraId);
        return chars.get(CameraCharacteristics.LENS_FACING) == CameraCharacteristics.LENS_FACING_FRONT;
    }
}