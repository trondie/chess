package com.eskilsund.etchess3d.etchess3d;

import android.annotation.TargetApi;
import android.app.Activity;
import android.app.Dialog;
import android.os.Build;
import android.support.annotation.NonNull;
import android.support.design.widget.Snackbar;
import android.support.v4.app.Fragment;
import android.content.Intent;
import android.content.IntentSender;
import android.os.Bundle;
import android.text.SpannableStringBuilder;
import android.util.Log;
import android.view.WindowManager;

import com.google.android.gms.auth.api.signin.GoogleSignIn;
import com.google.android.gms.auth.api.signin.GoogleSignInClient;
import com.google.android.gms.auth.api.signin.GoogleSignInAccount;
import com.google.android.gms.auth.api.signin.GoogleSignInOptions;
import com.google.android.gms.common.api.ApiException;
import com.google.android.gms.games.AchievementsClient;
import com.google.android.gms.games.Games;
import com.google.android.gms.games.GamesActivityResultCodes;
import com.google.android.gms.games.GamesCallbackStatusCodes;
import com.google.android.gms.games.GamesClient;
import com.google.android.gms.games.GamesClientStatusCodes;
import com.google.android.gms.games.InvitationsClient;
import com.google.android.gms.games.Player;
import com.google.android.gms.games.PlayersClient;
import com.google.android.gms.games.RealTimeMultiplayerClient;
import com.google.android.gms.games.multiplayer.Invitation;
import com.google.android.gms.games.multiplayer.InvitationCallback;
import com.google.android.gms.games.multiplayer.Multiplayer;
import com.google.android.gms.games.multiplayer.Participant;
import com.google.android.gms.games.multiplayer.realtime.OnRealTimeMessageReceivedListener;
import com.google.android.gms.games.multiplayer.realtime.RealTimeMessage;
import com.google.android.gms.games.multiplayer.realtime.Room;
import com.google.android.gms.games.multiplayer.realtime.RoomConfig;
import com.google.android.gms.games.multiplayer.realtime.RoomStatusUpdateCallback;
import com.google.android.gms.games.multiplayer.realtime.RoomUpdateCallback;
import com.google.android.gms.tasks.OnCanceledListener;
import com.google.android.gms.tasks.OnCompleteListener;
import com.google.android.gms.tasks.OnFailureListener;
import com.google.android.gms.tasks.OnSuccessListener;
import com.google.android.gms.tasks.Task;

import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.List;
import java.util.Random;

import static com.google.android.gms.games.GamesStatusCodes.getStatusString;

/**
 * Created by Eskil on 26.09.2016.
 */
@TargetApi(Build.VERSION_CODES.KITKAT)
public class GameActivity extends Fragment {

    final static String TAG = "eskil";

    game_menu menuActivity = null;

    // Client used to sign in with Google APIs
    private GoogleSignInClient mGoogleSignInClient = null;
    private RealTimeMultiplayerClient mRealTimeMultiplayerClient = null;
    private InvitationsClient mInvitationsClient = null;
    private AchievementsClient mAchievementsClient;

    // The currently signed in account, used to check the account has changed outside of this activity when resuming.
    GoogleSignInAccount mSignedInAccount = null;

    String mRoomId = null;
    private static final int RC_SIGN_IN = 9001;
    final static int RC_SELECT_PLAYERS = 10000;
    final static int RC_INVITATION_INBOX = 10001;
    final static int RC_WAITING_ROOM = 10002;
    boolean mWaiting = false;
    boolean mInWaitingRoom = false;
    String mMyId = null;
    ArrayList<Participant> mParticipants = null;
    private String mIncomingInvitationId = null;
    private String mPlayerId;
    Room mRoom = null;
    RoomConfig mRoomConfig = null;

    @Override
    public void onStop() {
        // if we're in a room, leave it.
        //leaveRoom();
        super.onStop();
    }

    @Override
    public void onStart() {
        isConnected();
        super.onStart();
    }

    public Boolean isConnected() {
        boolean retval = false;
        if (null == mSignedInAccount) {
            signInSilently();
            if (null != mSignedInAccount) {
                retval = true;
            }
        }
        else
        {
            retval = true;
        }

        return retval;
    }

    void setGoogleSignInClient(GoogleSignInClient signInClient) {
        mGoogleSignInClient = signInClient;
    }

