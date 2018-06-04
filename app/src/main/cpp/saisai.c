//
// Created by saisai on 2018/6/4 0004.
//

#include <jni.h>
#include <android/log.h>

//for opensles
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

//for native asset manager
#include <sys/types.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <stdio.h>
#include <malloc.h>
#include <stdint.h>

//打印日志
#define LOGI(FORMAT,...) __android_log_print(ANDROID_LOG_INFO,"saisai",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR,"saisai",FORMAT,##__VA_ARGS__);


//引擎接口
SLObjectItf engineObject=NULL;
SLEngineItf engineEngine=NULL;

//混音器
SLObjectItf outputMixObject=NULL;
SLEnvironmentalReverbItf  outputMixEnvironmentalReverb=NULL;
SLEnvironmentalReverbSettings reverbSettings=SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;

//assets播放器
SLObjectItf  fdPlayerObject=NULL;
SLPlayItf fdPlayerPlay=NULL;
SLVolumeItf fdPlayerVolumn=NULL;//声音控制接口

void release();

void createEngine(){
    SLresult  result;
    result=slCreateEngine(&engineObject,0,NULL,0,NULL,NULL);
    result=(*engineObject)->Realize(engineObject,SL_BOOLEAN_FALSE);
    result=(*engineObject)->GetInterface(engineObject,SL_IID_ENGINE,&engineEngine);
}

JNIEXPORT int JNICALL
Java_com_example_saisai_opensles_Jni_OpenAssets(JNIEnv *env, jobject instance,jobject assetManager,jstring filename){

    release();

    const char* utf8=(*env)->GetStringChars(env,filename,NULL);

    //use asset manager to open asset by filename
    AAssetManager* mgr=AAssetManager_fromJava(env,assetManager);
    AAsset* asset=AAssetManager_open(mgr,utf8,AASSET_MODE_UNKNOWN);
    (*env)->ReleaseStringChars(env,filename,utf8);

    //open asset as file descriptor
    off_t start,length;
    int fd=AAsset_openFileDescriptor(asset,&start,&length);
    AAsset_close(asset);

    SLresult result;

    //第一步，创建引擎
    createEngine();

    //第二步，创建混音器
    const SLInterfaceID mids[1]={SL_IID_ENVIRONMENTALREVERB};
    const SLboolean  mreq[1]={SL_BOOLEAN_FALSE};
    result=(*engineEngine)->CreateOutputMix(engineEngine,&outputMixObject,1,mids,mreq);
    (void)result;
    result=(*outputMixObject)->Realize(outputMixObject,SL_BOOLEAN_FALSE);
    (void)result;
    result=(*outputMixObject)->GetInterface(outputMixObject,SL_IID_ENVIRONMENTALREVERB, &outputMixEnvironmentalReverb);

    if(SL_RESULT_SUCCESS==result){
        result=(*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(outputMixEnvironmentalReverb, &reverbSettings);
        (void)result;
    }
    //第三步，设置 播放器参数和创建播放器
    //1、配置 audio source
    SLDataLocator_AndroidFD loc_fd = {SL_DATALOCATOR_ANDROIDFD, fd, start, length};
    SLDataFormat_MIME format_mime = {SL_DATAFORMAT_MIME, NULL, SL_CONTAINERTYPE_UNSPECIFIED};
    SLDataSource audioSrc = {&loc_fd, &format_mime};

    // 2、 配置 audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    //创建播放器
    const SLInterfaceID ids[3]={SL_IID_SEEK, SL_IID_MUTESOLO, SL_IID_VOLUME};
    const SLboolean req[3]={SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    result=(*engineEngine)->CreateAudioPlayer(engineEngine, &fdPlayerObject, &audioSrc, &audioSnk, 3, ids, req);
    (void)result;

    //实现播放器
    result=(*fdPlayerObject)->Realize(fdPlayerObject,SL_BOOLEAN_FALSE);
    (void)result;

    //得到播放器接口
    result=(*fdPlayerObject)->GetInterface(fdPlayerObject,SL_IID_PLAY,&fdPlayerPlay);
    (void)result;

    //得到声音控制接口
    result=(*fdPlayerObject)->GetInterface(fdPlayerObject,SL_IID_VOLUME,&fdPlayerVolumn);
    (void)result;

    //设置播放状态
    if(NULL!=fdPlayerPlay){
        result = (*fdPlayerPlay)->SetPlayState(fdPlayerPlay, SL_PLAYSTATE_PLAYING);
        (void)result;
    }


    //设置播放音量 （100 * -50：静音 ）
    (*fdPlayerVolumn)->SetVolumeLevel(fdPlayerVolumn, 20 * -50);


}


void release(){

    if(){

    }
}