package com.nng.nguyenngocquang.doanversion001;

import android.app.NotificationManager;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.IBinder;
import android.util.Log;

import org.eclipse.paho.android.service.MqttAndroidClient;
import org.eclipse.paho.client.mqttv3.IMqttActionListener;
import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.IMqttToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;

public class MainService extends Service {
    static boolean activityCallRunning = false;
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        initFore();

        registerReceiver(new MqttConnectionBoardcast(), new IntentFilter(MqttConnectionBoardcast.BOARD_CAST_NAME));
        MqttConnectionBoardcast.startMe(this);

        initMQTT();
    }

    Context mContext = this;
    public static MqttAndroidClient client;
    void initMQTT(){
        mContext = this;
        String clientId = MqttClient.generateClientId();
        if(client != null){
            try {
                client.unsubscribe("Reminds");
                client.disconnect();
            }catch (MqttException | NullPointerException e){
                //Do nothing
            }
        }

        client =
                new MqttAndroidClient(mContext, MainActivity.serverUriMQTT,
                        clientId);

        client.setCallback(new MqttCallback() {
            @Override
            public void connectionLost(Throwable cause) {
                //Do nothing
            }

            @Override
            public void messageArrived(String topic, MqttMessage message) throws Exception {
                if("Reminds".equals(topic)){
                    if("Alarm".equals(message.toString())){

                    }
                }
            }

            @Override
            public void deliveryComplete(IMqttDeliveryToken token) {
                //Do nothing
            }
        });

        try {
            IMqttToken token = null;
            if(MainActivity.securityMQTT){
                MqttConnectOptions options = new MqttConnectOptions();
                options.setUserName(MainActivity.usernameMQTT);
                options.setPassword(MainActivity.passwordMQTT.toCharArray());
                options.setCleanSession(true);
                token = client.connect(options);
            }else{
                token = client.connect();
            }
            token.setActionCallback(new IMqttActionListener() {
                @Override
                public void onSuccess(IMqttToken asyncActionToken) {
                    String[] topic = new String[]{"Reminds"};
                    int[] qos = new int[]{1};
                    try {
                        client.subscribe(topic, qos);
                    } catch (MqttException e) {
                        e.printStackTrace();
                    }
                }

                @Override
                public void onFailure(IMqttToken asyncActionToken, Throwable exception) {
                    // Something went wrong e.g. connection timeout or firewall problems
                    Log.d("htl", "onFailure");
                }
            });
        } catch (MqttException e) {
            //Do nothing
        }
    }

    private void initFore(){
        //"MainService" khong quan trong lam, day thong bao len
        NotifMng.createChanel(this, "MainService", "main_service", NotificationManager.IMPORTANCE_MAX);
        //MainService tren va duoi phai nhu nhau
       startForeground(1, NotifMng.callNotify4MainService(this, "MainService", "Nhắc nhở uống thuốc", "Uống thuốc đúng giờ"));
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        return START_STICKY;
    }

    @Override
    public boolean onUnbind(Intent intent) {
        return super.onUnbind(intent);
    }


    @Override
    public void onDestroy() {
        super.onDestroy();
    }
}
