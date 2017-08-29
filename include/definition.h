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
	char    chChanName[40];     //通道名称
	int		iChanIndex;			//监控通道号 = 数组索引+startchan
	int		iPicResolution;				//图片分辨率
	int		iPicQuality;				//图片质量
	char	chAccessChanIP[16];     //ip接入通道的ip地址
	BOOL    bEnable;              //是否有效

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
	int		iDeviceChanNum;		    //设备的通道数
	int		iStartChan;				//设备开始通道号
	int 	iIPChanNum;				//最大数字通道个数
	int		iEnableChanNum;			//有效通道数
	BOOL    bIPRet;                 //是否支持ip接入

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
