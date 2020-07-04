package com.nng.nguyenngocquang.doanversion001;

import android.app.NotificationManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
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

import java.io.UnsupportedEncodingException;

public class MqttConnectionBoardcast extends BroadcastReceiver {
    static public String BOARD_CAST_NAME = "MqttConnectionBoardcast";
    static public String NOTIFY_ID = "BOARD_CAST_NAME";
    Context mContext;
    public static MqttAndroidClient client;

    int idNotify = 2;
    void initMQTT(){
        String clientId = MqttClient.generateClientId();
        if(client != null){
            Log.d("aaa", "31. client.close()");
            try {
                client.unsubscribe("Reminds");
                client.disconnect();
            }catch (MqttException | NullPointerException e){
                //Do nothing
            }
            Log.d("aaa", "33. client.close()");
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
                        if(MainService.activityCallRunning == false){
                            NotifMng.callNotify(mContext, idNotify, NOTIFY_ID, "Đến giờ uống thuốc", "Đừng quên uống thuốc để luôn có sức khỏe tốt!");
                            Intent i = new Intent(mContext, callActivity.class);
                            i.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                            mContext.startActivity(i);
                        }
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

    void pub(String content){
        String topic = "androidTx";
        byte[] encodedPayload = new byte[0];
        try {
            encodedPayload = content.getBytes("UTF-8");
            MqttMessage message = new MqttMessage(encodedPayload);
            client.publish(topic, message);
        } catch (UnsupportedEncodingException | MqttException e) {
            //Do nothing
        }
    }

    static public void startMe (Context context){
        Intent intent = new Intent();
        intent.setAction(BOARD_CAST_NAME);
        context.sendBroadcast(intent);
    };

    @Override
    public void onReceive(Context context, Intent intent) {
        mContext = context;
        initMQTT();
        NotifMng.createChanel(mContext, NOTIFY_ID, "MqttConnectionBoardcast", NotificationManager.IMPORTANCE_DEFAULT);
    }
}