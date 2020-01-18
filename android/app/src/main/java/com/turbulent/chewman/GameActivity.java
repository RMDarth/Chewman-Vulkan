package com.turbulent.chewman;

import android.app.ActivityManager;
import android.app.AlarmManager;
import android.app.AlertDialog;
import android.app.PendingIntent;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.ActivityInfo;
import android.content.pm.ApplicationInfo;
import android.content.res.AssetFileDescriptor;
import android.media.AudioAttributes;
import android.media.MediaPlayer;
import android.media.SoundPool;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;
import android.provider.Settings;
import android.util.Log;
import android.widget.RelativeLayout;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.android.billingclient.api.AcknowledgePurchaseParams;
import com.android.billingclient.api.AcknowledgePurchaseResponseListener;
import com.android.billingclient.api.BillingClient;
import com.android.billingclient.api.BillingClientStateListener;
import com.android.billingclient.api.BillingFlowParams;
import com.android.billingclient.api.BillingResult;
import com.android.billingclient.api.ConsumeParams;
import com.android.billingclient.api.ConsumeResponseListener;
import com.android.billingclient.api.Purchase;
import com.android.billingclient.api.PurchaseHistoryRecord;
import com.android.billingclient.api.PurchaseHistoryResponseListener;
import com.android.billingclient.api.PurchasesUpdatedListener;
import com.android.billingclient.api.RewardLoadParams;
import com.android.billingclient.api.RewardResponseListener;
import com.android.billingclient.api.SkuDetails;
import com.android.billingclient.api.SkuDetailsParams;
import com.android.billingclient.api.SkuDetailsResponseListener;
import com.google.android.gms.ads.AdRequest;
import com.google.android.gms.ads.AdSize;
import com.google.android.gms.ads.AdView;
import com.google.android.gms.ads.MobileAds;
import com.google.android.gms.ads.initialization.InitializationStatus;
import com.google.android.gms.ads.initialization.OnInitializationCompleteListener;
import com.google.android.gms.auth.api.signin.GoogleSignIn;
import com.google.android.gms.auth.api.signin.GoogleSignInAccount;
import com.google.android.gms.auth.api.signin.GoogleSignInClient;
import com.google.android.gms.auth.api.signin.GoogleSignInOptions;
import com.google.android.gms.common.ConnectionResult;
import com.google.android.gms.common.GoogleApiAvailability;
import com.google.android.gms.common.api.ApiException;
import com.google.android.gms.games.AchievementsClient;
import com.google.android.gms.games.AnnotatedData;
import com.google.android.gms.games.Games;
import com.google.android.gms.games.GamesClient;
import com.google.android.gms.games.LeaderboardsClient;
import com.google.android.gms.games.Player;
import com.google.android.gms.games.leaderboard.LeaderboardScore;
import com.google.android.gms.games.leaderboard.LeaderboardScoreBuffer;
import com.google.android.gms.games.leaderboard.LeaderboardVariant;
import com.google.android.gms.games.leaderboard.ScoreSubmissionData;
import com.google.android.gms.tasks.OnCompleteListener;
import com.google.android.gms.tasks.OnSuccessListener;
import com.google.android.gms.tasks.Task;

import org.libsdl.app.SDLActivity;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Semaphore;

import static com.google.android.gms.games.leaderboard.LeaderboardVariant.COLLECTION_PUBLIC;
import static com.google.android.gms.games.leaderboard.LeaderboardVariant.TIME_SPAN_ALL_TIME;
import static com.google.android.gms.games.leaderboard.LeaderboardVariant.TIME_SPAN_WEEKLY;

public class GameActivity extends org.libsdl.app.SDLActivity {
    private static final String TAG = "Chewman";
    private final Semaphore mSemaphore = new Semaphore(0, true);

    // request codes we use when invoking an external activity
    private static final int RC_UNUSED = 5001;
    private static final int RC_SIGN_IN = 9001;
    private static final int RC_ACHIEVEMENT_UI = 9003;
    private static final int RC_LEADERBOARD_UI = 9004;

    private SharedPreferences mSharedPreferences;

    private SoundPool mSoundPool;
    private MediaPlayer mMusic;

    // Client used to sign in with Google APIs
    private GoogleSignInClient mGoogleSignInClient;

    // Client variables
    private AchievementsClient mAchievementsClient;
    private LeaderboardsClient mLeaderboardsClient;

    // Billing
    private BillingClient mBillingClient;
    private boolean mBillingConnected;
    private SkuDetails mCampaign3Product;
    private boolean mCampaign3ProductBought = false;
    private String mCampaignProductName = "levels";
    private SkuDetails mRewardProduct;
    private String mRewardProductName = "android.test.reward";
    private boolean mVideoProductAvailable = false;
    private int mVideosWatched = 0;

    // Scores
    class ScoreInfo
    {
        String name;
        long score;
    }

    private boolean mRefreshScores = false;

