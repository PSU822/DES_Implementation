#pragma once
#include "DES_interface.h"
#include <string.h>

typedef struct ArrayInfo {
	int currCnt;
	int maxCnt;
	DESInfo* dataPtr;
}ArrayInfo;

static int fileCntE = 1, fileCntD= 1;

// ���� �������̽� ���α׷�
void StartProgram();

// ���� ����� �Լ�
FILE* OpenFile(char* filename, char* mode);					// ���� �����͸� ó�����ִ� ������ �Լ�
void ForEncrpytion(ArrayInfo* arrInfo);
void ForDecrpytion(ArrayInfo* arrInfo);

// �迭 ���� �Լ�
void CheckArraySize(ArrayInfo* arrInfo);					// �迭�� ������ ����ٸ� �ٸ� ������ ������ ó���� ���ɼ��� �����Ƿ� �����͸� �ް� ����

// ���������� DES�� ������ ���� �ʾƼ� �� ������ ���� 
int Padding(uint64_t* text, int pos);

/* --------------------------------------------------------------------------------- */

FILE* OpenFile(char* filename, const char* mode) {
	FILE* result;
	fopen_s(&result,filename, mode);

	if (result == NULL) {
		return NULL;
	}

	return result;
}

int Padding(uint64_t* text, int currbyte) {		// ���� ���� i���� �̿��Ͽ� ���� ����Ʈ ��� �� �е�ó��.

	if(currbyte > 8)	// ���� ���� 8 �̻��̶��, ���� ������ �߻� �� �����̹Ƿ� -1�� ��ȯ
		return -1;
	
	int remainByte = 8 - currbyte;
	for (int i = 0; i < remainByte; ++i)
		*text = (*text << 8) | 0x00;			// �е��� ��������� ���� ������ �̻��� ���� ���س�

	return remainByte;							// ����ü�� ��ȯ�ϱ� ���� �е��� ����Ʈ ���� ����
}

