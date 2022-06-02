#include <iostream>

using namespace std;
//wav由3部分组成，RIFF chunk / FORMAT chunk / DATA chunk
typedef struct WAV_HEAD1
{
	char ID[4];
	unsigned long size;
	char type[4];
}header;

typedef struct WAV_HEAD2 {
	char ID[4];
	unsigned long size; //表示该区块数据的长度（不包含ID和Size的长度）
	unsigned short audioformat; //AudioFormat:音频格式
	unsigned short numberChannes; //通道数
	unsigned long sampleRate; //采样率
	unsigned long biteRate; //比特率
	unsigned short blockAlign;//数据库对齐
	unsigned short bitePerSample; //采样位数
}center;

typedef struct WAV_HEAD3
{
	char ID[4];
	unsigned long size; //表示音频数据data的长度，N = ByteRate * seconds
//	unsigned long data;
}data;

//获取文件的大小
long getFileSize(const char* file) {
	FILE* fp = fopen(file,"r");
	if (fp == NULL) {
		cout << "getFileSize fopen fail" << endl;
		return 0;
	}
	fseek(fp,0,SEEK_END);
	long size = ftell(fp);
	return size;
}

int main() {
	cout << "hello world " << endl;
	const char* inFile = "audio.pcm";
	const char* outFile = "out.wav";
	unsigned short bits = 16; //采样位数8：8bit，16：16bit，32：32bit
	unsigned short channleNumber= 2; //通道数
	unsigned short sampleRate = 44100; //采样率

	//打开文件
	FILE* fInp;
	fInp = fopen(inFile, "rb");
	if (fInp == NULL) {
		cout << " fopen inFile fail" << endl;
		return -1;
	}
	// 获取文件信息
	//定义3个结构体
	WAV_HEAD1 header;
	WAV_HEAD2 center;
	WAV_HEAD3 data;
	//第一部分
	memcpy(header.ID,"RIFF",strlen("RIFF"));
	header.size = 44 + getFileSize(inFile) - 8;
	memcpy(header.type,"WAVE",strlen("WAVE"));
	//第二部分
	memcpy(center.ID, "fmt ", strlen("fmt ")); //注意空格
	center.size = 16;
	center.audioformat = 1;//1:PCM
	center.numberChannes = channleNumber;
	center.sampleRate = sampleRate;
	center.bitePerSample = bits;
	center.biteRate = sampleRate * channleNumber * bits / 8;
	center.blockAlign = channleNumber * bits / 8;
	//第三部分
	memcpy(data.ID,"data",strlen("data"));
	data.size = getFileSize(inFile);
	//封装格式
	FILE* fOutp;
	fOutp = fopen(outFile, "wb");
	if (fOutp == NULL) {
		cout << " fopen outFile fail" << endl;
		return -1;
	}
	fwrite(&header,sizeof(header),1,fOutp);
	fwrite(&center,sizeof(center),1,fOutp);
	fwrite(&data,sizeof(data),1,fOutp);
    char* buff = (char*)malloc(512);
	int lent;
	//读取pcm数据写入wav文件
	while ((lent = fread(buff, sizeof(char), 512, fInp))!=0) {
		fwrite(buff,sizeof(char),lent,fOutp);
	}
	//清理信息
	fclose(fInp);
	fclose(fOutp);
	free(buff);
	buff = NULL;
	cout << "完成转换" << endl;

	return 0;
}