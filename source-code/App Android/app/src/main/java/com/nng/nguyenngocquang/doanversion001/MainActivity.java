package com.nng.nguyenngocquang.doanversion001;

import androidx.appcompat.app.AppCompatActivity;
import androidx.constraintlayout.widget.ConstraintLayout;

import android.app.Dialog;
import android.app.TimePickerDialog;
import android.content.Intent;
import android.content.SharedPreferences;
import android.media.MediaPlayer;
import android.os.Build;
import android.os.Bundle;
import android.view.View;
import android.view.Window;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.TimePicker;
import android.widget.Toast;

import com.android.volley.AuthFailureError;
import com.android.volley.Request;
import com.android.volley.RequestQueue;
import com.android.volley.Response;
import com.android.volley.VolleyError;
import com.android.volley.toolbox.JsonArrayRequest;
import com.android.volley.toolbox.StringRequest;
import com.android.volley.toolbox.Volley;

import org.eclipse.paho.android.service.MqttAndroidClient;
import org.eclipse.paho.client.mqttv3.IMqttActionListener;
import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.IMqttToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class MainActivity extends AppCompatActivity {
    public static boolean securityMQTT = true;
    public static String serverUriMQTT;
    String urlGetTimes;
    String urlInsertTimes;
    String urlUpdateTimes;
    String urlUpdateDelete;

    Intent mainSerVice;

    SharedPreferences sharedPreferences;

    void updateHost(String portServer, String portBroker){
        SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.putString("portServer",portServer);
        editor.putString("portBroker",portBroker);
        editor.commit();

        String host = "http://" + portServer + "/Server/templates/androidwebservice/";
        urlGetTimes = host + "getdata.php";
        urlInsertTimes = host + "insert.php";
        urlUpdateTimes = host + "update.php";
        urlUpdateDelete = host + "delete.php";
        serverUriMQTT = "tcp://" + portBroker;
        getTimes(urlGetTimes);
    }

    public static String usernameMQTT = "lvrikost";
    public static String passwordMQTT = "0kLqEfJE2Day";

    ListView listViewTime;
    List<Time> arrayTime;
    TimeAdapter adapter;

    static MediaPlayer mediaPlayer;
    static MqttAndroidClient client;

    ConstraintLayout constrSearch;
    ConstraintLayout constrAdd;
    ConstraintLayout constrSetting;
    TextView txtTemp;
    TextView txtHumi;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        makeMapping();
        adapter = new TimeAdapter(this, R.layout.itemclock, arrayTime);
        listViewTime.setAdapter(adapter);

        sharedPreferences = getSharedPreferences("dataIP", MODE_PRIVATE);
        updateHost(sharedPreferences.getString("portServer", "192.168.56.1:8080xx"),
                sharedPreferences.getString("portBroker", "postman.cloudmqtt.com:14232xx"));

        settingEventMenuToolbar();
    }

    void connectAndWorkMQTT(){
        if(client != null){
            try {
                client.disconnect();
            }catch (Exception e){

            }
        }

        String clientId = MqttClient.generateClientId();
        client =
                new MqttAndroidClient(this.getApplicationContext(), serverUriMQTT,
                        clientId);
        client.setCallback(new MqttCallback() {
            @Override
            public void connectionLost(Throwable cause) {}

            @Override
            public void messageArrived(String topic, MqttMessage message) throws Exception {
                if("Reminds".equals(topic)){
                    if("Alarm".equals(message.toString())){
                        initMediaPlayer();
                        mediaPlayer.start();
                    }else if("off".equals(message.toString())){
                        mediaPlayer.pause();
                    }
                }else if("Topic".equals(topic)){
                    String content = message.toString();
                    if(content != null){
                        if("off".equals(message.toString())){
                            mediaPlayer.pause();
                        }

                        int len = content.length();
                        if(len > 1){
                            char charFirst = content.charAt(0);
                            int value = -1;
                            boolean checkNuber = true;

                            try {
                                value = Integer.parseInt(content.substring(1));
                            }catch (NumberFormatException ex){
                                checkNuber = false;
                            }

                            if(checkNuber){
                                if(charFirst == 'H' && value >= 0 && value <= 100){
                                    txtHumi.setText(value + "%");
                                }else if(charFirst == 'T' && Math.abs(value) < 1000){
                                    txtTemp.setText(value + "°C");
                                }
                            }
                        }
                    }
                }else if("changeData".equals(topic)){
                    if("y".equals(message.toString())){
                        getTimes(urlGetTimes);
                    }
                }
            }

            @Override
            public void deliveryComplete(IMqttDeliveryToken token) {}
        });

        try {
            IMqttToken token = null;
            if(securityMQTT){
                MqttConnectOptions options = new MqttConnectOptions();
                options.setUserName(usernameMQTT);
                options.setCleanSession(true);
                options.setPassword(passwordMQTT.toCharArray());
                token =  client.connect(options);
            }else{
                token =  client.connect();
            }

            token.setActionCallback(new IMqttActionListener() {
                @Override
                public void onSuccess(IMqttToken asyncActionToken) {
                    String[] topic = new String[]{"Find", "changeData", "Reminds", "Topic"};
                    int[] qos = new int[]{1, 1, 1, 1};
                    try {
                        client.subscribe(topic, qos);
                    } catch (MqttException e) {
                        //Do nothing
                    }
                }

                @Override
                public void onFailure(IMqttToken asyncActionToken, Throwable exception) {
                    Toast.makeText(MainActivity.this, getResources().getString(R.string.toast_error_sub_mqtt), Toast.LENGTH_SHORT).show();
                }
            });
        } catch (MqttException e) {
            Toast.makeText(MainActivity.this, getResources().getString(R.string.toast_error_con_mqtt), Toast.LENGTH_SHORT).show();
            e.printStackTrace();
        }
    }

    void pub(String content){
        String topic = "androidTx";
        String payload = content;
        byte[] encodedPayload = new byte[0];
        try {
            encodedPayload = payload.getBytes("UTF-8");
            MqttMessage message = new MqttMessage(encodedPayload);
            client.publish(topic, message);
        } catch (UnsupportedEncodingException | MqttException e) {
            // Do nothing
        }
    }

    public static void publish(String topic, String content){
        byte[] encodedPayload = new byte[0];
        try {
            encodedPayload = content.getBytes("UTF-8");
            MqttMessage message = new MqttMessage(encodedPayload);
            client.publish(topic, message);
        } catch (UnsupportedEncodingException | MqttException e) {
            // Do nothing
        }
    }

    void settingEventMenuToolbar(){
        constrAdd.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                addTime();
            }
        });

        constrSearch.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                publish("Reminds", "find");
            }
        });

        constrSetting.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(mediaPlayer != null && mediaPlayer.isPlaying()){
                    MainActivity.mediaPlayer.pause();
                    MainActivity.publish("Reminds", "off");
                }else{
                    dialogSetting();
                }
            }
        });
    }

    //========== Update Ip Server and Ip MQTT =============
    void dialogSetting(){
        final Dialog dialog = new Dialog(this);
        dialog.requestWindowFeature(Window.FEATURE_NO_TITLE);
        dialog.setContentView(R.layout.setting);
        dialog.setCanceledOnTouchOutside(false);

        //Mapping
        final EditText edtServer = (EditText) dialog.findViewById(R.id.edtServer);
        final EditText edtBroker = (EditText) dialog.findViewById(R.id.edtBroker);
        Button btnCancel = (Button) dialog.findViewById(R.id.btnCancel);
        Button btnUpdate = (Button) dialog.findViewById(R.id.btnUpdate);

        edtServer.setText(sharedPreferences.getString("portServer", "192.168.56.1:8080xx"));
        edtBroker.setText(sharedPreferences.getString("portBroker", "postman.cloudmqtt.com:14232xx"));
        dialog.show();

        btnCancel.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                dialog.dismiss();
            }
        });

        btnUpdate.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String server = edtServer.getText().toString();
                String broker = edtBroker.getText().toString();

                if(!"".equals(server) && !"".equals(server)){
                    updateHost(server, broker);
                }else{
                    updateHost(sharedPreferences.getString("portServer", "192.168.56.1:8080xx"),
                            sharedPreferences.getString("portBroker", "postman.cloudmqtt.com:14232xx"));
                }
                dialog.dismiss();
                connectAndWorkMQTT();
                useWhenOffSmartPhone();
            }
        });
    }

    //============== End Setting menu toolbar =============
    private void getTimes(String url){
         RequestQueue requestQueue = Volley.newRequestQueue(this);
         JsonArrayRequest jsonArrayRequest = new JsonArrayRequest(Request.Method.GET, url, null,
                 new Response.Listener<JSONArray>() {
                     @Override
                     public void onResponse(JSONArray response) {
                         arrayTime.clear();
                         int len = response.length();
                         for(int i = 0; i < len; i++){
                             try {
                                 JSONObject object = response.getJSONObject(i);
                                 arrayTime.add(new Time(
                                         object.getInt("id"),
                                         object.getInt("hour"),
                                         object.getInt("minute"),
                                         object.getInt("seconds")
                                 ));
                             } catch (JSONException e) {
                                 e.printStackTrace();
                             }
                         }
                         adapter.notifyDataSetChanged();
                     }
                 },
                 new Response.ErrorListener(){
                     @Override
                     public void onErrorResponse(VolleyError error) {
                         Toast.makeText(MainActivity.this, getResources().getString(R.string.toast_error_timeout), Toast.LENGTH_SHORT).show();
                     }
                 });
         requestQueue.add(jsonArrayRequest);
     }

    void makeMapping(){
        listViewTime = (ListView)findViewById(R.id.listViewTime);
        arrayTime = new ArrayList<>();
        initMediaPlayer();

        constrSearch = (ConstraintLayout)findViewById(R.id.constrSearch);
        constrAdd = (ConstraintLayout)findViewById(R.id.constrAdd);
        constrSetting = (ConstraintLayout)findViewById(R.id.constrSetting);
        txtTemp = (TextView)findViewById(R.id.txtTemp);
        txtHumi = (TextView)findViewById(R.id.txtHumi);
    }

    void initMediaPlayer(){
        if(mediaPlayer != null){
            mediaPlayer.release();
        }
        mediaPlayer = MediaPlayer.create(MainActivity.this, R.raw.iphone_remix);
//        mediaPlayer.setLooping(true);
        mediaPlayer.start();
        mediaPlayer.pause();
    }