void ForEncrpytion(ArrayInfo* arrInfo) {		// DES info�� �����ϴ� �迭�� ȣ��, key�� padding ����Ʈ, ���� �̸��� ������.
	char inputName[255];
	char toutputName[255];
	char boutputName[255];

	char buf[9];		// 64��Ʈ ��ȣȭ ����� ���Ͽ� ���� ���� �ӽ� ����
	int byteCnt = 0;

	DESInfo* info = &arrInfo->dataPtr[arrInfo->currCnt];

	FILE* rfp;
	FILE* tofp;
	FILE* bofp;

	uint64_t key;

	int paddingByte = 0;

	while (1) {							// �Է��� ��Ÿ�� �� �� �����Ƿ� ���, �ٸ� ���� �޴��� ���� �� �� �ִ� �������� �ʿ��ϴ� �Ǵ��Ͽ� �߰�.
		printf("���� �̸� �Է� : ");
		fgets(inputName, sizeof(inputName), stdin);
		inputName[strcspn(inputName, "\n")] = 0;			// ���๮�� ����

		rfp = OpenFile(inputName, "rb");
		if (rfp != NULL)
			break;
		else if (strcmp(inputName,"3") == 0)	// �Է��� ���� 3 �� ���´ٸ� �Լ� ��ü�� ����.
			return;

		printf("������ �������� �ʽ��ϴ�.\n");
		printf("�������� ���ư��÷��� 3�� �Է� �� �ּ���.\n");
	}

	snprintf(toutputName, sizeof(toutputName), "��ȣ��_%d.txt", fileCntE);
	snprintf(boutputName, sizeof(boutputName), "��ȣ��_%d.bin", fileCntE++);				
	tofp = OpenFile(toutputName,"wt");			// �������� filecntE or D�� �̿��Ͽ� ���� �� ����.
	bofp = OpenFile(boutputName, "wb");			// ��ȣȭ�� ���� bin ������ ���� �Լ�

	if (tofp == NULL || bofp == NULL) {							// ����, ���� �۾��� �Ұ����ϴٸ� �����ִ� ��Ʈ���� �ݾ��ְ� ���ܹ߻� �Լ� ȣ��.
		printf("������ �� �� �������ϴ�.\n");
		fclose(rfp);
		IsException();
	}

	key = GenerateKey();				// ��ȣȭ �̹Ƿ� ���� ����ü���� key�� �޾ƿ��°� �ƴ�, key�� ����.
	//printf("���� KEY: 0x%016llx\n", key);
	buf[8] = '\0';
	paddingByte = 0;						// �е��� ���� ���� ����

	while (1) {
		uint64_t toDES = 0;
		int i;

		for (i = 0; i < 8; ++i) {				// 8����Ʈ ������ �ް� DES �Լ��� �ѱ� 64��Ʈ ���� ����
			int tmp = fgetc(rfp);				// �� �ݺ����� 1����Ʈ�� �����Ƿ�, int�� �̿��ص� ��� x, ++) EOF�� �˱� ���ؼ� uint8_t�� �������ϴٰ� �Ǵ�.

			if (tmp == EOF) {					// ���� �о���̴� ����, EOF�� �����ٸ� ���� ó���ϴ� ����Ʈ�� �ڿ� Padding ó�� ��, i�� ��ġ�� ���� toDES ���� �ڸ� ���� 0���� ó����
				if (i == 0) break;				// �� ���� eof ���� �е��� �� ��Ȳ�̰ų� ������ ������Ƿ� �ٷ� �ݺ� Ż��
				//printf("�е� �߻��� �о���� ���: 0x%016llx\n", toDES);
				paddingByte = Padding(&toDES, i);
				//printf("�е� �� ����Ʈ : %d\n", paddingByte);
	
				if (paddingByte == -1) {
					printf("8 ����Ʈ�� �Ѵ� ���� padding �õ��߽��ϴ�.\n");
					IsException();
				}
				//printf("�е� �߰��� ��� : 0x%016llx\n", toDES);
				break;
			}

			toDES <<= 8;						// 8��Ʈ�� ���� Ȯ�� �� �����͸� or�Ͽ� 64��Ʈ �������� �ջ�
			toDES |= (uint8_t)tmp;				
		}
		
		if (i == 0) {							// ������ ����ų�, ���� �ݺ������� �ٷ� eof�� ���� ���(��� �۾��� �Ϸ�ǰ� fgetc�� �� ȣ��ÿ��� eof���� �������� ����)
			break;								// ���� ��ȣȭ �۾� ����.
		}
		//printf("�о���� ���: 0x%016llx\n", toDES);
		toDES = DES(toDES, key, 'e');			// DES ����
		byteCnt++;								// DES�� ���� �ϸ� Cnt ���
		//printf("��ȣȭ�� ���: 0x%016llx\n", toDES);

		for (i = 7; i >= 0; --i) {
			buf[i] = (char)(toDES & 0xff);
			toDES >>= 8;
		}

		fprintf(tofp, "%s", buf);
		fwrite(buf, sizeof(uint64_t), 1, bofp);
	}

	fclose(rfp);
	fclose(bofp);
	fclose(tofp);		// �̿��� ��Ʈ������ �ݾ��ش�.

	char* tmpName = (char*)calloc(strlen(boutputName) + 1, sizeof(char));			// outputName ������ ��/��ȣȭ �Լ� ����� ������Ƿ� ���������� �̿��� ���� �����ϰ� ����ü�� �Ű��ش�.

	if (tmpName == NULL) {
		printf("���� �޸� �Ҵ� �Ұ���.\n");
		IsException();
	}

	strcpy_s(tmpName, strlen(boutputName) + 1, boutputName);

	info->fileName = tmpName;					// ���� ��ȣȭ �ÿ� �� ����ü�� ������ ����ٸ� ���� ������ �ѹ� �� ��ȣȭ �� �� ������ ���� �� �����Ƿ�, ���� free ������ �ʴ´�.
	info->key = key;
	info->paddingByte = paddingByte;			// ��ȣȭ�� �ʿ��� �������� ����ü�� �ѱ�
	info->byteCnt = byteCnt;
	arrInfo->currCnt++;

	printf("%s �ۼ� �Ϸ�\n",toutputName);
}

