package com.techniccontroller.bluetoothremoteledmatrix;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.inputmethod.EditorInfo;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;
import java.util.UUID;

public class MainActivity extends AppCompatActivity {

    Handler h;
    private ConnectedThread mConnectedThread;
    final int RECIEVE_MESSAGE = 1;

    // UUID fuer Kommunikation mit Seriellen Modulen
    private UUID uuid = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
    private static final String LOG_TAG = "LEDmatrix";

    // Variablen
    private BluetoothAdapter adapter = null;
    private BluetoothSocket socket = null;
    private OutputStream stream_out = null;
    private InputStream stream_in = null;
    private boolean is_connected = false;
    private static String mac_adresse; // MAC Adresse des Bluetooth Adapters

    //Speicherwoerter
    static final String STATE_Verbindung = "verbindung";
    static final String STATE_DEVICE = "btdevice";
    private SharedPreferences preferences;

    private Spinner spinner;

    private ArrayAdapter<String> dataAdapter;

    // Befehltags
    private String keyBT = "new";


    private ArrayList<Bluetoothgeraet> devices;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getSupportActionBar().setDisplayShowHomeEnabled(true);
        getSupportActionBar().setIcon(R.mipmap.icon_ledmatrix);
        getSupportActionBar().setDisplayUseLogoEnabled(false);

        setContentView(R.layout.activity_main);
        Log.d(LOG_TAG, "Bluetest: OnCreate");

        preferences = getSharedPreferences("SavedValues", MODE_PRIVATE);

