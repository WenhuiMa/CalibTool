#include <stdio.h> 
#include <iostream> 
#include "Windows.h" 
#include "HCNetSDK.h" 
#include <process.h>
#include "definition.h"
#include <cassert>
#include<vector>
#include <opencv2\opencv.hpp>
#include "plaympeg4.h"
#include <time.h>
#include<cstdlib>

#include<fstream>

using namespace std;
using namespace cv;


string imagepath;
string Int2String(int n)
{
	ostringstream stream;
	stream<<n;  //n为int类型
	return stream.str();
}
long int hex2int(const string& hexStr)
{
	char *pEnd;
	return strtol(hexStr.c_str(), &pEnd, 16);
}
string Char2String(char c)
{
	string str;
	stringstream stream;
	stream << c;
	str = stream.str();
	return str;
}
string Dec2String(int value)
{
	char dig[16]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

	string hexstring;
	stringstream stream;

	for(int i =3;i>=0;i--)
	{	
		int temp = value*100>>(i*4);
		int index = temp & 15;

		stream<<dig[index];
		hexstring = stream.str();	
	}
	return hexstring;
}

string CalcChecksum(int value)
{
	int DataH = value*100>>8;
	DataH &=255;
	int DataL = value*100;
	DataL &=255;
	string checksum = Dec2String(DataH+DataL);

	return checksum.substr(checksum.length()-2,checksum.length()-1);

}

string genCmd(int cmdtype,int value=0)
{
	string Cmdstr;
	switch (cmdtype)
	{
	case HORIZONTAL_CHECK:
		Cmdstr="FF 01 00 51 00 00 52";
		break;
	case VERTICAL_CHECK:
		Cmdstr="FF 01 00 53 00 00 54";
		break;
	case HORIZONTAL_SET:
		Cmdstr = "FF 01 00 4b ";
		string temp = Dec2String(value);
		Cmdstr = Cmdstr.append(temp.substr(0,2));
		Cmdstr = Cmdstr.append(" ");
		Cmdstr = Cmdstr.append(temp.substr(2,2));
		Cmdstr = Cmdstr.append(" ");
		Cmdstr = Cmdstr.append(CalcChecksum(value));
		break;
	}
	return Cmdstr;

}


int IsHexChar(char hc)
{
	if('0'<=hc && hc<='9')
		return (int(hc)-int('0'));
	else if('a'<=hc && hc<='f')
		return (int(hc)-int('a')+10);
	else if('A'<=hc && hc<='F')
		return (int(hc)-int('A')+10);
	return -1;
}

unsigned char Hex2Char(const string &hex)
{    	
	assert(hex.length() == 2);
	int high = IsHexChar(hex[0]);
	int low = IsHexChar(hex[1]);
	if(high == -1 || low == -1)
		return '\0';
	int asc = high*16+low;
	//	char b = toascii(asc);
	return asc;
}

string Hex2String(const string &hex)
{
	assert(hex.length()%2 == 0);
	string hstr;
	for(int i=0; i<hex.length(); i+=2)
	{
		string tmp = hex.substr(i,2);
		hstr.append(1,Hex2Char(tmp));
	}
	return hstr;
}
string PrepareHexString(string str)
{
	//先删除空格
	int i=0;
	while(i != str.length())
	{
		i = str.find(' ',i);
		if(i == -1)
			break;
		str = str.erase(i,1);
	}
	
	//删除0x
    i = 0;
	string tmp("0x");
	while(i != str.length())
	{
		i = str.find(tmp,i);
		if(i == -1)
			break;
		str = str.erase(i,2);
	}
	
	//删除0X
    i = 0;
	tmp = "0X";
	while(i != str.length())
	{
		i = str.find(tmp,i);
		if(i == -1)
			break;
		str = str.erase(i,2);
	}
	if(str.length()%2 != 0)
		str.append(1,'0');

	str = Hex2String(str);
    return str;
}


void CALLBACK g_fSerialDataCallBack(LONG lSerialHandle, char *pRecvDataBuffer, DWORD dwBufSize, DWORD dwUser) 
{    
	SerialTrans *ptrans = (SerialTrans *)dwUser;
	
	
	for (int i = 0; i < (int)dwBufSize; i++)
	{
		char temp[20]={0};
		temp[i] = pRecvDataBuffer[i];
		sprintf(temp,"%x",pRecvDataBuffer[i]);

		cout<<temp<<"  ";

	}
	cout<<endl;
	
}