    private String mPlayerDisplayName = "Player";
    private boolean[] mScoresUpdated = { false, false };
    private ScoreInfo[] mWeeklyScores = new ScoreInfo[6];
    private int mWeeklyScoresSize = 0;
    private ScoreInfo[] mFullScores = new ScoreInfo[6];
    private int mFullScoresSize = 0;
    private long[] mPlayerPlace = {-2, -2};


    private boolean[] mScoreSubmitted = new boolean[37];

    private boolean[] mTimeScoresUpdatedFull = new boolean[36];
    private boolean[] mTimeScoresUpdatedWeekly = new boolean[36];
    private boolean mTimeScoreNewChanges = false;
    private ScoreInfo[] mWeeklyTimeScores = new ScoreInfo[36];
    private ScoreInfo[] mFullTimeScores = new ScoreInfo[36];

    // ads
    private AdView mAdView = null;
    private boolean mAdsInitialized = false;
    private boolean mAdsHidden = true;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        this.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        super.onCreate(savedInstanceState);
        Log.d(TAG, "onCreate()");

        mSharedPreferences = getSharedPreferences("settings", Context.MODE_PRIVATE);
        mPlayerDisplayName = mSharedPreferences.getString("playerName", "Player");

        for (int i = 0; i < 37; i++)
        {
            mScoreSubmitted[i] = mSharedPreferences.getBoolean("isSubmitted" + i, true);
        }

        mSoundPool = new SoundPool.Builder().setMaxStreams(5).setAudioAttributes(
                new AudioAttributes.Builder()
                        .setUsage(AudioAttributes.USAGE_GAME)
                        .setContentType(AudioAttributes.CONTENT_TYPE_SONIFICATION)
                        .build()).build();

        mGoogleSignInClient = GoogleSignIn.getClient(this,
                new GoogleSignInOptions.Builder(GoogleSignInOptions.DEFAULT_GAMES_SIGN_IN).build());

