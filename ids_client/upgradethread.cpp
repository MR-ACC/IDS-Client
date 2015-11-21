#include "upgradethread.h"

void RWThread::sendFile(const int i, const QString &filepath, const QString &dest,const QString &md5, int type)
{
    file = new QFile(filepath);
    if(!file->open(QIODevice::ReadOnly))
    {
        emit message(ipaddress + tr(": 升级失败！文件打开失败\r\n"));
        emit sig(index);
        return;
    }
    upgradetype = (UpgradeType)type;

    QByteArray ba = QCryptographicHash::hash(file->readAll(), QCryptographicHash::Md5);
    file->close();
    md5value = QString(ba.toHex().constData());
    ipaddress = dest;
    index = i;
    qDebug()<<md5value;
    filename = filepath;
    filename = filename.remove(0, filename.lastIndexOf('/')+1).right(MAX_STRING_LEN - 1);
    //qDebug()<<filename;
    /*if(filename.size()>=MAX_STRING_LEN)
    {
        emit message(ipaddress + tr(": 升级失败！固件名太长，请小于36个字符\r\n"));
        emit sig(index);
        return;
    }*/

    this->start();
}

int RWThread::upgradeIPCWithNewMethod()
{
    uint Count = 0;
    uint CountOld = -1;
    long long hassend = 0;
    long long sendlen = 0;
    int cnt = 0;
    char sendbuf[1300];
    long long filelen = file->size();
    char version[4];
    memset(version, '\0', sizeof(version));
    client->connectToHost(ipaddress, 9527);  //new port
    if(!client->waitForConnected(2000))
    {
        emit message(ipaddress + tr(": 升级失败！网络连接错误\r\n"));
        file->close();
        emit sig(index);
        return -1;
    }
    if(!client->waitForReadyRead())
    {
        emit message(ipaddress + tr(": 升级失败！无法收到服务器版本信息\r\n"));
        file->close();
        emit sig(index);
        goto quit_flag;
    }
    client->read(version,2);
    qDebug()<<ipaddress<<" version: "<<version;

    if(!strcmp(version, "11"))
    {
        FileInfoV11 fileinfo;
        fileinfo.upgradetype = (int)upgradetype;
        fileinfo.filelen = file->size();
        strcpy(fileinfo.filename,filename.toUtf8().data());
        strcpy(fileinfo.md5value,md5value.toUtf8().data());
        client->write((char*)&fileinfo,sizeof(fileinfo));
    }
    else
    {
        FileInfo fileinfo;
        fileinfo.upgradetype = (int)upgradetype;
        fileinfo.filelen = file->size();
        strcpy(fileinfo.filename,filename.toUtf8().data());
        strcpy(fileinfo.md5value,md5value.toUtf8().data());
        fileinfo.clientversion = CLIETN_VERSION;
        qDebug()<<md5value<<fileinfo.clientversion<<fileinfo.filelen;
        client->write((char*)&fileinfo,sizeof(fileinfo));
    }
    if(!client->waitForBytesWritten())
    {
        emit message(ipaddress + tr(": 升级失败！网络连接错误\r\n"));
        file->close();
        emit sig(index);
        goto quit_flag;
    }

    char msg[4];
    if(!client->waitForReadyRead())
    {
        emit message(ipaddress + tr(": 升级失败！无法收到服务器回应\r\n"));
        file->close();
        emit sig(index);
        goto quit_flag;
    }
    client->read(msg,2);
    if(!strncmp(msg, "VL", 2))
    {
        emit message(ipaddress + tr(": 升级失败！VL\r\n"));
        file->close();
        emit sig(index);
        goto quit_flag;
    }
    else if(!strncmp(msg, "NS", 2))
    {
        emit message(ipaddress + tr(": 升级失败！NS\r\n"));
        file->close();
        emit sig(index);
        goto quit_flag;
    }
    else if(!strncmp(msg, "ME", 2) && upgradetype == FPGA)
    {
        emit message(ipaddress + tr(": 升级失败！ME\r\n"));
        file->close();
        emit sig(index);
        goto quit_flag;
    }
    if(strcmp(version, "12") >= 0)
    {
        do
        {
            if(filelen - hassend > 512)
            {
                sendlen = 512;
            }
            else
            {
                sendlen = filelen - hassend;
            }

            file->read(sendbuf, sendlen);

            int offset = 0;
            while((cnt = client->write(sendbuf+offset,sendlen)) != sendlen)
            {
                qDebug()<<"write:"<<cnt;
                sendlen -= cnt;
                offset += cnt;
                if(!client->waitForBytesWritten())
                {
                    emit message(ipaddress + tr(": 升级失败！固件发送中断，请不要重启服务器，确认网络通畅后，重新升级\r\n"));
                    file->close();
                    emit sig(index);
                    goto quit_flag;
                }
            }

            hassend += (sendlen + offset);
            //qDebug()<<filelen - hassend;
            Count = hassend*100/filelen;
            if(CountOld != Count)
            {
#if 0
                qDebug()<<ipaddress<<", Count...:"<<Count<<"%";
#else
                emit message(ipaddress + tr(": 正在升级，请勿断网、断电，传输进度：") + QString::number(Count, 10) + tr("%"));
#endif
                CountOld = Count;
            }
            if(hassend == filelen)
                break;

         }while(!file->atEnd());
        file->close();
        emit message(ipaddress + tr(": 传输完成！\r\n"));

        if(!client->waitForReadyRead())
        {
            emit message(ipaddress + tr(": 升级失败！无法收到MD5\r\n"));
            emit sig(index);
            goto quit_flag;
        }
        client->read(msg,5);
        if(strncmp(msg, "MD5ER", 5) == 0)
        {
            emit message(ipaddress + tr(": 升级失败！MD5值验证失败\r\n"));
            emit sig(index);
            goto quit_flag;
        }

        client->read(msg,6);
        if(strncmp(msg, "PACKER", 6) == 0)
        {
            emit message(ipaddress + tr(": 升级失败！写操作失败\r\n"));
            emit sig(index);
            goto quit_flag;
        }
    }
    else
    {
        emit message(ipaddress + tr(": 升级失败！该服务器固件版本不支持远程升级\r\n"));
        file->close();
        emit sig(index);
        goto quit_flag;
    }

    emit timesig(index);
quit_flag:
    client->disconnectFromHost();
    return -1;
}
void RWThread::run()
{

    if(!file->open(QIODevice::ReadOnly))
    {
        emit message(ipaddress + tr(": 升级失败！文件打开失败\r\n"));
        emit sig(index);
        return;
    }
    client=new QTcpSocket();

    upgradeIPCWithNewMethod();
    return;

}



