package com.nng.nguyenngocquang.doanversion001;

import android.os.Bundle;
import android.view.View;
import android.view.WindowManager;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.Button;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import org.eclipse.paho.android.service.MqttAndroidClient;
import org.eclipse.paho.client.mqttv3.IMqttActionListener;
import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.IMqttToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;

public class callActivity extends AppCompatActivity {
    Button btnAgree;
    @Override
    protected void onStart() {
        super.onStart();
        //du tat ban hinh thi van hien thi len duoc
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_SHOW_WHEN_LOCKED |
                WindowManager.LayoutParams.FLAG_TURN_SCREEN_ON);
        MainService.activityCallRunning = true;

        Animation animation = AnimationUtils.loadAnimation(this, R.anim.anim_agree);
        btnAgree = (Button)findViewById(R.id.btnAgree);
        btnAgree.startAnimation(animation);
    }

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_call);
        initMQTT();
    }

    public static MqttAndroidClient client;
    void initMQTT(){
        String clientId = MqttClient.generateClientId();

        if(client != null){
            try {
                client.disconnect();
            }catch (Exception e){
                //Do nothing
            }
        }
        client =
                new MqttAndroidClient(this, MainActivity.serverUriMQTT,
                        clientId);

        client.setCallback(new MqttCallback() {
            @Override
            public void connectionLost(Throwable cause) {
                // Do nothing
            }

            @Override
            public void messageArrived(String topic, MqttMessage message) throws Exception {
                if("off".equals(message.toString())){
                    if(MainActivity.mediaPlayer != null){
                        MainActivity.mediaPlayer.pause();
                    }

                    MainService.activityCallRunning = false;
                    finish();
                }
            }

            @Override
            public void deliveryComplete(IMqttDeliveryToken token) {
                // Do nothing
            }
        });

        try {
            IMqttToken token = null;
            if(MainActivity.securityMQTT){
                MqttConnectOptions options = new MqttConnectOptions();
                options.setUserName(MainActivity.usernameMQTT);
                options.setCleanSession(true);
                options.setPassword(MainActivity.passwordMQTT.toCharArray());
                token = client.connect(options);
            }else{
                token = client.connect();
            }
            token.setActionCallback(new IMqttActionListener() {
                @Override
                public void onSuccess(IMqttToken asyncActionToken) {
                    String[] topic = new String[]{"Reminds", "Topic"};
                    int[] qos = new int[]{1, 1};
                    try {
                        client.subscribe(topic, qos);
                    } catch (MqttException e) {
                        //Do nothing
                    }
                }

                @Override
                public void onFailure(IMqttToken asyncActionToken, Throwable exception) {
                    // Do nothing
                }
            });
        } catch (MqttException e) {
            // Do nothing
        }
    }

    public void stopSoud(View v){
        MainActivity.publish("Reminds", "off");
        if(MainActivity.mediaPlayer != null){
            MainActivity.mediaPlayer.pause();
        }
        MainService.activityCallRunning = false;
        finish();
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