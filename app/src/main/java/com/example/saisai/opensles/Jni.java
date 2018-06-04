package com.example.saisai.opensles;

/**
 * Created by saisai on 2018/6/4 0004.
 */

public class Jni {

    private static Jni instance=null;

    private Jni(){

    }

    public static Jni getInstance(){
        if (instance==null){
            synchronized (Jni.class){
                if (instance==null){
                    instance=new Jni();
                }
            }
        }
        return  instance;
    }

    public native int OpenAssets(String filename);
}
