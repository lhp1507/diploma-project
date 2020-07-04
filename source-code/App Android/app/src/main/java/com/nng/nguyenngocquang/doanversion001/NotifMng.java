package com.nng.nguyenngocquang.doanversion001;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.content.Context;
import android.graphics.Color;
import android.os.Build;

public class NotifMng {
    //Dang ky channel
    public  static void createChanel(Context mContext,
                                     String chId,
                                     String chName,
                                     int impt){
        NotificationManager nm = (NotificationManager) mContext.getSystemService(Context.NOTIFICATION_SERVICE);


        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            //Dang ky chanl => chi can nhung phien ban orenge tro len
            NotificationChannel nc = new NotificationChannel(chId, chName, impt);

            //Khi co thong bao thi den se sang len.
            nc.enableLights(true);

            //Thong bao cua channel se co mau xanh
            nc.setLightColor(Color.GREEN);

            //Dang ky
            nm.createNotificationChannel(nc);
        }
    }

    //Khi goi no thi truyen context len
    public static void callNotify(Context mContext,
                                  int id,
                                  String chId,
                                  String title,
                                  String text){
        NotificationManager nm = (NotificationManager) mContext.getSystemService(Context.NOTIFICATION_SERVICE);

        Notification.Builder nb = new Notification.Builder(mContext)
                .setContentTitle(title)
                .setContentText(text)
                .setSmallIcon(R.drawable.medicine);;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            nb.setChannelId(chId);
        }
        nm.notify(id, nb.build());
    }

    public static Notification callNotify4MainService(Context mContext,
                                  String chId,
                                  String title,
                                  String text){

        Notification.Builder nb = new Notification.Builder(mContext)
                .setContentTitle(title)
                .setContentText(text)
                .setSmallIcon(R.drawable.medicine);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            nb.setChannelId(chId);
        }
        return nb.build();
    }
}
