#ifndef COMMON_H
#define COMMON_H

#include <QSemaphore>
/*
 * 命令传递网络端口：8888
 * 音频接受网络端口：8889
 * 视频接受网络端口：8890
 */
const quint16 COMMAND_PORT = 8888;
const quint16 AUDIO_PORT = 8889;
const quint16 VIDEO_PORT = 8890;

const int AudioBufSize = 230400;

//缓冲空闲大小和已使用大小作为信号量
extern QSemaphore AudioBufFree;
extern QSemaphore AudioBufUsed;
//缓冲数组
extern char AudioBuffer[AudioBufSize];
#endif // COMMON_H