        ((EditText)findViewById(R.id.editText)).setOnEditorActionListener(new TextView.OnEditorActionListener() {
                @Override
                public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
                    boolean handled = false;
                        if (actionId == EditorInfo.IME_ACTION_SEND) {
                                sendenMsg(null);
                                handled = true;
                        }
                        return handled;
                    }
        });


        // Verbindung mit Bluetooth-Adapter herstellen
        adapter = BluetoothAdapter.getDefaultAdapter();
        if (adapter == null || !adapter.isEnabled()) {
            Toast.makeText(this, "Bitte Bluetooth aktivieren",
                    Toast.LENGTH_LONG).show();
            Log.d(LOG_TAG,
                    "onCreate: Bluetooth Fehler: Deaktiviert oder nicht vorhanden");
            finish();
            return;
        } else
            Log.d(LOG_TAG, "onCreate: Bluetooth-Adapter ist bereit");

        spinner = (Spinner) findViewById(R.id.spinner);

        List<String> devicenames = new ArrayList<String>();

        devices = new ArrayList<Bluetoothgeraet>();

        Set<BluetoothDevice> pairedDevices = adapter.getBondedDevices();
        if (pairedDevices.size() > 0) {
            for (BluetoothDevice dev : pairedDevices) {
                devices.add(new Bluetoothgeraet(dev.getName(), dev.getAddress()));
                devicenames.add(dev.getName());
                Log.d(LOG_TAG, dev.getName() + " - " + dev.getAddress());
            }
        }


        dataAdapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_dropdown_item, devicenames);
        dataAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        spinner.setAdapter(dataAdapter);

        h = new Handler() {
            public void handleMessage(android.os.Message msg) {
                switch (msg.what) {
                    case RECIEVE_MESSAGE:
                        break;
                }
            }
        };
    }

    public void verbinden(View v) {
        SharedPreferences.Editor editor = preferences.edit();
        editor.clear();
        editor.putInt(STATE_DEVICE,(int) spinner.getSelectedItemId());
        editor.commit();
        mac_adresse = devices.get((int)spinner.getSelectedItemId()).getAdresse();
        Log.d(LOG_TAG, "Verbinde mit " + mac_adresse);

        BluetoothDevice remote_device = adapter.getRemoteDevice(mac_adresse);

        // Socket erstellen
        try {
            socket = remote_device
                    .createRfcommSocketToServiceRecord(uuid);
            Log.d(LOG_TAG, "Socket erstellt");
        } catch (Exception e) {
            Log.e(LOG_TAG, "Socket Erstellung fehlgeschlagen: " + e.toString());
        }

        adapter.cancelDiscovery();

        // Socket verbinden
        try {
            socket.connect();
            Log.d(LOG_TAG, "Socket verbunden");
            is_connected = true;
        } catch (IOException e) {
            is_connected = false;
            Log.e(LOG_TAG, "Socket kann nicht verbinden: " + e.toString());
        }

        // Socket beenden, falls nicht verbunden werden konnte
        if (!is_connected) {
            try {
                socket.close();
            } catch (Exception e) {
                Log.e(LOG_TAG,
                        "Socket kann nicht beendet werden: " + e.toString());
            }
        }

        // Outputstream erstellen:
        try {
            stream_out = socket.getOutputStream();
            Log.d(LOG_TAG, "OutputStream erstellt");
        } catch (IOException e) {
            Log.e(LOG_TAG, "OutputStream Fehler: " + e.toString());
            is_connected = false;
        }

        // Inputstream erstellen
        try {
            stream_in = socket.getInputStream();
            Log.d(LOG_TAG, "InputStream erstellt");
        } catch (IOException e) {
            Log.e(LOG_TAG, "InputStream Fehler: " + e.toString());
            is_connected = false;
        }

        if (is_connected) {
            Toast.makeText(this, "Verbunden mit " + mac_adresse,
                    Toast.LENGTH_LONG).show();
            (findViewById(R.id.bt_verbinden))
                    .setBackgroundColor(Color.GREEN);
        } else {
            Toast.makeText(this, "Verbindungsfehler mit " + mac_adresse,
                    Toast.LENGTH_LONG).show();
            (findViewById(R.id.bt_verbinden))
                    .setBackgroundColor(Color.RED);
        }

        onResume();
    }

    public void sendenMsg(View view){
        senden((keyBT + ((EditText)findViewById(R.id.editText)).getText().toString()).getBytes());
        ((EditText)findViewById(R.id.editText)).setText("");
    }


    public void beenden(View view){
        Log.d(LOG_TAG, "App wird beendet. Trenne Verbindung, falls vorhanden");
        onDestroy();
        System.exit(0);
    }

    public void senden(byte code) {
        if (is_connected) {
            Log.d(LOG_TAG, "Sende Nachricht: " + code);
            try {
                stream_out.write(code);
                //stream_out.write(code);
                //stream_out.write(code);
            } catch (IOException e) {
                Log.e(LOG_TAG,
                        "Bluetest: Exception bei "+ code + " :  " + e.toString());
            }
        }
    }

    public void senden(byte[] code) {
        if (is_connected) {
            for(int i = 0; i < code.length; i++)
                Log.d(LOG_TAG, "Sende Nachrichten: " + code[i]);
            try {
                stream_out.write(code);
                //stream_out.write(code);
                //stream_out.write(code);
            } catch (IOException e) {
                Log.e(LOG_TAG,
                        "Bluetest: Exception bei "+ code + " :  " + e.toString());
            }
        }
    }

    public void empfangen(View v) {
        byte[] buffer = new byte[1024]; // Puffer
        int laenge; // Anzahl empf. Bytes
        String msg = "";
        try {
            if (stream_in.available() > 0) {
                laenge = stream_in.read(buffer);
                Log.d(LOG_TAG,
                        "Anzahl empfangender Bytes: " + String.valueOf(laenge));

                // Message zusammensetzen:
                for (int i = 0; i < laenge; i++)
                    msg += (char) buffer[i];

                Log.d(LOG_TAG, "Message: " + msg);
                Toast.makeText(this, msg, Toast.LENGTH_LONG).show();

            } else
                Toast.makeText(this, "Nichts empfangen", Toast.LENGTH_LONG)
                        .show();
        } catch (Exception e) {
            Log.e(LOG_TAG, "Fehler beim Empfangen: " + e.toString());
        }
    }

    public void trennen(View v) {
        if (is_connected && stream_out != null) {
            is_connected = false;
            if(findViewById(R.id.bt_verbinden) != null)(findViewById(R.id.bt_verbinden)).setBackgroundColor(Color.RED);
            Log.d(LOG_TAG, "Trennen: Beende Verbindung");
            try {
                stream_out.flush();
                socket.close();
            } catch (IOException e) {
                Log.e(LOG_TAG,
                        "Fehler beim beenden des Streams und schliessen des Sockets: "
                                + e.toString());
            }
        } else{
            Log.d(LOG_TAG, "Trennen: Keine Verbindung zum beenden");
        }

        SharedPreferences.Editor editor = preferences.edit();
        editor.putBoolean(STATE_Verbindung, false);
        editor.commit();

        mConnectedThread = null;
    }

    public int lenINT(int i){
        int c = 1;
        while ((i /= 10) != 0)c++;
        return c;
    }

    public void select(View v){
            v.setBackgroundColor(Color.BLACK);
    }

    @Override
    public void onPause(){
        super.onPause();
        Log.d(LOG_TAG, "Pause");
        trennen(null);
        SharedPreferences.Editor editor = preferences.edit();
        editor.putBoolean(STATE_Verbindung, is_connected);
        editor.commit();
    }

    @Override
    public void onResume(){
        super.onResume();
        Log.d(LOG_TAG, "Resume");

        if(socket != null){
                try {

                    mConnectedThread = new ConnectedThread(socket);
                    mConnectedThread.start();

                } catch (IOException e) {
                    e.printStackTrace();
                }
        }

    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Log.d(LOG_TAG, "onDestroy. Trenne Verbindung, falls vorhanden");
        trennen(null);
    }

    private class ConnectedThread extends Thread {
        private final InputStream mmInStream;
        //private final OutputStream mmOutStream;

        public ConnectedThread(BluetoothSocket socket) throws IOException {
            InputStream tmpIn = null;
            //OutputStream tmpOut = null;

            // Get the input and output streams, using temp objects because
            // member streams are final
            try {
                tmpIn = socket.getInputStream();
                //tmpOut = socket.getOutputStream();
            } catch (IOException e) {
                e.printStackTrace();
            }

            mmInStream = tmpIn;
            //mmOutStream = tmpOut;
        }

        public void run() {
            char in = '0';
            String inLine = "";
            // Keep listening to the InputStream until an exception occurs
            while (is_connected) {
                while(in != '\n'){
                    try {
                        // Read from the InputStream
                        in = Character.toChars(mmInStream.read())[0];
                        inLine = inLine + String.valueOf(in);
                    } catch (IOException e) {
                        break;
                    }
                }
                if(inLine.length() > 0){
                    h.obtainMessage(RECIEVE_MESSAGE, inLine).sendToTarget();
                    inLine = "";
                    in = '0';
                }
            }
        }
    }
}