void ForDecrpytion(ArrayInfo* arrInfo) {
	char inputName[255];
	char outputName[255];
	char buf[16];		// 64��Ʈ ��ȣȭ ����� ���Ͽ� ���� ���� �ӽ� ����
	int byteCnt = 0;
	int maxCnt;
	DESInfo* info = arrInfo->dataPtr;

	FILE* rfp;
	FILE* ofp;

	uint64_t key;

	int paddingByte;

	while (1) {							// �Է��� ��Ÿ�� �� �� �����Ƿ� ���, �ٸ� ���� �޴��� ���� �� �� �ִ� �������� �ʿ��ϴ� �Ǵ��Ͽ� �߰�.
		printf("���� �̸� �Է� : ");
		fgets(inputName, sizeof(inputName), stdin);
		inputName[strcspn(inputName, "\n")] = 0;			// ���๮�� ����

		rfp = OpenFile(inputName, "rb");

		if (rfp != NULL) {				// ���ο��� ���ϸ�� ���� �� ������ ��ȣȭ �� �� �ִ��� �˻��ϴ� �׸� ����
			int findFlag = 0;

			for (int i = 0; i < arrInfo->currCnt; ++i) {		// �Էµ� ���ϸ��� �����ҽ�, ����ü�� �ش� ���� ���� �����ϴ��� �˻�.
				if (strcmp(info[i].fileName, inputName) == 0) {
					info = &info[i];
					findFlag = 1;
				}
			}													// �� �κ��� �Լ�ȭ �� �� ���� �� ����, ���� ����?

			if (findFlag == 1)
				break;
			else {												// flag �� 0 �� ���, �ش� ���α׷��� ������� �� ������ ������ �ƴϹǷ� ��ȣȭ �� ������ �߻� �� �� �����Ƿ� ��ȣȭ ����.
				printf("������ ����������, ��ȣȭ �� �� �ִ������� �����ϴ�.\n");
				fclose(rfp);
				return;
			}

		}
		else if (strcmp(inputName, "3") == 0)	// �Է��� ���� 3 �� ���´ٸ� �Լ� ��ü�� ����.
			return;

		printf("������ �������� �ʽ��ϴ�.\n");
		printf("�������� ���ư��÷��� 3�� �Է� �� �ּ���.\n");
	}

	snprintf(outputName, sizeof(outputName), "��ȣ��_%d.txt", fileCntD++); // �������� filecntE or D�� �̿��Ͽ� ���� �� ����.
	ofp = OpenFile(outputName, "wt");

	if (ofp == NULL) {							// ����, ���� �۾��� �Ұ����ϴٸ� �����ִ� ��Ʈ���� �ݾ��ְ� ���ܹ߻� �Լ� ȣ��
		printf("������ �� �� �������ϴ�.\n");
		fclose(rfp);
		IsException();
	}

	paddingByte = info->paddingByte;
	maxCnt = info->byteCnt;
	key = info->key;				// ��ȣȭ �̹Ƿ� ���� ����ü���� key�� �޾ƿ�
	//printf("���� KEY: 0x%016llx\n", key);
	buf[8] = '\0';

	while (1) {
		uint64_t toDES = 0;
		int i;

		for (i = 0; i < 8; ++i) {				// 8����Ʈ ������ �ް� DES �Լ��� �ѱ� 64��Ʈ ���� ����
			int tmp = fgetc(rfp);				// �� �ݺ����� 1����Ʈ�� �����Ƿ�, int�� �̿��ص� ��� x, ++) EOF�� �˱� ���ؼ� uint8_t�� �������ϴٰ� �Ǵ�.

			if (tmp == EOF && maxCnt == byteCnt) {					// ���� �о���̴� ����, EOF�� �����ٸ� ���� ó���ϴ� ����Ʈ�� �ڿ� Padding ó�� ��, i�� ��ġ�� ���� toDES ���� �ڸ� ���� 0���� ó����
				if (i == 0) {
					break;
				}
			}

			toDES <<= 8;						// 8��Ʈ�� ���� Ȯ�� �� �����͸� or�Ͽ� 64��Ʈ �������� �ջ�
			toDES |= (uint8_t)tmp;
		}

		if (i == 0) 						// ������ ����ų�, �ٷ� EOF�� ���� ���
			break;							// �� �̻� ���� �����Ͱ� �����Ƿ� ����

		//printf("�о���� ���: 0x%016llx\n", toDES);
		toDES = DES(toDES, key, 'd');			// DES ����
		byteCnt++;
		//printf("��ȣȭ�� ���: 0x%016llx\n", toDES);

		uint64_t tmp = toDES;
		for (int i = 7; i >= 0; --i) {
			buf[i] = (char)(tmp & 0xff);
			tmp >>= 8;
		}

		if (byteCnt == maxCnt  && paddingByte > 0) {
			//printf("�е� �� ����Ʈ : %d\n", paddingByte);
			/*for (int j = 0; j < paddingByte; ++j) {
				toDES >>= 8;
			}*/
			//printf("�е� ������ ���: 0x%016llx\n", toDES);
			buf[8 - paddingByte] = '\0';
			//printf("�е� ���� �� ���� : %s\n", buf);
		}

		fputs(buf, ofp);
	}


	fclose(rfp);
	fclose(ofp);		// �̿��� ��Ʈ������ �ݾ��ش�.

	// ��ȣȭ�� ����ü�� �����͸� �߰�/���� ���� �����Ƿ� ��ȣȭ�� �����ϴ� ���� �Ҵ�, ����ü ������ �̷������ ����
	printf("%s �ۼ� �Ϸ�\n", outputName);

}	

