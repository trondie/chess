package com.eskilsund.etchess3d.etchess3d;

import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.support.design.widget.Snackbar;
import android.support.v4.app.Fragment;
import android.support.v7.app.AlertDialog;
import android.util.Log;
import android.view.KeyEvent;
import android.widget.CheckBox;
import android.widget.NumberPicker;
import android.support.design.widget.FloatingActionButton;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.View;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.ToggleButton;

import com.google.android.gms.auth.api.signin.GoogleSignIn;
import com.google.android.gms.auth.api.signin.GoogleSignInOptions;

import java.io.File;

public class game_menu extends AppCompatActivity
        implements KeyEvent.Callback {
    NativeChessActivity chess = null;
    GameActivity gameActivity = null;
    AboutActivity aboutActivity = null;

    final String TAG = "eskil";

    ToggleButton tb_white = null;
    boolean white_cpu = false;

    ToggleButton tb_black = null;
    boolean black_cpu = true;

    ToggleButton tb_load = null;
    boolean load_game = true;

    ToggleButton tb_difficulty = null;
    boolean difficulty_low = false;
    boolean difficulty_medium = true;
    boolean difficulty_high = false;
    int difficulty_value = 4;

    FloatingActionButton fab = null;
    FloatingActionButton fabMultiplayer = null;
    boolean multiplayer = false;
    Intent nativeIntent = null;
    Snackbar sbAccept = null;

    public void newGameAccepted() {
        if (sbAccept != null) {
            sbAccept.dismiss();
        }
        fab.setEnabled(true);
        fab.show();
        startGame(false);
    }

    public void askToStartGame(boolean continue_game)
    {
        String start_string;
        final boolean accepted_continue_game = continue_game;
        if (continue_game) {
            start_string = "continue on-going game";
        } else {
            start_string = "start a new game";
        }
        chess.finishMe();
        final Snackbar sb;
        sb = Snackbar.make(findViewById(android.R.id.content), "Opponent wants to " + start_string, Snackbar.LENGTH_INDEFINITE);
        sb.setAction("Accept", new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        if (!accepted_continue_game) {
                            gameActivity.performAcceptNewGame();
                        }
                        startGame(accepted_continue_game);
                        sb.dismiss();
                    }
                });
        sb.show();
    }

    public void startGame(boolean continue_game) {
        //Delete save if !continue
        if (!continue_game) {
            File dir = getFilesDir();
            File file = new File(dir, "../save.xml");
            boolean deleted = file.delete();
            Log.e("etchess3d", "path:" + file.getAbsolutePath() + " deleted: " + deleted);
        }

        //Set instance so native know where to send moves
        chess.setNativeGameActivity(gameActivity);

        //Launch native game
        nativeIntent = new Intent(getBaseContext(), chess.getClass());
        startActivity(nativeIntent);
        not_started = false;
        tb_load.setEnabled(true);
        tb_load.setText("Continue previous game");
        load_game = true;
    }

    public void showInvitation(String inviter, final String invitationId) {
        AlertDialog alertbox = new AlertDialog.Builder(this)
                .setMessage("Chess invitation from: " + inviter)
                .setPositiveButton("Accept", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface arg0, int arg1) {
                        gameActivity.respondToInvitation(true, invitationId);
                    }
                })
                .setNegativeButton("Reject", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface arg0, int arg1) {
                        gameActivity.respondToInvitation(false, invitationId);
                    }
                })
                .show();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_game_menu);

        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        chess = new NativeChessActivity();
        aboutActivity = new AboutActivity();

        gameActivity = (GameActivity)getSupportFragmentManager().findFragmentByTag("GameActivity");
        if (gameActivity == null) {
            Log.d(TAG, this + ": Existing fragment not found.!!!");
            gameActivity = new GameActivity();
            getSupportFragmentManager().beginTransaction().add((Fragment)gameActivity, "GameActivity").commit();
            gameActivity.setGoogleSignInClient(GoogleSignIn.getClient(this, GoogleSignInOptions.DEFAULT_GAMES_SIGN_IN));
        }

        tb_load = (ToggleButton) findViewById(R.id.load_game);
        tb_load.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                load_game = !load_game;
                if (load_game) tb_load.setText("Continue previous game");
                else tb_load.setText("Start new game");
            }
        });

        tb_white = (ToggleButton) findViewById(R.id.cpu_white);
        tb_white.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                white_cpu = !white_cpu;
                if (white_cpu) tb_white.setText("CPU is white");
                else tb_white.setText("Player is white");
            }
        });

        tb_black = (ToggleButton) findViewById(R.id.cpu_black);
        tb_black.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                black_cpu = !black_cpu;
                if (black_cpu) tb_black.setText("CPU is black");
                else tb_black.setText("Player is black");
            }
        });

        tb_difficulty = (ToggleButton) findViewById(R.id.cpu_difficulty);
        tb_difficulty.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (difficulty_low) {
                    difficulty_low = false;
                    difficulty_medium = true;
                    tb_difficulty.setText("CPU difficulty medium");
                    difficulty_value = 3;
                } else if (difficulty_medium) {
                    difficulty_medium = false;
                    difficulty_high = true;
                    tb_difficulty.setText("CPU difficulty high");
                    difficulty_value = 4;
                } else if (difficulty_high) {
                    difficulty_high = false;
                    difficulty_low = true;
                    tb_difficulty.setText("CPU difficulty low");
                    difficulty_value = 5;
                }
            }
        });

        fab = (FloatingActionButton) findViewById(R.id.fab);
        fab.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {

                if (multiplayer) {
                    //Game settings negotiated so just send start command
                    gameActivity.performStart(load_game);
                    if (!load_game) {
                        fab.setEnabled(false);
                        fab.hide();
                        sbAccept = Snackbar.make(view, "Awaiting acceptance for new game", Snackbar.LENGTH_INDEFINITE);
                        sbAccept.show();
                        return; //Await acceptance for new game
                    }
                } else {
                    //Read out user defined game settings
                    if (white_cpu) {
                        chess.whiteIsCpu();
                    } else {
                        chess.whiteIsPlayer();
                    }
                    if (black_cpu) {
                        chess.blackIsCpu();
                    } else {
                        chess.blackIsPlayer();
                    }
                    chess.setSearchDepth(difficulty_value);
                }

                startGame(load_game);

                //We don't want to start over so reset
                if (multiplayer) {
                    load_game = true;
                    tb_load.setText("Continue previous game");
                }
            }
        });

        fabMultiplayer = (FloatingActionButton) findViewById(R.id.fabMultiplayer);
        fabMultiplayer.setOnClickListener(new View.OnClickListener() {
               @Override
               public void onClick(View view) {
                   if (multiplayer) {
                       gameActivity.leaveRoom(); // user want to setup new game
                   }

                   if (gameActivity.isConnected()) {
                       if (!gameActivity.selectOpponentsIntent()) {
                           Snackbar.make(view, "Connected. Ready to play", Snackbar.LENGTH_LONG)
                                   .show();
                           setMultiplayer();
                           Log.e(TAG, "Connected.");
                       }
                   } else {
                       Snackbar.make(view, "Not connected. Can't share", Snackbar.LENGTH_LONG)
                               .show();
                       Log.e(TAG, "Not connected.");
                   }
               }
       });
    }

    public void notifyPeerLeft(String peername) {
        chess.finishMe();
        Snackbar.make(findViewById(android.R.id.content), "Peer left: " + peername, Snackbar.LENGTH_LONG)
                .show();
        setSinglePlayer();
    }

    public void notifyGameWon(String wonstring) {
        chess.finishMe();
        final Snackbar sb;
        sb = Snackbar.make(findViewById(android.R.id.content), wonstring, Snackbar.LENGTH_INDEFINITE);
        sb.setAction("Leave game", new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        setSinglePlayer();
                        sb.dismiss();
                    }
                });
        sb.show();
    }

    public void nativeMove(int from_x, int from_y, int to_x, int to_y) {
        chess.nativePerformMove(from_x, from_y, to_x, to_y);
    }

    boolean not_started = true;
    Snackbar sbMultiplayer = null;

    public void setMultiplayer() {
        not_started = true;
        load_game = false;
        multiplayer = true;
        tb_white.setEnabled(false);
        tb_black.setEnabled(false);
        tb_load.setEnabled(false);
        tb_difficulty.setEnabled(false);
        fab.setEnabled(false);
        fab.hide();
        if (sbMultiplayer != null) {
            sbMultiplayer.dismiss();
            sbMultiplayer = null;
        }
        sbMultiplayer = Snackbar.make(findViewById(android.R.id.content), "Awaiting opponent.", Snackbar.LENGTH_INDEFINITE);
        sbMultiplayer.setAction("Cancel", new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                setSinglePlayer();
                sbMultiplayer.dismiss();
                gameActivity.rejectInviteToRoom();
            }
        });
        sbMultiplayer.show();
    }

    public void setMultiplayerColor(boolean opponentWhite) {
        String color;
        if (opponentWhite) {
            tb_white.setText("Player external white");
            tb_black.setText("Player black");
            chess.whiteIsExternalPlayer();
            chess.blackIsPlayer();
            color = "black";
        } else {
            tb_white.setText("Player white");
            tb_black.setText("Player external black");
            chess.whiteIsPlayer();
            chess.blackIsExternalPlayer();
            color = "white";
            fab.setEnabled(true);
            fab.show();
        }

        if (sbMultiplayer != null) {
            sbMultiplayer.dismiss();
            sbMultiplayer = null;
        }
        Snackbar.make(findViewById(android.R.id.content), "Negotiated my color " + color, Snackbar.LENGTH_LONG)
                .show();
    }

    public void setSinglePlayer() {
        multiplayer = false;
        if (white_cpu) {
            tb_white.setText("CPU is white");
        } else {
            tb_white.setText("Player is white");
        }
        if (black_cpu) {
            tb_black.setText("CPU is black");
        } else {
            tb_black.setText("Player is black");
        }

        tb_white.setEnabled(true);
        tb_black.setEnabled(true);
        tb_load.setEnabled(true);
        tb_difficulty.setEnabled(true);
        fab.setEnabled(true);
        fab.show();
        gameActivity.leaveRoom();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_game_menu, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_about) {
            Intent intent = new Intent(this.getBaseContext(), aboutActivity.getClass());
            startActivity(intent);
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            exitByBackKey();

            //moveTaskToBack(false);

            return true;
        }
        return super.onKeyDown(keyCode, event);
    }

    protected void exitByBackKey() {

        AlertDialog alertbox = new AlertDialog.Builder(this)
                .setMessage("Do you want to exit application?")
                .setPositiveButton("Yes", new DialogInterface.OnClickListener() {

                    public void onClick(DialogInterface arg0, int arg1) {
                        gameActivity.leaveRoom();
                        finish();
                    }
                })
                .setNegativeButton("No", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface arg0, int arg1) {
                    }
                })
                .show();

    }
}