void SendDataProc(LPVOID pParam)
{
	SerialTrans *pTrans = (SerialTrans*)pParam;
	if (!NET_DVR_SerialSend(pTrans->m_lSerialHandle,pTrans->m_iSerialChan,pTrans->m_DataBuf,pTrans->m_DataLen) )
	{
		printf("NET_DVR_SerialSend error, %d\n", NET_DVR_GetLastError());
		
		return;
	}

}

void SerialTrans::DoSend(string hexstring)
{

	hexstring = PrepareHexString(hexstring);
	int len = hexstring.length();
    m_DataLen = len<DATABUF_LEN ? len : DATABUF_LEN;
	memcpy(m_DataBuf,hexstring.c_str(),m_DataLen); 
	
	//创建发送线程
	ULONG SendDataThread = _beginthread(SendDataProc,NULL,(void*)this);
	
}


void SerialTrans::SerialStart()
{
	
	m_lSerialHandle = NET_DVR_SerialStart(m_struDeviceInfo.lLoginID, m_iCurSerialType, g_fSerialDataCallBack, (DWORD)this);
}

void SerialTrans::SerialStop()
{
	NET_DVR_SerialStop(m_lSerialHandle); //注销用户 
	NET_DVR_Logout(m_struDeviceInfo.lLoginID);     //释放 SDK 资源 
	NET_DVR_Cleanup();
	return;
}

void SerialTrans::Initialize()
{
	NET_DVR_Init();     //设置连接时间与重连时间 
	NET_DVR_SetConnectTime(2000, 1);
	NET_DVR_SetReconnect(10000, true);
}
BOOL SerialTrans::DoLogin(char* ip)
{
	NET_DVR_DEVICEINFO_V30 DeviceInfoTmp;
	memset(&DeviceInfoTmp,0,sizeof(NET_DVR_DEVICEINFO_V30));
	
	LONG lLoginID = NET_DVR_Login_V30(ip,8000,"admin","q1w2e3r4",&DeviceInfoTmp);	
	
	if(lLoginID == -1)
	{
		cout<<"Login to Device failed!\n"<<endl;
		return FALSE;
	}

	m_struDeviceInfo.lLoginID = lLoginID;
	m_struDeviceInfo.iDeviceChanNum = DeviceInfoTmp.byChanNum;
	return TRUE;
}

bool SerialTrans::get_pos(NET_DVR_PTZPOS &ptz_pos)
{
	DWORD  dwReturned;
	BOOL bRet;
	memset(&ptz_pos,0,sizeof(NET_DVR_PTZPOS));

	bRet = NET_DVR_GetDVRConfig(m_struDeviceInfo.lLoginID,NET_DVR_GET_PTZPOS,1,&ptz_pos,sizeof(NET_DVR_PTZPOS),&dwReturned);

	if(!bRet)
	{
		std::cout<<"get_pos error"<<NET_DVR_GetLastError()<<std::endl;
		return false;
	}

	return true;
}
bool SerialTrans::set_pos( NET_DVR_PTZPOS &ptz_pos )
{
	BOOL bRet;
	bRet = NET_DVR_SetDVRConfig(m_struDeviceInfo.lLoginID,NET_DVR_SET_PTZPOS,1,&ptz_pos,sizeof(NET_DVR_PTZPOS));

	if(!bRet)
	{
		std::cout<<"HCNetCamera::set_pos "<<NET_DVR_GetLastError()<<std::endl;
		return false;
	}

	return true;
}
SerialTrans::SerialTrans()
{
	m_iSerialChan = 1;
	m_iCurSerialType = 2;
	m_lSerialHandle = -1;
	m_DataLen = 16;
	memset(m_DataBuf,0,sizeof(m_DataBuf));

}
/////////////////////////////////////////////
volatile int gbHandling = 3;
LONG nPort = -1;

vector<Point2f> PointA;
vector<Point2f> PointB;

bool setpoint = false;

