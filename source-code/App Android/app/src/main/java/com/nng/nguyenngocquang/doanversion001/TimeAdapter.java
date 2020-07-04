package com.nng.nguyenngocquang.doanversion001;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageButton;
import android.widget.TextView;
import java.text.DecimalFormat;
import java.util.List;

public class TimeAdapter extends BaseAdapter {

        private MainActivity context;
        private int layout;
        private List<Time> listTime;
        DecimalFormat formatTime = new DecimalFormat("00");

    public TimeAdapter(MainActivity context, int layout, List<Time> listTime) {
            this.context = context;
            this.layout = layout;
            this.listTime = listTime;
    }

    public TimeAdapter() {
    }

    @Override
    public int getCount() {
        return listTime.size();
    }

    @Override
    public Object getItem(int position) {
        return null;
    }

    @Override
    public long getItemId(int position) {
        return 0;
    }

    private  class ViewHolder{
        TextView txtTime;
        ImageButton imgbtnEdit;
        ImageButton imgbtnDelete;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {

        ViewHolder viewHolder;

        if(convertView == null){
            viewHolder = new ViewHolder();

            LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            convertView = inflater.inflate(layout, null);

            viewHolder.txtTime = (TextView) convertView.findViewById(R.id.txtTime);
            viewHolder.imgbtnEdit = (ImageButton) convertView.findViewById(R.id.imgbtnEdit);
            viewHolder.imgbtnDelete = (ImageButton) convertView.findViewById(R.id.imgbtnDelete);

            convertView.setTag(viewHolder);
        }else{
            viewHolder = (ViewHolder) convertView.getTag();
        }

        final Time time = listTime.get(position);

        viewHolder.txtTime.setText(formatTime.format(time.getHour()) + ":" + formatTime.format(time.getMinute()));

        //catch event edit and delete
        viewHolder.imgbtnEdit.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                context.updateTime(time.getId(), time.getHour(), time.getMinute());
            }
        });

        viewHolder.imgbtnDelete.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                confirmDelete(time.getId(),formatTime.format(time.getHour()) + ":" + formatTime.format(time.getMinute()));
            }
        });

        return convertView;
    }

    private void confirmDelete(final int id, String time){
        AlertDialog.Builder dialogDelete = new AlertDialog.Builder(context);
        dialogDelete.setMessage("Bạn có muốn xóa thời gian '" + time + "' không?");

        dialogDelete.setPositiveButton("Có", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                context.deleteTimeIntoDB(id);
            }
        });

        dialogDelete.setNegativeButton("Không", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {

            }
        });

        dialogDelete.show();
    }
}