void CheckArraySize(ArrayInfo* arrInfo) {
	int currSize = arrInfo->currCnt;
	int maxSize = arrInfo->maxCnt;
	DESInfo* tmp = arrInfo->dataPtr;

	if (currSize > 0) {
		if (arrInfo->dataPtr[currSize - 1].fileName == NULL) {
			printf("�迭�� ������ ����ִ� ���� ǥ�Ⱑ �ٸ��ϴ�.\n");
			IsException();
		}
	}

	if (currSize >= (maxSize / 2)) {
		tmp = (DESInfo*)calloc(2 * maxSize, sizeof(DESInfo));

		if (tmp == NULL) {
			printf("�迭 �޸� �� �Ҵ� ����.\n");
			IsException();	// �޸� �� ������ �Ұ����ϴٸ� �ٷ� ���α׷��� ����. �̴� ��ȣȭ�� ����� ����ü�� ������ ���ڶ�ٴ� �ǹ��̸� �ٷ� ���α׷��� �����ص� �ȴ�.
		}

		memcpy(tmp, arrInfo->dataPtr, sizeof(DESInfo) * currSize);
		maxSize *= 2;

		arrInfo->maxCnt = maxSize;
		free(arrInfo->dataPtr);
		arrInfo->dataPtr = tmp;
	}

	return;
}

void StartProgram() {
	int input;
	// �迭 ����, �ʱ�ȭ
	DESInfo* DES_info = (DESInfo*)calloc(2,sizeof(DESInfo));
	ArrayInfo* arrayInfo = (ArrayInfo*)malloc(sizeof(ArrayInfo));

	// �ʱ� ���� 2�� �ʱ�ȭ
	arrayInfo->currCnt = 0;
	arrayInfo->maxCnt = 2;		
	arrayInfo->dataPtr = DES_info;

	printf("-----------------------------------------------------\n");
	printf("DES ���� ���α׷�\n");
	printf("\n");

	do {
		printf("1. ��ȣȭ �ϱ�\n");
		printf("2. ��ȣȭ �ϱ�\n");
		printf("3. ���α׷� ����\n");

		scanf_s("%d", &input);
		getchar();

		switch (input)
		{
		case 1:
			ForEncrpytion(arrayInfo);
			CheckArraySize(arrayInfo);
			break;
		case 2:
			ForDecrpytion(arrayInfo);
			CheckArraySize(arrayInfo);
			break;
		case 3:
			printf("���α׷� ����\n");
			break;
		default:
			printf("������ ���� �Է��Դϴ�. �ٽ� �Է� �� �ּ���.\n");
			break;
		}
	} while (input != 3);

	return;
}