///////////////////////////////////////////// tracking parameter
Point2f point;
bool addRemovePt = false;

TermCriteria termcrit(TermCriteria::MAX_ITER | TermCriteria::EPS, 20, 0.03);
Size subPixWinSize(10, 10), winSize(31, 31);

const int MAX_COUNT = 1;
bool needToInit = false;
Mat gray, prevGray, image;
vector<Point2f> points[2]; 
/////////////////////////////////////////////


int imgcount=0;

int zoomtable[59] = {16, 21, 32, 37, 48, 
                     53, 64, 69, 80, 85, 
					 96, 101, 112, 117, 128, 
					 133, 144, 149, 256, 261, 
					 272, 277, 288, 293, 304, 
				     309, 320, 325, 336, 341, 
				     352, 357, 368, 373, 384, 
				     389, 400, 405, 512, 517,
					 528, 533, 544, 549, 560,
					 565, 576, 581, 592, 597, 
					 608, 613, 624, 629, 640, 
					 645, 656, 661, 768 
};

static void onMouse(int event, int x, int y, int /*flags*/, void* /*param*/)
{
	if (event == EVENT_LBUTTONDOWN)
	{
		point = Point2f((float)x, (float)y);
		addRemovePt = true;
		setpoint = true;

		cout<<point<<endl;
	}
}

void track(Mat frame)
{
		namedWindow("Mywindow", 1);
		setMouseCallback("Mywindow", onMouse, 0);
		frame.copyTo(image);
		Mat element = getStructuringElement(MORPH_RECT,Size(3,3));
		//blur(image,image,Size(5,5));//预处理	
		cvtColor(image, gray, COLOR_BGR2GRAY);
		threshold(gray,gray,80,255,CV_THRESH_BINARY);
		dilate(gray,gray,element);
		erode(gray,gray,element);
		dilate(gray,gray,element);

		if (needToInit)
		{
			goodFeaturesToTrack(gray, points[1], MAX_COUNT, 0.01, 10, Mat(), 3, 0, 0.04);
			cornerSubPix(gray, points[1], subPixWinSize, Size(-1, -1), termcrit);
			addRemovePt = false;
		}
		else if (!points[0].empty())
		{
			vector<uchar> status;
			vector<float> err;
			if (prevGray.empty())
				gray.copyTo(prevGray);
			calcOpticalFlowPyrLK(prevGray, gray, points[0], points[1], status, err, winSize,
				3, termcrit, 0, 0.001);
			size_t i, k;
			for (i = k = 0; i < points[1].size(); i++)
			{
				if (addRemovePt)
				{
					if (norm(point - points[1][i]) <= 5)
					{
						addRemovePt = false;
						continue;
					}
				}

				if (!status[i])
					continue;

				points[1][k++] = points[1][i];
				circle(image, points[1][i], 3, Scalar(0, 255, 0), -1, 8);


			}
			points[1].resize(k);
		}

		if (addRemovePt && points[1].size() < (size_t)MAX_COUNT)
		{
			vector<Point2f> tmp;
			tmp.push_back(point);

			cornerSubPix(gray, tmp, winSize, Size(-1, -1), termcrit);
			points[1].push_back(tmp[0]);
			addRemovePt = false;
		}

		needToInit = false;
		circle(image, Point(0.5*image.cols,0.5*image.rows), 3, Scalar(0, 0, 255), -1, 8);
		
		imshow("Mywindow", image);

		char c = (char)waitKey(10);

		switch (c)
		{
		case 'r':
			needToInit = true;
			break;
		case 'c':
			points[0].clear();
			points[1].clear();
			break;
		case 'q':
			return;
			break;
		case 's':
			
			imwrite("C:/Users/MWH/Desktop/image/"+Int2String(imgcount)+".jpg",image);
			imgcount++;
			break;
		}
		std::swap(points[1], points[0]);
		cv::swap(prevGray, gray);
	
	waitKey(1);

}