//  ==================== Start Add One Time Into Database =====================
    private void addTime(){
        final Calendar calendar = Calendar.getInstance();
        int hour = calendar.get(Calendar.HOUR_OF_DAY);
        int minute = calendar.get(Calendar.MINUTE);

        TimePickerDialog timePickerDialog = new TimePickerDialog(this,
                new TimePickerDialog.OnTimeSetListener() {
                    @Override
                    public void onTimeSet(TimePicker view, int hourOfDay, int minute) {
                        addTimeIntoDB(hourOfDay, minute, hourOfDay * 3600 + minute * 60);
                    }
                }, hour, minute, true);
        timePickerDialog.show();
    }

    private void addTimeIntoDB(final int hour, final int minute, final int seconds){
        RequestQueue requestQueue = Volley.newRequestQueue(this);
        StringRequest stringRequest = new StringRequest(Request.Method.POST, urlInsertTimes,
                new Response.Listener<String>() {
                    @Override
                    public void onResponse(String response) {
                        if("success".equals(response.trim())){
                            Toast.makeText(MainActivity.this, getResources().getString(R.string.toast_add_success), Toast.LENGTH_SHORT).show();
                            getTimes(urlGetTimes);
                        } else if("Has existed".equals(response.trim())){
                            Toast.makeText(MainActivity.this, getResources().getString(R.string.toast_record_existed), Toast.LENGTH_SHORT).show();
                        }else{
                            Toast.makeText(MainActivity.this, getResources().getString(R.string.toast_error_add), Toast.LENGTH_SHORT).show();
                        }
                    }
                },
                new Response.ErrorListener() {
                    @Override
                    public void onErrorResponse(VolleyError error) {
                        Toast.makeText(MainActivity.this, getResources().getString(R.string.toast_error_system_internet), Toast.LENGTH_SHORT).show();
                    }
                }
        ){
            @Override
            protected Map<String, String> getParams() throws AuthFailureError {
                Map<String, String> params = new HashMap<>();
                params.put("hour", String.valueOf(hour));
                params.put("minute", String.valueOf(minute));
                params.put("seconds", String.valueOf(seconds));

                return params;
            }
        };
        requestQueue.add(stringRequest);
    }
    //  =================== End Add One Time Into Database ====================

    //  ================= Start Update One Time Into Database =================
    public void updateTime(final int id, int hour, int minute){
        TimePickerDialog timePickerDialog = new TimePickerDialog(this,
                new TimePickerDialog.OnTimeSetListener() {
                    @Override
                    public void onTimeSet(TimePicker view, int hourOfDay, int minute) {
                        updateTimeIntoDB(id, hourOfDay, minute, hourOfDay * 3600 + minute * 60);
                    }
                }, hour, minute, true);
        timePickerDialog.show();
    }

    private void updateTimeIntoDB(final int id, final int hour, final int minute, final int seconds){
        RequestQueue requestQueue = Volley.newRequestQueue(this);
        StringRequest stringRequest = new StringRequest(Request.Method.POST, urlUpdateTimes,
                new Response.Listener<String>() {
                    @Override
                    public void onResponse(String response) {
                        if(response.trim().equals("success")){
                            Toast.makeText(MainActivity.this, getResources().getString(R.string.toast_update_success), Toast.LENGTH_SHORT).show();
                            getTimes(urlGetTimes);
                        }else{
                            Toast.makeText(MainActivity.this, getResources().getString(R.string.toast_error_update), Toast.LENGTH_SHORT).show();
                        }
                    }
                },
                new Response.ErrorListener() {
                    @Override
                    public void onErrorResponse(VolleyError error) {
                        Toast.makeText(MainActivity.this, getResources().getString(R.string.toast_error_system_internet), Toast.LENGTH_SHORT).show();
                    }
                }
        ){
            @Override
            protected Map<String, String> getParams() throws AuthFailureError {
                Map<String, String> params = new HashMap<>();
                params.put("id", String.valueOf(id));
                params.put("hour", String.valueOf(hour));
                params.put("minute", String.valueOf(minute));
                params.put("seconds", String.valueOf(seconds));

                return params;
            }
        };
        requestQueue.add(stringRequest);
    }
    //  ================== End Update One Time Into Database ==================

    //  ================= Start Delete One Time Into Database =================\
    public void deleteTimeIntoDB(final int id){
        RequestQueue requestQueue = Volley.newRequestQueue(this);
        StringRequest stringRequest = new StringRequest(Request.Method.POST, urlUpdateDelete,
                new Response.Listener<String>() {
                    @Override
                    public void onResponse(String response) {
                        if(response.trim().equals("success")){
                            Toast.makeText(MainActivity.this, getResources().getString(R.string.toast_delete_success), Toast.LENGTH_SHORT).show();
                            getTimes(urlGetTimes);
                        }else{
                            Toast.makeText(MainActivity.this, getResources().getString(R.string.toast_error_delete), Toast.LENGTH_SHORT).show();
                        }
                    }
                },
                new Response.ErrorListener() {
                    @Override
                    public void onErrorResponse(VolleyError error) {
                        Toast.makeText(MainActivity.this, getResources().getString(R.string.toast_error_system_internet), Toast.LENGTH_SHORT).show();
                    }
                }
        ){
            @Override
            protected Map<String, String> getParams() throws AuthFailureError {
                Map<String, String> params = new HashMap<>();
                params.put("id", String.valueOf(id));

                return params;
            }
        };
        requestQueue.add(stringRequest);
    }
    //  ================== End Delete One Time Into Database ==================

    void useWhenOffSmartPhone(){
        if(mainSerVice != null){
            stopService(mainSerVice);
            mainSerVice.clone();
        }
        mainSerVice = new Intent(MainActivity.this, MainService.class);

        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.O){
            //Phiên bản mới
            startForegroundService(mainSerVice);
        }else{
            startService(mainSerVice);
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if(client != null){
            try {
                client.disconnect();
            }catch (Exception e){

            }
        }
    }
}