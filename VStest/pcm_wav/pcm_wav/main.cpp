#include <iostream>

using namespace std;
//wav��3������ɣ�RIFF chunk / FORMAT chunk / DATA chunk
typedef struct WAV_HEAD1
{
	char ID[4];
	unsigned long size;
	char type[4];
}header;

typedef struct WAV_HEAD2 {
	char ID[4];
	unsigned long size; //��ʾ���������ݵĳ��ȣ�������ID��Size�ĳ��ȣ�
	unsigned short audioformat; //AudioFormat:��Ƶ��ʽ
	unsigned short numberChannes; //ͨ����
	unsigned long sampleRate; //������
	unsigned long biteRate; //������
	unsigned short blockAlign;//���ݿ����
	unsigned short bitePerSample; //����λ��
}center;

typedef struct WAV_HEAD3
{
	char ID[4];
	unsigned long size; //��ʾ��Ƶ����data�ĳ��ȣ�N = ByteRate * seconds
//	unsigned long data;
}data;

//��ȡ�ļ��Ĵ�С
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
	unsigned short bits = 16; //����λ��8��8bit��16��16bit��32��32bit
	unsigned short channleNumber= 2; //ͨ����
	unsigned short sampleRate = 44100; //������

	//���ļ�
	FILE* fInp;
	fInp = fopen(inFile, "rb");
	if (fInp == NULL) {
		cout << " fopen inFile fail" << endl;
		return -1;
	}
	// ��ȡ�ļ���Ϣ
	//����3���ṹ��
	WAV_HEAD1 header;
	WAV_HEAD2 center;
	WAV_HEAD3 data;
	//��һ����
	memcpy(header.ID,"RIFF",strlen("RIFF"));
	header.size = 44 + getFileSize(inFile) - 8;
	memcpy(header.type,"WAVE",strlen("WAVE"));
	//�ڶ�����
	memcpy(center.ID, "fmt ", strlen("fmt ")); //ע��ո�
	center.size = 16;
	center.audioformat = 1;//1:PCM
	center.numberChannes = channleNumber;
	center.sampleRate = sampleRate;
	center.bitePerSample = bits;
	center.biteRate = sampleRate * channleNumber * bits / 8;
	center.blockAlign = channleNumber * bits / 8;
	//��������
	memcpy(data.ID,"data",strlen("data"));
	data.size = getFileSize(inFile);
	//��װ��ʽ
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
	//��ȡpcm����д��wav�ļ�
	while ((lent = fread(buff, sizeof(char), 512, fInp))!=0) {
		fwrite(buff,sizeof(char),lent,fOutp);
	}
	//������Ϣ
	fclose(fInp);
	fclose(fOutp);
	free(buff);
	buff = NULL;
	cout << "���ת��" << endl;

	return 0;
}