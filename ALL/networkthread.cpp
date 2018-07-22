#include "networkthread.h"
/*
 *网路也可以放在线程里，为发送数据建立线程类，run中发送数据（来自成员变量）
 *  接收数据时，一旦接受到数据就进入新线程，在run中作数组拷贝
 */
NetworkThread::NetworkThread(QThread *parent) : QThread(parent)
{

}
NetworkThread::~NetworkThread()
{

}
void NetworkThread::run()
{

}