void CALLBACK DecCBFun(long nPort, char * pBuf, long nSize, FRAME_INFO * pFrameInfo, long nReserved1, long nReserved2)
{
	
	if (gbHandling)
	{
		gbHandling--;
		return;
	}

	long lFrameType = pFrameInfo->nType;
	if (lFrameType == T_YV12)
	{

		Mat pImg(pFrameInfo->nHeight, pFrameInfo->nWidth, CV_8UC3);
		Mat src(pFrameInfo->nHeight + pFrameInfo->nHeight / 2, pFrameInfo->nWidth, CV_8UC1, pBuf);
		cvtColor(src, pImg, CV_YUV2BGR_YV12);
		//track(pImg);
		pImg.copyTo(image);
		imshow("Mywindow",pImg);
		waitKey(10);
	}
	gbHandling = 3;

}


///实时流回调
void CALLBACK fRealDataCallBack(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void *pUser)
{

	switch (dwDataType)
	{
	case NET_DVR_SYSHEAD: //系统头

		if (!PlayM4_GetPort(&nPort))  //获取播放库未使用的通道号
		{
			break;
		}
		//m_iPort = lPort; //第一次回调的是系统头，将获取的播放库port号赋值给全局port，下次回调数据时即使用此port号播放
		if (dwBufSize > 0)
		{
			if (!PlayM4_SetStreamOpenMode(nPort, STREAME_REALTIME))  //设置实时流播放模式
			{
				break;
			}

			if (!PlayM4_OpenStream(nPort, pBuffer, dwBufSize, 10 * 1024 * 1024)) //打开流接口
			{
				break;
			}

			if (!PlayM4_Play(nPort, NULL)) //播放开始
			{
				break;
			}
			if (!PlayM4_SetDecCallBack(nPort, DecCBFun))
			{
				break;
			}
		}
		break;
	case NET_DVR_STREAMDATA:   //码流数据
		if (dwBufSize > 0 && nPort != -1)
		{
			if (!PlayM4_InputData(nPort, pBuffer, dwBufSize))
			{
				cout << "error" << PlayM4_GetLastError(nPort) << endl;
				break;
			}
		}
		break;
	default: //其他数据
		if (dwBufSize > 0 && nPort != -1)
		{
			if (!PlayM4_InputData(nPort, pBuffer, dwBufSize))
			{
				break;
			}
		}
		break;
	}
}


void CALLBACK g_ExceptionCallBack(DWORD dwType, LONG lUserID, LONG lHandle, void *pUser)
{
	char tempbuf[256] = { 0 };
	switch (dwType)
	{
	case EXCEPTION_RECONNECT:    //预览时重连
		printf("----------reconnect--------%d\n", time(NULL));
		break;
	default:
		break;
	}
}


