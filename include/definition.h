#pragma once
#include "Windows.h" 
#include "string"

#define DATABUF_LEN 512

#define HORIZONTAL_CHECK 1
#define VERTICAL_CHECK 2

#define HORIZONTAL_SET 3
#define VERTICAL_SET 4
#define IMAGEPAIRS 59
#define PTSTEP 100
typedef struct STRU_CHANNEL_INFO
{
	char    chChanName[40];     //ͨ������
	int		iChanIndex;			//���ͨ���� = ��������+startchan
	int		iPicResolution;				//ͼƬ�ֱ���
	int		iPicQuality;				//ͼƬ����
	char	chAccessChanIP[16];     //ip����ͨ����ip��ַ
	BOOL    bEnable;              //�Ƿ���Ч

	STRU_CHANNEL_INFO()
	{
		chChanName[0] = '\0';
		iChanIndex = -1;
		iPicResolution = 0;
		iPicQuality = 2;
		chAccessChanIP[0] = '\0';
		bEnable = FALSE;
	}
}CHANNEL_INFO, *pCHANNEL_INFO;

typedef struct STRU_DEVICE_INFO
{

	LONG    lLoginID;
	int		iDeviceChanNum;		    //�豸��ͨ����
	int		iStartChan;				//�豸��ʼͨ����
	int 	iIPChanNum;				//�������ͨ������
	int		iEnableChanNum;			//��Чͨ����
	BOOL    bIPRet;                 //�Ƿ�֧��ip����

	CHANNEL_INFO struChanInfo[64];


	STRU_DEVICE_INFO()
	{
		lLoginID = -1;
		iDeviceChanNum = -1;
		iStartChan = 0;
		iIPChanNum = 0;
		iEnableChanNum = -1;
		bIPRet = FALSE;
	}
}LOCAL_DEVICE_INFO, *pLOCAL_DEVICE_INFO;


class SerialTrans
{

public:
	BOOL m_bIsSending;
	int m_iCurSerialType;
	int m_iSerialChan;
	int m_iCurInterface;
	BOOL DoLogin(char* ip);
	BOOL m_bIsLogin;
	LONG m_lSerialHandle;
	LOCAL_DEVICE_INFO m_struDeviceInfo;
	char m_DataBuf[DATABUF_LEN];
	int m_DataLen;


public:
	SerialTrans();
	void Initialize();
	void SerialStart();
	void SerialStop();
	void DoSend(std::string hexstring);
	bool get_pos(NET_DVR_PTZPOS &ptz_pos);
	bool set_pos(NET_DVR_PTZPOS &ptz_pos);
};
