package com.nng.nguyenngocquang.doanversion001;

public class Time {
    private int id;
    private int hour;
    private int minute;
    private int seconds;

    public Time(int id, int hour, int minute, int seconds) {
        this.id = id;
        this.hour = hour;
        this.minute = minute;
        this.seconds = seconds;
    }

    public Time() {
    }

    public int getId() {
        return id;
    }

    public void setId(int id) {
        this.id = id;
    }

    public int getHour() {
        return hour;
    }

    public void setHour(int hour) {
        this.hour = hour;
    }

    public int getMinute() {
        return minute;
    }

    public void setMinute(int minute) {
        this.minute = minute;
    }

    public int getSeconds() {
        return seconds;
    }

    public void setSeconds(int seconds) {
        this.seconds = seconds;
    }
}
