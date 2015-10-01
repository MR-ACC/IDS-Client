#ifndef NFCLIENT_H
#define NFCLIENT_H

#include <QAbstractSocket>
#include <QTcpSocket>
#include <QThread>
#include <QFile>
#include <QCryptographicHash>

#define CLIETN_VERSION 15
#define MAX_STRING_LEN 36
struct Data{
    char md5_value[MAX_STRING_LEN];
    char filesavename[MAX_STRING_LEN];
    uint filelen;
    uint shouldoplen;
    uint oplencount;
    char filebuf[1301];
};

struct Data_return {
    int writelen;
    int writelencount;
    int recvlen;
    int md5_return;
};

enum UpgradeType{
    ROOTFS = 0,
    KERNEL,
    FPGA
};

struct FileInfoV11{
    int upgradetype;
    char filename[MAX_STRING_LEN];
    uint filelen;
    char md5value[MAX_STRING_LEN];
};

struct FileInfo{
    int upgradetype;
    char filename[MAX_STRING_LEN];
    uint filelen;
    char md5value[MAX_STRING_LEN];
    int clientversion;
};

class RWThread:public QThread
{
    Q_OBJECT
signals:
    void message(const QString& str);
    void sig(int i);
    void timesig(int i);
public:
    QFile *file;
    QString filename;
    QString ipaddress;
    QString md5value;
    int index;
    UpgradeType upgradetype;
    QTcpSocket *client;
    void run();
    void sendFile(const int i, const QString& filepath, const QString& dest,const QString &md5, int type);
    int upgradeIPCWithNewMethod();
};


#endif  // NFCLIENT_H