    public void signInSilently() {
        Log.d(TAG, "signInSilently()");

        mGoogleSignInClient.silentSignIn().addOnCompleteListener(menuActivity,
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

    private void handleException(Exception exception, String details) {
        int status = 0;

        if (exception instanceof ApiException) {
            ApiException apiException = (ApiException) exception;
            status = apiException.getStatusCode();
        }

        String errorString = null;
        switch (status) {
            case GamesCallbackStatusCodes.OK:
                break;
            case GamesClientStatusCodes.MULTIPLAYER_ERROR_NOT_TRUSTED_TESTER:
                errorString = "GamesClientStatusCodes.MULTIPLAYER_ERROR_NOT_TRUSTED_TESTER";
                break;
            case GamesClientStatusCodes.MATCH_ERROR_ALREADY_REMATCHED:
                errorString = "GamesClientStatusCodes.MATCH_ERROR_ALREADY_REMATCHED";
                break;
            case GamesClientStatusCodes.NETWORK_ERROR_OPERATION_FAILED:
                errorString = "GamesClientStatusCodes.NETWORK_ERROR_OPERATION_FAILED";
                break;
            case GamesClientStatusCodes.INTERNAL_ERROR:
                errorString = "GamesClientStatusCodes.INTERNAL_ERROR";
                break;
            case GamesClientStatusCodes.MATCH_ERROR_INACTIVE_MATCH:
                errorString = "GamesClientStatusCodes.MATCH_ERROR_INACTIVE_MATCH";
                break;
            case GamesClientStatusCodes.MATCH_ERROR_LOCALLY_MODIFIED:
                errorString = "GamesClientStatusCodes.MATCH_ERROR_LOCALLY_MODIFIED";
                break;
            default:
                errorString = "Unexpected status: " + GamesClientStatusCodes.getStatusCodeString(status);
                break;
        }

        if (errorString == null) {
            return;
        }

        String message = "Status exception error. " + " : " + details + " : " + status + " : " + exception;

        Snackbar.make(menuActivity.findViewById(android.R.id.content), message, Snackbar.LENGTH_LONG)
                .setAction("Action", null)
                .show();
    }

    private OnFailureListener createFailureListener(final String string) {
        return new OnFailureListener() {
            @Override
            public void onFailure(@NonNull Exception e) {
                handleException(e, string);
            }
        };
    }

    private InvitationCallback mInvitationCallback = new InvitationCallback() {
        @Override
        public void onInvitationRemoved(String invitationId) {
            Log.d(TAG, "invitation removed");
            if (mIncomingInvitationId.equals(invitationId) && mIncomingInvitationId != null) {
                mIncomingInvitationId = null;
            }
        }

        @Override
        public void onInvitationReceived(Invitation invitation) {
            Log.d(TAG, "invitation received");
            mIncomingInvitationId = invitation.getInvitationId();
            menuActivity.showInvitation(invitation.getInviter().getDisplayName(), mIncomingInvitationId);
            menuActivity.getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        }
    };

    public void respondToInvitation(boolean accept, String invitationId) {
        if (accept) {
            acceptInviteToRoom(invitationId);
        }
        else {
            rejectInviteToRoom();
        }
    }

    // Accept the given invitation.
    void acceptInviteToRoom(String invitationId) {
        // accept the invitation
        Log.d(TAG, "Accepting invitation: " + invitationId);

        mRoomConfig = RoomConfig.builder(mRoomUpdateCallback)
                .setInvitationIdToAccept(invitationId)
                .setOnMessageReceivedListener(mOnRealTimeMessageReceivedListener)
                .setRoomStatusUpdateCallback(mRoomStatusUpdateCallback)
                .build();

        mRealTimeMultiplayerClient.join(mRoomConfig)
                .addOnSuccessListener(new OnSuccessListener<Void>() {
                    @Override
                    public void onSuccess(Void aVoid) {
                        Log.d(TAG, "Room Joined Successfully!");
                    }
                });
    }

    OnRealTimeMessageReceivedListener mOnRealTimeMessageReceivedListener = new OnRealTimeMessageReceivedListener() {

        @Override
        public void onRealTimeMessageReceived(RealTimeMessage rtm) {
            byte[] buf = rtm.getMessageData();
            String sender = rtm.getSenderParticipantId();
            String msg;
            try {
                msg = new String(buf, "UTF-8");
            } catch (UnsupportedEncodingException e) {
                msg = e.getMessage();
            }
            if (msg.startsWith("Negotiate: ")) {
                try {
                    opponentNegotiateValue = Integer.parseInt(msg.substring(11));
                    if (opponentNegotiateValue != myNegotiateValue) {
                        negotiateDone = true;
                        menuActivity.setMultiplayerColor(isOpponentWhite());
                    } else if (negotiateStarted) {
                        negotiateColor(getRoom());
                    }
                } catch(NumberFormatException e) {
                    Log.e(TAG, "Number received is wrong " + msg.substring(11));
                }
            } else if (msg.startsWith("Move: ")) {
                try {
                    String moveValues = msg.substring(6);
                    String move[] = moveValues.split(",");
                    int from_x = Integer.parseInt(move[0]);
                    int from_y = Integer.parseInt(move[1]);
                    int to_x = Integer.parseInt(move[2]);
                    int to_y = Integer.parseInt(move[3]);
                    menuActivity.nativeMove(from_x,from_y,to_x,to_y);
                } catch(NumberFormatException e) {
                    Log.e(TAG, "Number received is wrong " + msg.substring(6));
                }
            } else if (msg.startsWith("Start: ")) {
                String startMode = msg.substring(7);
                boolean load_game;
                if (startMode.startsWith("new")) {
                    load_game = false;
                } else {
                    load_game = true;
                }
                //Notify the user that opponent has started
                menuActivity.askToStartGame(load_game);
            } else if (msg.startsWith("Accept: ")) {
                //Notified that opponent accepted our new game request.
                menuActivity.newGameAccepted();
            }
            Log.d(TAG, msg);
        }
    };

    void rejectInviteToRoom() {
        // accept the invitation
        if (mIncomingInvitationId != null) {
            Log.d(TAG, "Rejecting invitation: " + mIncomingInvitationId);

            mRealTimeMultiplayerClient.declineInvitation(mIncomingInvitationId)
                    .addOnSuccessListener(new OnSuccessListener<Void>() {
                        @Override
                        public void onSuccess(Void aVoid) {
                            Log.d(TAG, "Rejecting invitation done!");
                            mIncomingInvitationId = null;
                        }
                    });
        }
    }


    public void signOut() {
        Log.d(TAG, "signOut()");

        mGoogleSignInClient.signOut().addOnCompleteListener(menuActivity,
                new OnCompleteListener<Void>() {
                    @Override
                    public void onComplete(@NonNull Task<Void> task) {

                        if (task.isSuccessful()) {
                            Log.d(TAG, "signOut(): success");
                        } else {
                            handleException(task.getException(), "signOut() failed!");
                        }

                        onDisconnected();
                    }
                });
    }

    void showWaitingRoom(Room room) {
        if (mInWaitingRoom == true || mWaiting == false) {
            return;
        }

        mInWaitingRoom = true;
        // minimum number of players required for our game
        // For simplicity, we require everyone to join the game before we start it
        // (this is signaled by Integer.MAX_VALUE).
        final int MIN_PLAYERS = Integer.MAX_VALUE;
        mRealTimeMultiplayerClient.getWaitingRoomIntent(room, MIN_PLAYERS)
                .addOnSuccessListener(new OnSuccessListener<Intent>() {
                    @Override
                    public void onSuccess(Intent intent) {
                        // show waiting room UI
                        startActivityForResult(intent, RC_WAITING_ROOM);
                    }
                })
                .addOnFailureListener(createFailureListener("There was a problem getting the waiting room!"));
    }

    private RoomUpdateCallback mRoomUpdateCallback = new RoomUpdateCallback() {

        @Override
        public void onLeftRoom(int statusCode, String roomId) {
            // we have left the room; return to main screen.
            Log.d(TAG, "onLeftRoom, code " + statusCode);
        }


        @Override
        public void onJoinedRoom(int statusCode, Room room) {
            Log.d(TAG, "onJoinedRoom(" + statusCode + ", " + room + ")");
            if (statusCode != GamesCallbackStatusCodes.OK) {
                Log.e(TAG, "*** Error: onJoinedRoom, status " + statusCode);
                Snackbar.make(menuActivity.findViewById(android.R.id.content), "Room join error: " + getStatusString(statusCode) + ". Try again.", Snackbar.LENGTH_LONG).show();
                menuActivity.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
                leaveRoom();
            }
            else {
                mWaiting = true;
                showWaitingRoom(room);
                menuActivity.setMultiplayer();
            }
        }

        @Override
        public void onRoomCreated(int statusCode, Room room) {
            Log.d(TAG, "onRoomCreated(" + statusCode + ")");
            if (statusCode != GamesCallbackStatusCodes.OK) {
                Snackbar.make(menuActivity.findViewById(android.R.id.content), "Room create error: " + getStatusString(statusCode) + ". Try again.", Snackbar.LENGTH_LONG).show();
                menuActivity.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
                menuActivity.setSinglePlayer();
            }
            else
            {
                mRoomId = room.getRoomId();
                mRoom = room;
                showWaitingRoom(room);
            }
        }

        @Override
        public void onRoomConnected(int statusCode, Room room) {
            Log.d(TAG, "onRoomConnected(" + statusCode + ")");
            if (statusCode != GamesCallbackStatusCodes.OK) {
                Snackbar.make(menuActivity.findViewById(android.R.id.content), "Room connect error: " + getStatusString(statusCode) + ". Try again.", Snackbar.LENGTH_LONG).show();
                menuActivity.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
                leaveRoom();
            } else {
                updateRoom(room);
                negotiateColor(room);
            }
        }
    };

    void updateRoom(Room room) {
        mParticipants = room.getParticipants();
    }

    private RoomStatusUpdateCallback mRoomStatusUpdateCallback = new RoomStatusUpdateCallback() {
        // Called when we are connected to the room. We're not ready to play yet! (maybe not everybody
        // is connected yet).

        @Override
        public void onDisconnectedFromRoom(Room room) {
            Log.d(TAG, "onDisconnectedFromRoom.");
            mRoomId = null;
        }

        @Override
        public void onRoomAutoMatching(Room room) {
            updateRoom(room);
        }

        public void onConnectedToRoom(Room room) {
            Log.d(TAG, "onConnectedToRoom.");

            //get participants and my ID:
            mParticipants = room.getParticipants();
            mMyId = room.getParticipantId(mPlayerId);
            mRoom = room;

            // save room ID if its not initialized in onRoomCreated() so we can leave cleanly before the game starts.
            if (mRoomId == null) {
                mRoomId = room.getRoomId();
            }

            mAchievementsClient = Games.getAchievementsClient(menuActivity, mSignedInAccount);

            // print out the list of participants (for debug purposes)
            Log.d(TAG, "Room ID: " + mRoomId);
            Log.d(TAG, "My ID " + mMyId);
            Log.d(TAG, "<< CONNECTED TO ROOM>>");
        }

        @Override
        public void onRoomConnecting(Room room) {
            Log.e(TAG, "onRoomConnecting to: " + mRoomId);
            updateRoom(room);
        }

        @Override
        public void onPeerDeclined(Room room, List<String> arg1) {
            Log.d(TAG, "onPeerDeclined to: " + mRoomId);

            if (arg1.size() >= mParticipants.size()) {
                leaveRoom();
            } else {
                updateRoom(room);
            }
        }

        @Override
        public void onPeerInvitedToRoom(Room room, List<String> arg1) {
            updateRoom(room);
        }

        @Override
        public void onP2PDisconnected(String participant) {
            Log.e("eskil", "onP2PDisconnected");
        }

        @Override
        public void onP2PConnected(String participant) {
            Log.e("eskil", "onP2PConnected");
        }

        @Override
        public void onPeerJoined(Room room, List<String> arg1) {
            updateRoom(room);
        }

        @Override
        public void onPeerLeft(Room room, List<String> peersWhoLeft) {
            Log.d(TAG, "onPeerLeft: " + mRoomId);
           if (peersWhoLeft.size() >= mParticipants.size()) {
                leaveRoom();
            } else {
                updateRoom(room);
            }
            menuActivity.notifyPeerLeft(peersWhoLeft.get(0));
        }

        @Override
        public void onPeersConnected(Room room, List<String> peers) {
            Log.d(TAG, "onPeersConnected to: " + mRoomId);
            updateRoom(room);
        }

        @Override
        public void onPeersDisconnected(Room room, List<String> peers) {
            Log.d(TAG, "onPeersDisconnected: " + mRoomId);
            if (peers.size() == mParticipants.size()) {
                leaveRoom();
            } else {
                updateRoom(room);
            }
        }
    };

    private void onConnected(GoogleSignInAccount googleSignInAccount) {
        Log.d(TAG, "onConnected(): connected to Google APIs");
        if (mSignedInAccount != googleSignInAccount) {

            mSignedInAccount = googleSignInAccount;

            // update the clients
            mRealTimeMultiplayerClient = Games.getRealTimeMultiplayerClient(menuActivity, googleSignInAccount);
            mInvitationsClient = Games.getInvitationsClient(menuActivity, googleSignInAccount);

            // get the playerId from the PlayersClient
            PlayersClient playersClient = Games.getPlayersClient(menuActivity, googleSignInAccount);
            playersClient.getCurrentPlayer()
                    .addOnSuccessListener(new OnSuccessListener<Player>() {
                        @Override
                        public void onSuccess(Player player) {
                            mPlayerId = player.getPlayerId();
                        }
                    })
                    .addOnFailureListener(createFailureListener("There was a problem getting the player id!"));
        }
        // register listener so we are notified if we receive an invitation to play
        // while we are in the game
        mInvitationsClient.registerInvitationCallback(mInvitationCallback);

        // get the invitation from the connection hint
        // Retrieve the TurnBasedMatch from the connectionHint
        GamesClient gamesClient = Games.getGamesClient(menuActivity, googleSignInAccount);
        gamesClient.getActivationHint()
                .addOnSuccessListener(new OnSuccessListener<Bundle>() {
                    @Override
                    public void onSuccess(Bundle hint) {
                        if (hint != null) {
                            Invitation invitation =
                                    hint.getParcelable(Multiplayer.EXTRA_INVITATION);

                            if (invitation != null && invitation.getInvitationId() != null) {
                                // retrieve and cache the invitation ID
                                Log.d(TAG, "onConnected: connection hint has a room invite!");
                                acceptInviteToRoom(invitation.getInvitationId());
                                menuActivity.getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
                            }
                        }
                    }
                })
                .addOnFailureListener(createFailureListener("There was a problem getting the activation hint!"));
    }


    public void onDisconnected() {
        Log.d(TAG, "onDisconnected()");
        if (mInvitationsClient != null) {
            mInvitationsClient.unregisterInvitationCallback(mInvitationCallback);
        }
        mRealTimeMultiplayerClient = null;
        mInvitationsClient = null;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setRetainInstance(true);
    }

    @Override
    public void onDestroy() {
        Log.e("eskil", "onDestroy: GameActivity");
        leaveRoom();
        signOut();
        super.onDestroy();
    }

    public Boolean selectOpponentsIntent() {
        if (mRoomId != null) return false;

        mRoom = null;
        mRealTimeMultiplayerClient.getSelectOpponentsIntent(1, 1).addOnSuccessListener(
                new OnSuccessListener<Intent>() {
                    @Override
                    public void onSuccess(Intent intent) {
                        startActivityForResult(intent, RC_SELECT_PLAYERS);
                    }
                }
        ).addOnFailureListener(createFailureListener("There was a problem selecting opponents."));
        return true;
    }

    public Boolean gameStarted() {
        if (null == mParticipants) {
            return false;
        }
        return true;
    }

    RealTimeMultiplayerClient.ReliableMessageSentCallback mReliableMessageSent = new RealTimeMultiplayerClient.ReliableMessageSentCallback() {
        @Override
        public void onRealTimeMessageSent(int statusCode, int tokenId, String recipientParticipantId) {
                                    /*case GamesStatusCodes.STATUS_REAL_TIME_ROOM_NOT_JOINED: {
                                        disableFab(recipientParticipantId);
                                        mParticipants = mRoom.getParticipants();
                                        Log.e(TAG, "onRealTimeMessageSent: STATUS_REAL_TIME_ROOM_NOT_JOINED. Disabling participant: " + getDisplayName(recipientParticipantId));
                                        break;
                                    }*/
            Log.d("eskil", "RealtimeMessageSent statuscode:" + statusCode);
        }
    };

    @Override public void onAttach(Activity activity) {
        super.onAttach(activity);
        menuActivity = (game_menu)activity;
        startSignInIntent();
    }

    public void startSignInIntent() {
        startActivityForResult(mGoogleSignInClient.getSignInIntent(), RC_SIGN_IN);
    }

    // Handle the result of the "Select players UI" we launched when the user clicked the
    // "Invite friends" button. We react by creating a room with those players.
    private void handleSelectPlayersResult(int response, Intent data) {
        if (response != Activity.RESULT_OK) {
            Log.w(TAG, "*** select players UI cancelled, " + response);
            return;
        }

        Log.d(TAG, "Select players UI succeeded.");

        // get the invitee list
        final ArrayList<String> invitees = data.getStringArrayListExtra(Games.EXTRA_PLAYER_IDS);
        Log.d(TAG, "Invitee count: " + invitees.size());

        // get the automatch criteria
        Bundle autoMatchCriteria = null;
        int minAutoMatchPlayers = data.getIntExtra(Multiplayer.EXTRA_MIN_AUTOMATCH_PLAYERS, 0);
        int maxAutoMatchPlayers = data.getIntExtra(Multiplayer.EXTRA_MAX_AUTOMATCH_PLAYERS, 0);
        if (minAutoMatchPlayers > 0 || maxAutoMatchPlayers > 0) {
            autoMatchCriteria = RoomConfig.createAutoMatchCriteria(
                    minAutoMatchPlayers, maxAutoMatchPlayers, 0);
            Log.d(TAG, "Automatch criteria: " + autoMatchCriteria);
        }

        // create the room
        Log.d(TAG, "Creating room...");

        mRoomConfig = RoomConfig.builder(mRoomUpdateCallback)
                .addPlayersToInvite(invitees)
                .setOnMessageReceivedListener(mOnRealTimeMessageReceivedListener)
                .setRoomStatusUpdateCallback(mRoomStatusUpdateCallback).build();
        mRealTimeMultiplayerClient.create(mRoomConfig)
            .addOnFailureListener(createFailureListener("There was a problem creating the waiting room!"))
            .addOnSuccessListener(new OnSuccessListener<Void>() {
                                      @Override
                                      public void onSuccess(Void aVoid) {
                                          Log.e(TAG, "Room created successfully?");
                                      }
                                  })
                .addOnCanceledListener(new OnCanceledListener() {
                                           @Override
                                           public void onCanceled() {
                                               Log.e(TAG,"Room cancelled?");
                                           }
                                       });


        Log.d(TAG, "Room created, waiting for it to be ready...");

        // prevent screen from sleeping during handshake
        menuActivity.getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        menuActivity.setMultiplayer();

        mWaiting = true;
    }

    // Handle the result of the invitation inbox UI, where the player can pick an invitation
    // to accept. We react by accepting the selected invitation, if any.
    private void handleInvitationInboxResult(int response, Intent data) {
        if (response != Activity.RESULT_OK) {
            Log.w(TAG, "*** invitation inbox UI cancelled, " + response);
            return;
        }

        Log.d(TAG, "Invitation inbox UI succeeded.");
        Invitation invitation = data.getExtras().getParcelable(Multiplayer.EXTRA_INVITATION);

        // accept invitation
        if (invitation != null) {
            acceptInviteToRoom(invitation.getInvitationId());
        }

    }

    @Override
    public void onActivityResult(int requestCode, int responseCode,
                                 Intent intent) {

        if (requestCode == RC_SIGN_IN) {

            Task<GoogleSignInAccount> task =
                    GoogleSignIn.getSignedInAccountFromIntent(intent);

            try {
                GoogleSignInAccount account = task.getResult(ApiException.class);
                onConnected(account);
            } catch (ApiException apiException) {
                String message = apiException.getMessage();
                if (message == null || message.isEmpty()) {
                    message = "Signin other error";
                }
                Log.e("eskil", message);
                onDisconnected();
            }
        } else if (requestCode == RC_SELECT_PLAYERS) {
            // we got the result from the "select players" UI -- ready to create the room
            handleSelectPlayersResult(responseCode, intent);

        } else if (requestCode == RC_INVITATION_INBOX) {
            // we got the result from the "select invitation" UI (invitation inbox). We're
            // ready to accept the selected invitation:
            handleInvitationInboxResult(responseCode, intent);

        } else if (requestCode == RC_WAITING_ROOM) {
            // we got the result from the "waiting room" UI.
            if (responseCode == Activity.RESULT_OK) {
                // ready to start playing
                Log.d(TAG, "Starting game (waiting room returned OK).");
            } else if (responseCode == GamesActivityResultCodes.RESULT_LEFT_ROOM) {
                // player indicated that they want to leave the room
                leaveRoom();
            } else if (responseCode == Activity.RESULT_CANCELED) {
                // Dialog was cancelled (user pressed back key, for instance). In our game,
                // this means leaving the room too. In more elaborate games, this could mean
                // something else (like minimizing the waiting room UI).
                leaveRoom();
            }
            mWaiting = false;
            mInWaitingRoom = false;
        }
        super.onActivityResult(requestCode, responseCode, intent);
    }

    int myNegotiateValue = 0;
    int opponentNegotiateValue = 0;
    boolean negotiateStarted = false;
    boolean negotiateDone = false;

    public void negotiateColor(Room room) {
        Random r = new Random();
        myNegotiateValue = r.nextInt(100);
        negotiateStarted = true;
        sendMsgToOpponent(room, new String("Negotiate: " + myNegotiateValue));
    }

    public boolean isNegotiateDone() {
        return negotiateDone;
    }

    public boolean isOpponentWhite() {
        return opponentNegotiateValue < myNegotiateValue;
    }

    // Called from native to send a move
    public void performMove(int from_x, int from_y, int to_x, int to_y) {
        sendMsgToOpponent(getRoom(), new String("Move: " + from_x + "," + from_y + "," + to_x + "," + to_y));
    }

    public void performAchievementWon(int opponent_is_cpu, int opponent_is_external, int opponent_is_local, int player_is_white) {
        String opponent_string = "opponent: ";
        String color_string = "color: ";
        if (opponent_is_cpu > 0) {
            mAchievementsClient.unlock("CgkI0uiXj9cREAIQAg");
            opponent_string = "cpu";
        } else if (opponent_is_external > 0) {
            mAchievementsClient.unlock("CgkI0uiXj9cREAIQAw");
            opponent_string = "external player";
        } else if (opponent_is_local > 0) {
            mAchievementsClient.unlock("CgkI0uiXj9cREAIQBA");
            opponent_string = "local player";
        }
        if (player_is_white > 0) {
            mAchievementsClient.unlock( "CgkI0uiXj9cREAIQBg");
            color_string = "white";
        } else {
            mAchievementsClient.unlock("CgkI0uiXj9cREAIQBQ");
            color_string = "black";
        }

        Log.d(TAG, "Achievement: opponent_is_cpu: " + opponent_is_cpu + " opponent_is_external:" + opponent_is_external +
            " opponent_is_local:" + opponent_is_local + " player_is_white:" + player_is_white);

        menuActivity.notifyGameWon("Game won against " + opponent_string + " with " + color_string);
    }

    public void performAcceptNewGame() {
        String msg = "Accept: new";
        sendMsgToOpponent(getRoom(), msg);
    }

    public void performStart(boolean continue_game) {
        String msg = "Start: ";
        if (continue_game) {
            msg += "continue";
        } else {
            msg += "new";
        }
        sendMsgToOpponent(getRoom(), msg);
    }

    public Room getRoom() {
        return mRoom;
    }


    public void leaveRoom() {
        Log.d(TAG, "Leaving room " + mRoomId);
        if (mRoomId != null ) {
            menuActivity.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

            mRealTimeMultiplayerClient.leave(mRoomConfig, mRoomId)
                    .addOnCompleteListener(new OnCompleteListener<Void>() {
                        @Override
                        public void onComplete(@NonNull Task<Void> task) {
                            Log.d(TAG, "Left room mRoomId");
                            mRoomId = null;
                            mRoomConfig = null;
                        }
                    });
        }
    }

    public void sendMsgToOpponent(Room room, String posText) {
        if (room == null) {
            return;
        }
        String roomId = room.getRoomId();

        Log.d(TAG,"Sending message: " + posText);
        byte[] message = posText.getBytes();
        for (Participant p : room.getParticipants()) {
            if (!p.getParticipantId().equals(mMyId)) {
                mRealTimeMultiplayerClient.sendReliableMessage(message, roomId, p.getParticipantId(), mReliableMessageSent);
            }
        }
    }

}