        MobileAds.initialize(this, new OnInitializationCompleteListener() {
            @Override
            public void onInitializationComplete(InitializationStatus initializationStatus) {
                Log.d(TAG, "MobileAds.initialize() finished");
                mAdsInitialized = true;

                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        mAdView = new AdView(getContext());
                        mAdView.setAdSize(AdSize.BANNER);
                        mAdView.setAdUnitId("ca-app-pub-3940256099942544/6300978111");

                        AdRequest adRequest = new AdRequest.Builder().build();
                        mAdView.loadAd(adRequest);
                    }
                });
            }
        });

        loadPurchases();
        mBillingClient = BillingClient.newBuilder(this).enablePendingPurchases().setListener(new PurchasesUpdatedListener() {
            @Override
            public void onPurchasesUpdated(BillingResult billingResult, @Nullable List<Purchase> purchases) {
                if (billingResult.getResponseCode() == BillingClient.BillingResponseCode.OK && purchases != null) {
                    for (Purchase purchase : purchases) {
                        handlePurchase(purchase);
                    }
                }
            }
        }).build();

        startBillingConnection();
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.d(TAG, "onResume()");

        if (!mAdsHidden && mAdView != null)
            mAdView.resume();

        // Since the state of the signed in user can change when the activity is not active
        // it is recommended to try and sign in silently from when the app resumes.
        signInSilently();
    }

    @Override
    protected void onPause() {
        if (!mAdsHidden && mAdView != null)
            mAdView.pause();

        super.onPause();
    }

    @Override
    protected void onDestroy() {
        if (mAdView != null)
            mAdView.destroy();
        super.onDestroy();
    }


    public void showAlert(final String message)
    {
        final GameActivity activity = this;

        ApplicationInfo applicationInfo = activity.getApplicationInfo();
        final String applicationName = getString(R.string.app_name);

        this.runOnUiThread(new Runnable() {
            public void run() {
                AlertDialog.Builder builder = new AlertDialog.Builder(activity, AlertDialog.THEME_HOLO_DARK);
                builder.setTitle(applicationName);
                builder.setMessage(message);
                builder.setPositiveButton("Close", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        mSemaphore.release();
                    }
                });
                builder.setCancelable(false);
                AlertDialog dialog = builder.create();
                dialog.show();
            }
        });
        try {
            mSemaphore.acquire();
        }
        catch (InterruptedException e) { }
    }

    public void showAds(final int horizontalAlignment, final int verticalAlignment)
    {
        if (mCampaign3ProductBought || mAdView == null)
            return;

        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                try {
                    if (mAdsHidden) {
                        mAdView.resume();

                        mLayout.addView(mAdView);
                        mAdsHidden = false;
                    }

                    RelativeLayout.LayoutParams params = (RelativeLayout.LayoutParams) mAdView.getLayoutParams();

                    for (int i = 0; i < params.getRules().length; i++)
                        params.removeRule(i);
                    params.addRule(verticalAlignment);
                    params.addRule(horizontalAlignment);
                    mAdView.setLayoutParams(params);
                    mLayout.updateViewLayout(mAdView, params);
                } catch (Exception ex)
                {
                    Log.d(TAG, "showAds error: " + ex.getLocalizedMessage());
                }
            }
        });
    }

    public void hideAds()
    {
        if (!mAdsHidden && mAdView != null) {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    try {
                        mAdView.pause();
                        mLayout.removeView(mAdView);

                        mAdsHidden = true;
                    }  catch (Exception ex)
                    {
                        Log.d(TAG, "hideAds error: " + ex.getLocalizedMessage());
                    }
                }
            });
        }
    }

    public boolean showVideoAds()
    {
        if (!mBillingConnected || mRewardProduct == null)
        {
            Toast.makeText(this, "Can't display video ads",
                    Toast.LENGTH_SHORT).show();
            startBillingConnection();
            return false;
        }
        if (!mVideoProductAvailable)
        {
            Toast.makeText(this, "No video ads available",
                    Toast.LENGTH_SHORT).show();
            quaryNewVideo();
            return false;
        }

        BillingFlowParams flowParams = BillingFlowParams.newBuilder()
                .setSkuDetails(mRewardProduct)
                .build();
        BillingResult result = mBillingClient.launchBillingFlow(this, flowParams);

        return result.getResponseCode() == BillingClient.BillingResponseCode.OK;
    }

    public boolean wasVideoAdsWatched()
    {
        if (mVideosWatched > 0)
        {
            mVideosWatched--;
            return true;
        }
        return false;
    }

    public void unlockAchievement(final String achievementName)
    {
        if (mAchievementsClient != null) {
            int id = getResources().getIdentifier(achievementName, "string", this.getPackageName());
            mAchievementsClient.unlock(getString(id));
        }
    }

    public void updateAchievement(final String achievementName, int score)
    {
        if (mAchievementsClient != null) {
            int id = getResources().getIdentifier(achievementName, "string", this.getPackageName());
            mAchievementsClient.setSteps(getString(id), score);
        }
    }

    public void showAchievementUI()
    {
        mAchievementsClient
                .getAchievementsIntent()
                .addOnSuccessListener(new OnSuccessListener<Intent>() {
                    @Override
                    public void onSuccess(Intent intent) {
                        startActivityForResult(intent, RC_ACHIEVEMENT_UI);
                    }
                });
    }

    public void showLeaderboard(boolean weekly)
    {
        mLeaderboardsClient.getLeaderboardIntent(getString(R.string.leaderboard_scores), weekly ? TIME_SPAN_WEEKLY : TIME_SPAN_ALL_TIME, COLLECTION_PUBLIC)
                .addOnSuccessListener(new OnSuccessListener<Intent>() {
            @Override
            public void onSuccess(Intent intent) {
                startActivityForResult(intent, RC_LEADERBOARD_UI);
            }
        });
    }

    public void showTimeLeaderboard(int level, boolean weekly)
    {
        String levelStr = level < 9 ? "0" : "";
        levelStr = levelStr + (level + 1);
        int id = getResources().getIdentifier("leaderboard_level" + levelStr + "time", "string", this.getPackageName());

        mLeaderboardsClient.getLeaderboardIntent(getString(id), weekly ? TIME_SPAN_WEEKLY : TIME_SPAN_ALL_TIME, COLLECTION_PUBLIC)
                .addOnSuccessListener(new OnSuccessListener<Intent>() {
                    @Override
                    public void onSuccess(Intent intent) {
                        startActivityForResult(intent, RC_LEADERBOARD_UI);
                    }
                });
    }

    public void playSound(int id, float volume, boolean loop)
    {
        if (id == -100)
        {
            mMusic.setLooping(loop);
            mMusic.start();
        } else {
            mSoundPool.play(id, volume, volume, 0, loop ? -1 : 0, 1);
        }
    }

    public void setMusicVolume(float volume)
    {
        mMusic.setVolume(volume, volume);
    }

    public void stopSound(int id)
    {
        if (id == -100)
        {
            mMusic.pause();
        } else {
            mSoundPool.stop(id);
        }
    }

    public int loadSound(final String filename, boolean isMusic)
    {
        try {
            if (!isMusic) {
                return mSoundPool.load(getAssets().openFd(filename), 1);
            } else {
                AssetFileDescriptor afd = getAssets().openFd(filename);
                mMusic = new MediaPlayer();
                mMusic.setDataSource(afd.getFileDescriptor(), afd.getStartOffset(), afd.getLength());
                afd.close();
                mMusic.prepare();
                return -100;
            }
        } catch (IOException e)
        {
            return -1;
        }
    }

    public int getQuality()
    {
        if (android.os.Build.VERSION.SDK_INT <= Build.VERSION_CODES.N_MR1)
        {
            return 0;
        } else {
            ActivityManager actManager = (ActivityManager) getSystemService(ACTIVITY_SERVICE);
            ActivityManager.MemoryInfo memInfo = new ActivityManager.MemoryInfo();
            actManager.getMemoryInfo(memInfo);
            int totalMemory = (int)(memInfo.totalMem / 1048576);

            if (totalMemory < 2000)
                return 0;

            if (totalMemory > 5000)
                return 2;
        }

        return 4;
    }

    public int getAndroidVersion()
    {
        return android.os.Build.VERSION.SDK_INT;
    }

    public void restartApp()
    {
        Intent mStartActivity = new Intent(getContext(), SDLActivity.class);
        int mPendingIntentId = 132465;
        PendingIntent mPendingIntent = PendingIntent.getActivity(getContext(), mPendingIntentId,    mStartActivity, PendingIntent.FLAG_CANCEL_CURRENT);
        AlarmManager mgr = (AlarmManager)getContext().getSystemService(Context.ALARM_SERVICE);
        mgr.set(AlarmManager.RTC, System.currentTimeMillis() + 1000, mPendingIntent);
        System.exit(0);
    }

    private void signInSilently() {
        Log.d(TAG, "signInSilently()");

        mGoogleSignInClient.silentSignIn().addOnCompleteListener(this,
                new OnCompleteListener<GoogleSignInAccount>() {
                    @Override
                    public void onComplete(@NonNull Task<GoogleSignInAccount> task) {
                        if (task.isSuccessful()) {
                            Log.d(TAG, "signInSilently(): success");
                            onConnected(task.getResult());
                        } else {
                            Log.d(TAG, "signInSilently(): failure", task.getException());
                            onDisconnected();
                        }
                    }
                });
    }

    private void startSignInIntent() {
        startActivityForResult(mGoogleSignInClient.getSignInIntent(), RC_SIGN_IN);
    }

    // Google services requests
    public void logInGoogle()
    {
        startSignInIntent();
    }

    public void logOutGoogle()
    {
        mGoogleSignInClient.signOut().addOnCompleteListener(this,
                new OnCompleteListener<Void>() {
                    @Override
                    public void onComplete(@NonNull Task<Void> task) {
                        boolean successful = task.isSuccessful();
                        Log.d(TAG, "signOut(): " + (successful ? "success" : "failed"));

                        onDisconnected();
                    }
                });
    }

    public boolean isGoogleServicesAvailable()
    {
        int resultCode = GoogleApiAvailability.getInstance().isGooglePlayServicesAvailable(getContext());
        return resultCode == ConnectionResult.SUCCESS;
    }

    public boolean isGoogleEverSigned()
    {
        return GoogleSignIn.getLastSignedInAccount(this) != null;
    }

    public boolean isGoogleLogged()
    {
        return mLeaderboardsClient != null;
    }

    public void storeSubmitState(int level)
    {
        SharedPreferences.Editor editor = mSharedPreferences.edit();
        editor.putBoolean("isSubmitted" + level, mScoreSubmitted[level]);
        editor.apply();
    }

    public boolean[] getSubmitState()
    {
        return mScoreSubmitted;
    }

    public void submitScore(int score, final boolean isBest)
    {
        if (score <= 0)
            return;

        if (isBest)
        {
            mScoreSubmitted[36] = false;
            storeSubmitState(36);
        }

        if (isGoogleLogged())
        {
            Task<ScoreSubmissionData> task = mLeaderboardsClient.submitScoreImmediate(getString(R.string.leaderboard_scores), score);
            task.addOnCompleteListener(this,
                    new OnCompleteListener<ScoreSubmissionData>() {
                        @Override
                        public void onComplete(@NonNull Task<ScoreSubmissionData> task) {
                            boolean successful = task.isSuccessful();
                            Log.d(TAG, "submitScore(): " + (successful ? "success" : "failed"));

                            if (isBest) {
                                if (successful)
                                    mScoreSubmitted[36] = true;

                                storeSubmitState(36);
                            }
                        }
            });
        }
    }

    public void submitTimeScore(final int level, int timeSeconds, final boolean isBest)
    {
        if (level >= 36)
            return;

        if (isBest)
        {
            mScoreSubmitted[level] = false;
            storeSubmitState(level);
        }

        if (isGoogleLogged())
        {
            String levelStr = level < 9 ? "0" : "";
            levelStr = levelStr + (level + 1);
            int id = getResources().getIdentifier("leaderboard_level" + levelStr + "time", "string", this.getPackageName());

            Task<ScoreSubmissionData> task = mLeaderboardsClient.submitScoreImmediate(getString(id), (long)timeSeconds * 1000);
            task.addOnCompleteListener(this,
                    new OnCompleteListener<ScoreSubmissionData>() {
                        @Override
                        public void onComplete(@NonNull Task<ScoreSubmissionData> task) {
                            boolean successful = task.isSuccessful();
                            Log.d(TAG, "submitTimeScore(): " + (successful ? "success" : "failed"));

                            if (isBest) {
                                if (successful)
                                    mScoreSubmitted[level] = true;

                                storeSubmitState(level);
                            }
                        }
                    });
        }
    }

    OnCompleteListener<AnnotatedData<LeaderboardsClient.LeaderboardScores>>  getLeaderboardListener(final boolean weekly)
    {
        final ScoreInfo[] scoresList = weekly ? mWeeklyScores : mFullScores;

        return new OnCompleteListener<AnnotatedData<LeaderboardsClient.LeaderboardScores>>() {

            @Override
            public void onComplete(@NonNull Task<AnnotatedData<LeaderboardsClient.LeaderboardScores>> task) {
                try {
                    if (task.isSuccessful()) {
                        AnnotatedData<LeaderboardsClient.LeaderboardScores> result = task.getResult();
                        LeaderboardsClient.LeaderboardScores scores = result.get();
                        LeaderboardScoreBuffer buffer = scores.getScores();
                        int scoreIndex = 0;
                        for (LeaderboardScore item : buffer) {
                            scoresList[scoreIndex] = new ScoreInfo();
                            scoresList[scoreIndex].name = item.getScoreHolder().getDisplayName();
                            scoresList[scoreIndex].score = item.getRawScore();
                            scoreIndex++;
                        }
                        if (weekly)
                            mWeeklyScoresSize = scoreIndex;
                        else
                            mFullScoresSize = scoreIndex;
                        buffer.release();
                        scores.release();
                        Log.d(TAG, "getTopScores:: successfully retrieved data");
                        mScoresUpdated[weekly ? 0 : 1] = true;
                    } else {
                        String error = "Unknown error";
                        if (task.getException() != null) {
                            error = task.getException().getMessage();
                        }
                        Log.d(TAG, "getTopScores can't retrieve data: " + error);
                    }
                } catch (Exception e) {
                    Log.d(TAG, "getTopScores::onComplete: " + e.getMessage());
                }
            }
        };
    }

    public boolean hasNewTimeScoreChanges()
    {
        if (mTimeScoreNewChanges)
        {
            mTimeScoreNewChanges = false;
            return true;
        }
        return false;
    }

    public boolean hasTimescoresUpdated()
    {
        for (int i = 0; i < 36; i++)
            if (!mTimeScoresUpdatedWeekly[i] || !mTimeScoresUpdatedFull[i])
                return false;

        return true;
    }

    OnCompleteListener<AnnotatedData<LeaderboardsClient.LeaderboardScores>>  getTimeLeaderboardListener(final boolean weekly, final int level)
    {
        return new OnCompleteListener<AnnotatedData<LeaderboardsClient.LeaderboardScores>>() {

            @Override
            public void onComplete(@NonNull Task<AnnotatedData<LeaderboardsClient.LeaderboardScores>> task) {
                try {
                    if (task.isSuccessful()) {
                        AnnotatedData<LeaderboardsClient.LeaderboardScores> result = task.getResult();
                        LeaderboardsClient.LeaderboardScores scores = result.get();
                        LeaderboardScoreBuffer buffer = scores.getScores();
                        if (buffer.getCount() > 0) {
                            ScoreInfo scoreInfo;
                            if (weekly) {
                                mWeeklyTimeScores[level] = new ScoreInfo();
                                scoreInfo = mWeeklyTimeScores[level];
                            }
                            else {
                                mFullTimeScores[level] = new ScoreInfo();
                                scoreInfo = mFullTimeScores[level];
                            }

                            LeaderboardScore topScore = buffer.get(0);
                            scoreInfo.name = topScore.getScoreHolderDisplayName();
                            scoreInfo.score = topScore.getRawScore();

                            mTimeScoreNewChanges = true;
                        }

                        if (weekly)
                            mTimeScoresUpdatedWeekly[level] = true;
                        else
                            mTimeScoresUpdatedFull[level] = true;

                        buffer.release();
                        scores.release();
                        Log.d(TAG, "getTimeTopScores: successfully retrieved data for level " + level);
                    } else {
                        String error = "Unknown error";
                        if (task.getException() != null) {
                            error = task.getException().getMessage();
                        }
                        Log.d(TAG, "getTimeTopScores can't retrieve data: " + error);
                    }
                } catch (Exception e) {
                    Log.d(TAG, "getTimeTopScores::onComplete: " + e.getMessage());
                }
            }
        };
    }

    public void refreshScores()
    {
        mRefreshScores = true;
        getTopScores();
        getTopTimeScores();
        mRefreshScores = false;
    }

    public void getTopScores()
    {
        mScoresUpdated[0] = false;
        mScoresUpdated[1] = false;

        mLeaderboardsClient.loadTopScores(
                getString(R.string.leaderboard_scores), LeaderboardVariant.TIME_SPAN_ALL_TIME, COLLECTION_PUBLIC, 5, mRefreshScores)
                .addOnCompleteListener(this, getLeaderboardListener(false));

        mLeaderboardsClient.loadTopScores(
                getString(R.string.leaderboard_scores), TIME_SPAN_WEEKLY, COLLECTION_PUBLIC, 5, mRefreshScores)
                .addOnCompleteListener(this, getLeaderboardListener(true));

        mLeaderboardsClient.loadCurrentPlayerLeaderboardScore(
                getString(R.string.leaderboard_scores), LeaderboardVariant.TIME_SPAN_ALL_TIME, COLLECTION_PUBLIC)
                .addOnCompleteListener(this, new OnCompleteListener<AnnotatedData<LeaderboardScore>>() {
                    @Override
                    public void onComplete(@NonNull Task<AnnotatedData<LeaderboardScore>> task) {
                        try {
                            if (task.isSuccessful()) {
                                LeaderboardScore result = task.getResult().get();
                                mFullScores[5] = new ScoreInfo();
                                mFullScores[5].score = result.getRawScore();
                                mFullScores[5].name = result.getScoreHolderDisplayName();
                                mPlayerPlace[0] = result.getRank();
                            } else {
                                mPlayerPlace[0] = 0;
                            }
                        } catch (Exception ex)
                        {
                            Log.d(TAG, "getTopScores::loadCurrentPlayerLeaderboardScore::onComplete (all-time): " + ex.getMessage());
                            mPlayerPlace[0] = 0;
                        }
                    }
                });

        mLeaderboardsClient.loadCurrentPlayerLeaderboardScore(
                getString(R.string.leaderboard_scores), TIME_SPAN_WEEKLY, COLLECTION_PUBLIC)
                .addOnCompleteListener(this, new OnCompleteListener<AnnotatedData<LeaderboardScore>>() {
                    @Override
                    public void onComplete(@NonNull Task<AnnotatedData<LeaderboardScore>> task) {
                        try {
                            if (task.isSuccessful()) {
                                LeaderboardScore result = task.getResult().get();
                                mWeeklyScores[5] = new ScoreInfo();
                                mWeeklyScores[5].score = result.getRawScore();
                                mWeeklyScores[5].name = result.getScoreHolderDisplayName();
                                mPlayerPlace[1] = result.getRank();
                            } else {
                                mPlayerPlace[1] = 0;
                            }
                        } catch (Exception ex)
                        {
                            Log.d(TAG, "getTopScores::loadCurrentPlayerLeaderboardScore::onComplete (weekly): " + ex.getMessage());
                            mPlayerPlace[1] = 0;
                        }
                    }
                });
    }

    public void getTopTimeScores()
    {
        mTimeScoreNewChanges = false;
        for (int i = 0; i < 36; i++) {
            mTimeScoresUpdatedFull[i] = false;
            mTimeScoresUpdatedWeekly[i] = false;
        }

        final GameActivity activity = this;

        AsyncTask.execute(new Runnable() {
            @Override
            public void run() {
                try {
                    for (int i = 0; i < 36; i++) {
                        String levelStr = i < 9 ? "0" : "";
                        levelStr = levelStr + (i + 1);
                        int id = getResources().getIdentifier("leaderboard_level" + levelStr + "time", "string", activity.getPackageName());

                        mLeaderboardsClient.loadTopScores(
                                getString(id), LeaderboardVariant.TIME_SPAN_ALL_TIME, COLLECTION_PUBLIC, 1, mRefreshScores)
                                .addOnCompleteListener(activity, getTimeLeaderboardListener(false, i));

                        mLeaderboardsClient.loadTopScores(
                                getString(id), TIME_SPAN_WEEKLY, COLLECTION_PUBLIC, 1, mRefreshScores)
                                .addOnCompleteListener(activity, getTimeLeaderboardListener(true, i));
                        try {
                            Thread.sleep(500);
                        } catch (InterruptedException ignored) { }
                    }
                } catch (Exception ex)
                {
                    Log.d(TAG, "getTopTimeScores error: " + ex.getLocalizedMessage());
                }
            }
        });

    }

    public boolean isScoresUpdated()
    {
        return mScoresUpdated[0] && mScoresUpdated[1] && mPlayerPlace[0] != -2 && mPlayerPlace[1] != -2;
    }

    public String getTopScoresName(int rank, boolean isWeekly)
    {
        if (rank < 0)
            return "";
        if (isWeekly)
        {
            if (rank >= mWeeklyScoresSize)
                return "";
            return mWeeklyScores[rank].name;
        } else {
            if (rank >= mFullScoresSize)
                return "";
            return mFullScores[rank].name;
        }
    }

    public int getTopScoresScore(int rank, boolean isWeekly)
    {
        if (rank < 0)
            return 0;
        if (isWeekly)
        {
            if (rank >= mWeeklyScoresSize)
                return 0;
            return (int)mWeeklyScores[rank].score;
        } else {
            if (rank >= mFullScoresSize)
                return 0;
            return (int)mFullScores[rank].score;
        }
    }

    public String getTopTimeScoresName(int level, boolean isWeekly)
    {
        if (level < 0 || level >= 36)
            return "";
        if (isWeekly)
        {
            if (mWeeklyTimeScores[level] == null)
                return "";
            return mWeeklyTimeScores[level].name;
        } else {
            if (mFullTimeScores[level] == null)
                return "";
            return mFullTimeScores[level].name;
        }
    }

    public int getTopTimeScoresValue(int level, boolean isWeekly)
    {
        if (level < 0 || level >= 36)
            return 0;
        if (isWeekly)
        {
            if (mWeeklyTimeScores[level] == null)
                return 0;
            return (int)mWeeklyTimeScores[level].score;
        } else {
            if (mFullTimeScores[level] == null)
                return 0;
            return (int)mFullTimeScores[level].score;
        }
    }

    public int getPlayerPlace(boolean weekly)
    {
        return (int)mPlayerPlace[weekly ? 1 : 0];
    }

    public int getPlayerScore(boolean weekly)
    {
        if (weekly)
        {
            if (mWeeklyScores[5] == null)
                return 0;
            return (int)mWeeklyScores[5].score;
        } else {
            if (mFullScores[5] == null)
                return 0;
            return (int)mFullScores[5].score;
        }
    }

    public String getPlayerName(boolean weekly)
    {
        if (weekly)
        {
            if (mWeeklyScores[5] == null)
                return "Player";
            return mWeeklyScores[5].name;
        } else {
            if (mFullScores[5] == null)
                return "Player";
            return mFullScores[5].name;
        }
    }

    public String getPlayerDisplayName()
    {
        return mPlayerDisplayName;
    }

    public boolean isCampaignPurchased()
    {
        return mCampaign3ProductBought;
    }

    public void purchaseCampaign()
    {
        if (!mBillingConnected || mCampaign3Product == null) {
            startBillingConnection();
            return;
        }

        BillingFlowParams flowParams = BillingFlowParams.newBuilder()
                .setSkuDetails(mCampaign3Product)
                .build();
        BillingResult result = mBillingClient.launchBillingFlow(this, flowParams);
    }

    public void openLink(String link)
    {
        Intent browserIntent = new Intent(Intent.ACTION_VIEW, Uri.parse(link));
        startActivity(browserIntent);
    }

    public byte[] encrypt(byte[] data)
    {
        return EncryptUtils.encrypt(this, data);
    }

    public byte[] decrypt(byte[] data)
    {
        return EncryptUtils.decrypt(this, data);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent intent) {
        super.onActivityResult(requestCode, resultCode, intent);
        if (requestCode == RC_SIGN_IN) {
            Task<GoogleSignInAccount> task =
                    GoogleSignIn.getSignedInAccountFromIntent(intent);

            try {
                GoogleSignInAccount account = task.getResult(ApiException.class);
                onConnected(account);
            } catch (ApiException apiException) {
                String message = apiException.getMessage();
                if (message == null || message.isEmpty()) {
                    message = getString(R.string.signin_other_error);
                }

                onDisconnected();
                Log.d(TAG, "Log in error: " + message);

                Toast.makeText(this, "Sign in error: " + message,
                        Toast.LENGTH_LONG).show();
            }
        }
    }

    private void onConnected(GoogleSignInAccount googleSignInAccount)
    {
        Log.d(TAG, "onConnected(): connected to Google APIs");

        GamesClient gamesClient = Games.getGamesClient(this, googleSignInAccount);
        gamesClient.setViewForPopups(mSurface);

        if (mPlayerDisplayName.equals("Player")) {
            Games.getPlayersClient(this, googleSignInAccount).getCurrentPlayer().addOnCompleteListener(
                    new OnCompleteListener<Player>() {
                        @Override
                        public void onComplete(@NonNull Task<Player> task) {
                            if (task.isSuccessful()) {
                                String displayName = task.getResult().getDisplayName();
                                if (displayName != null) {
                                    mPlayerDisplayName = displayName;

                                    SharedPreferences.Editor editor = mSharedPreferences.edit();
                                    editor.putString("playerName", displayName);
                                    editor.apply();
                                }
                            }
                        }
                    });
        }

        mAchievementsClient = Games.getAchievementsClient(this, googleSignInAccount);
        mLeaderboardsClient = Games.getLeaderboardsClient(this, googleSignInAccount);

        getTopTimeScores();
        getTopScores();
    }

    private void onDisconnected()
    {
        Log.d(TAG, "onDisconnected()");

        mAchievementsClient = null;
        mLeaderboardsClient = null;
    }

    private void storePurchases()
    {
        // TODO: Do more secure store
        SharedPreferences.Editor editor = mSharedPreferences.edit();
        editor.putBoolean(mCampaignProductName, mCampaign3ProductBought);
        editor.apply();
    }

    private void loadPurchases()
    {
        mCampaign3ProductBought = mSharedPreferences.getBoolean(mCampaignProductName, false);
        Log.d(TAG, "Loaded purchase: " + mCampaign3ProductBought);
    }

    private void quaryPurchases()
    {
        if (!mBillingConnected)
            return;

        List<String> skuList = new ArrayList<>();
        skuList.add(mCampaignProductName);
        skuList.add(mRewardProductName);
        SkuDetailsParams.Builder params = SkuDetailsParams.newBuilder();
        params.setSkusList(skuList).setType(BillingClient.SkuType.INAPP);
        mBillingClient.querySkuDetailsAsync(params.build(),
                new SkuDetailsResponseListener() {
                    @Override
                    public void onSkuDetailsResponse(BillingResult billingResult,
                                                     List<SkuDetails> skuDetailsList) {
                        if (billingResult.getResponseCode() == BillingClient.BillingResponseCode.OK && skuDetailsList != null)
                        {
                            for (SkuDetails skuDetails : skuDetailsList)
                            {
                                if (mCampaignProductName.equals(skuDetails.getSku()))
                                {
                                    mCampaign3Product = skuDetails;
                                }
                                else if (mRewardProductName.equals(skuDetails.getSku()))
                                {
                                    mRewardProduct = skuDetails;
                                    quaryNewVideo();
                                }
                            }
                        }
                    }
                });

        mBillingClient.queryPurchaseHistoryAsync(BillingClient.SkuType.INAPP, new PurchaseHistoryResponseListener() {
            @Override
            public void onPurchaseHistoryResponse(BillingResult billingResult, List<PurchaseHistoryRecord> list) {
                /* do nothing, just refresh the cache */
            }
        });
        Purchase.PurchasesResult purchasesResult = mBillingClient.queryPurchases(BillingClient.SkuType.INAPP);
        if (purchasesResult.getBillingResult().getResponseCode() == BillingClient.BillingResponseCode.OK)
        {
            for (Purchase purchase : purchasesResult.getPurchasesList())
            {
                Log.d(TAG, "Purchase: " + purchase.getSku() + ", State: " + purchase.getPurchaseState());
                if (purchase.getPurchaseState() == Purchase.PurchaseState.PURCHASED
                     && purchase.getSku().equals(mCampaignProductName))
                {
                    mCampaign3ProductBought = true;
                    storePurchases();
                }
            }
        }
    }

    private void quaryNewVideo()
    {
        if (mRewardProduct != null && mRewardProduct.isRewarded()) {
            RewardLoadParams.Builder params = RewardLoadParams.newBuilder();
            params.setSkuDetails(mRewardProduct);
            mBillingClient.loadRewardedSku(params.build(),
                    new RewardResponseListener() {
                        @Override
                        public void onRewardResponse(BillingResult billingResult) {
                            if (billingResult.getResponseCode() == BillingClient.BillingResponseCode.OK)
                            {
                                mVideoProductAvailable = true;
                            }
                        }
                    });
        }
    }

    private void startBillingConnection()
    {
        mBillingClient.startConnection(new BillingClientStateListener() {
            @Override
            public void onBillingSetupFinished(BillingResult billingResult) {
                if (billingResult.getResponseCode() == BillingClient.BillingResponseCode.OK) {
                    mBillingConnected = true;
                    quaryPurchases();
                }
            }
            @Override
            public void onBillingServiceDisconnected() {
                mBillingConnected = false;
            }
        });
    }

    private void handlePurchase(Purchase purchase) {
        if (purchase.getPurchaseState() == Purchase.PurchaseState.PURCHASED) {
            if (purchase.getSku().equals(mCampaignProductName)) {
                mCampaign3ProductBought = true;
                storePurchases();

                // Acknowledge the purchase if it hasn't already been acknowledged.
                if (!purchase.isAcknowledged()) {
                    AcknowledgePurchaseParams acknowledgePurchaseParams =
                            AcknowledgePurchaseParams.newBuilder()
                                    .setPurchaseToken(purchase.getPurchaseToken())
                                    .build();
                    mBillingClient.acknowledgePurchase(acknowledgePurchaseParams, new AcknowledgePurchaseResponseListener() {
                        @Override
                        public void onAcknowledgePurchaseResponse(BillingResult billingResult) {
                            if(billingResult.getResponseCode() == BillingClient.BillingResponseCode.OK){
                                Log.d(TAG, "Purchase acknowledged");
                            } else {
                                mCampaign3ProductBought = false;
                                storePurchases();
                            }
                        }
                    });
                }
            }
            if (purchase.getSku().equals(mRewardProductName))
            {
                ConsumeParams consumeParams =
                        ConsumeParams.newBuilder()
                                .setPurchaseToken(purchase.getPurchaseToken())
                                .setDeveloperPayload(purchase.getDeveloperPayload())
                                .build();

                mBillingClient.consumeAsync(consumeParams, new ConsumeResponseListener() {
                    @Override
                    public void onConsumeResponse(BillingResult billingResult, String outToken) {
                        if (billingResult.getResponseCode() == BillingClient.BillingResponseCode.OK) {
                            ++mVideosWatched;
                            quaryNewVideo();
                        }
                    }
                });
            }
        }
    }
}