void setFunction(SerialTrans trans63)
{
			
			
			cout<<"saving img A"<<endl;
			imwrite(imagepath + Int2String(imgcount)+"A.jpg",image);
			Sleep(1500);
			trans63.DoSend("FF 01 00 4D 00 64 B2");	
			Sleep(1000);
			trans63.DoSend("FF 01 00 4B 00 64 B0");
			Sleep(2000);

			cout<<"saving img B"<<endl;
			imwrite(imagepath + Int2String(imgcount)+"B.jpg",image);
			Sleep(1500);
			trans63.DoSend("FF 01 00 4B 00 00 4C");
			Sleep(1000);
			trans63.DoSend("FF 01 00 4D 00 00 4e");
			Sleep(2000);

			imgcount++;
}
void main() 
{
	bool imageExtracted = false;
	char ExePath[MAX_PATH];
	//GetModuleFileName(NULL,ExePath,MAX_PATH);
	GetCurrentDirectory(MAX_PATH,ExePath);
	imagepath = ExePath;
	imagepath +="/";
	if(imagepath.length() == 0)
		return;

	//cout<<imagepath<<endl;

	SerialTrans trans64;
	trans64.Initialize();
	Sleep(500);
	if(trans64.DoLogin("192.168.192.64"))
	{
		cout<<"Login succeeded:"<<"192.168.192.64"<<endl;
		LONG lRealPlayHandle;

		NET_DVR_PREVIEWINFO struPlayInfo = { 0 };

		struPlayInfo.lChannel = trans64.m_iSerialChan;           //预览通道号
		struPlayInfo.dwStreamType = 0;                     //0-主码流，1-子码流，2-码流3，3-码流4，以此类推
		struPlayInfo.dwLinkMode = 0;                    //0- TCP方式，1- UDP方式，2- 多播方式，3- RTP方式，4-RTP/RTSP，5-RSTP/HTTP

		lRealPlayHandle = NET_DVR_RealPlay_V40(trans64.m_struDeviceInfo.lLoginID, &struPlayInfo, fRealDataCallBack, NULL);
		
		if (lRealPlayHandle < 0)
		{
			printf("NET_DVR_RealPlay_V40 error\n");
			printf("%d\n", NET_DVR_GetLastError());
			NET_DVR_Logout(trans64.m_struDeviceInfo.lLoginID);
			NET_DVR_Cleanup();
			return;
		}
		cout << "playing" << endl;
		

		SerialTrans trans63;
	    trans63.Initialize();
	    Sleep(500);
		
		if(trans63.DoLogin("192.168.192.63"))
		{	
			cout<<"Login succeeded:"<<"192.168.192.63"<<endl;
			trans63.SerialStart();
			
			cout<<"Position initializing...";
			trans63.DoSend("FF 01 00 4B 00 00 4C");cout<<"moving..."; //horizontal 0
			trans63.DoSend("FF 01 00 4D 00 00 4e");cout<<"initialized !"<<endl;//ver 0
			
			cout<<"**************************************"<<endl;

			Sleep(2000);

			NET_DVR_PTZPOS pos;
			
			pos.wAction = 1;
			pos.wPanPos = 1;
			pos.wTiltPos = 1;
			pos.wZoomPos = zoomtable[0];
			trans64.set_pos(pos);
			Sleep(9000);
			for(int i = 0;i<IMAGEPAIRS;i++)
			{
				cout<<"Zoom: "<<10+5*i<<endl;
				pos.wAction = 1;
				pos.wPanPos = 1;
				pos.wTiltPos = 1;
				pos.wZoomPos = zoomtable[i];
				trans64.set_pos(pos);
				Sleep(3000);
				setFunction(trans63);
			}

			pos.wZoomPos = zoomtable[0];
			trans64.set_pos(pos);

		
		}
		trans63.SerialStop();
		//Sleep(-1);
		imageExtracted = true;
	}
	
	destroyAllWindows();
	trans64.SerialStop();
	

	if(!imageExtracted)
		return;

	
	//////////////////////////let's calibrate
	cout<<"*************"<<"Set the point"<<"************"<<endl;

	Mat src_img;

	for(int i=0;i<IMAGEPAIRS;i++)
	{
		src_img = imread(imagepath + Int2String(i)+"A.jpg");
		imshow("set_pointA",src_img);	
		setMouseCallback("set_pointA", onMouse, 0);
		waitKey(0);
		destroyWindow("set_pointA");

		PointA.push_back(point);

		src_img = imread(imagepath + Int2String(i)+"B.jpg");
		imshow("set_pointB",src_img);	
		setMouseCallback("set_pointB", onMouse, 0);
		waitKey(0);
		destroyWindow("set_pointB");

		PointB.push_back(point);
	}

	/////////////////////////// dump data
	ofstream ofs;
	string csvpath = imagepath + "output.csv";
	ofs.open(csvpath.data(),ios::out);
	
	ofs<<"no."<<","
		<<"x_corner"<<","
		<<"y_corner"<<","
		<<"x_center"<<","
		<<"y_center"<<","
		<<"diff_x"<<","
		<<"diff_y"<<","
		<<"P_coeff"<<","
		<<"T_coeff"<<","<<endl;
		
	for(int i = 0;i<IMAGEPAIRS;i++)
	{
		ofs<<10+i*5<<","
			<<PointA[i].x<<","
			<<PointA[i].y<<","
			<<PointB[i].x<<","
			<<PointB[i].y<<","
			<<PointA[i].x-PointB[i].x<<","
			<<PointA[i].y-PointB[i].y<<","
			<<(PointA[i].x-PointB[i].x)/PTSTEP<<","
			<<(PointA[i].y-PointB[i].y)/PTSTEP<<","
			<<endl;
	}
	ofs.close();

}

#if 0
void main()
{
	cout<<genCmd(HORIZONTAL_SET,360)<<endl;
}

#endif