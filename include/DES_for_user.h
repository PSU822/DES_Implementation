#pragma once
#include "DES_interface.h"
#include <string.h>

typedef struct ArrayInfo {
	int currCnt;
	int maxCnt;
	DESInfo* dataPtr;
}ArrayInfo;

static int fileCntE = 1, fileCntD= 1;

// 유저 인터페이스 프로그램
void StartProgram();

// 파일 입출력 함수
FILE* OpenFile(char* filename, char* mode);					// 파일 포인터를 처리해주는 간단한 함수
void ForEncrpytion(ArrayInfo* arrInfo);
void ForDecrpytion(ArrayInfo* arrInfo);

// 배열 관리 함수
void CheckArraySize(ArrayInfo* arrInfo);					// 배열을 여러개 만든다면 다른 데이터 구조로 처리할 가능성이 있으므로 포인터를 받게 설계

// 직접적으로 DES에 영향을 주지 않아서 이 곳에서 선언 
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

int Padding(uint64_t* text, int currbyte) {		// 현재 들어온 i값을 이용하여 남은 바이트 계산 후 패딩처리.

	if(currbyte > 8)	// 들어온 값이 8 이상이라면, 무언가 문제가 발생 한 상태이므로 -1을 반환
		return -1;
	
	int remainByte = 8 - currbyte;
	for (int i = 0; i < remainByte; ++i)
		*text = (*text << 8) | 0x00;			// 패딩을 명시적으로 하지 않으면 이상한 값을 토해냄

	return remainByte;							// 구조체에 반환하기 위한 패딩한 바이트 정보 리턴
}

void ForEncrpytion(ArrayInfo* arrInfo) {		// DES info를 저장하는 배열을 호출, key와 padding 바이트, 파일 이름을 관리함.
	char inputName[255];
	char toutputName[255];
	char boutputName[255];

	char buf[9];		// 64비트 암호화 결과를 파일에 쓰기 위한 임시 버퍼
	int byteCnt = 0;

	DESInfo* info = &arrInfo->dataPtr[arrInfo->currCnt];

	FILE* rfp;
	FILE* tofp;
	FILE* bofp;

	uint64_t key;

	int paddingByte = 0;

	while (1) {							// 입력의 오타를 낼 수 있으므로 고려, 다만 이전 메뉴로 돌아 갈 수 있는 선택지도 필요하다 판단하여 추가.
		printf("파일 이름 입력 : ");
		fgets(inputName, sizeof(inputName), stdin);
		inputName[strcspn(inputName, "\n")] = 0;			// 개행문자 제거

		rfp = OpenFile(inputName, "rb");
		if (rfp != NULL)
			break;
		else if (strcmp(inputName,"3") == 0)	// 입력이 단일 3 만 들어온다면 함수 자체를 종료.
			return;

		printf("파일이 존재하지 않습니다.\n");
		printf("이전으로 돌아가시려면 3을 입력 해 주세요.\n");
	}

	snprintf(toutputName, sizeof(toutputName), "암호문_%d.txt", fileCntE);
	snprintf(boutputName, sizeof(boutputName), "암호문_%d.bin", fileCntE++);				
	tofp = OpenFile(toutputName,"wt");			// 전역변수 filecntE or D를 이용하여 파일 명 관리.
	bofp = OpenFile(boutputName, "wb");			// 복호화시 쓰일 bin 파일을 쓰는 함수

	if (tofp == NULL || bofp == NULL) {							// 만일, 쓰기 작업이 불가능하다면 열려있는 스트림을 닫아주고 예외발생 함수 호출.
		printf("파일을 열 수 없었습니다.\n");
		fclose(rfp);
		IsException();
	}

	key = GenerateKey();				// 암호화 이므로 따로 구조체에서 key를 받아오는게 아닌, key를 생성.
	//printf("읽은 KEY: 0x%016llx\n", key);
	buf[8] = '\0';
	paddingByte = 0;						// 패딩을 위한 정보 변수

	while (1) {
		uint64_t toDES = 0;
		int i;

		for (i = 0; i < 8; ++i) {				// 8바이트 정보를 받고 DES 함수에 넘길 64비트 정보 생성
			int tmp = fgetc(rfp);				// 각 반복마다 1바이트씩 받으므로, int를 이용해도 상관 x, ++) EOF를 알기 위해선 uint8_t는 부적합하다고 판단.

			if (tmp == EOF) {					// 만일 읽어들이는 도중, EOF를 만난다면 현재 처리하던 바이트의 뒤에 Padding 처리 후, i의 위치를 보고 toDES 변수 뒤를 전부 0으로 처리함
				if (i == 0) break;				// 이 경우는 eof 이후 패딩을 한 상황이거나 파일이 비었으므로 바로 반복 탈출
				//printf("패딩 발생한 읽어들인 블록: 0x%016llx\n", toDES);
				paddingByte = Padding(&toDES, i);
				//printf("패딩 된 바이트 : %d\n", paddingByte);
	
				if (paddingByte == -1) {
					printf("8 바이트를 넘는 값을 padding 시도했습니다.\n");
					IsException();
				}
				//printf("패딩 추가한 블록 : 0x%016llx\n", toDES);
				break;
			}

			toDES <<= 8;						// 8비트의 공간 확보 후 데이터를 or하여 64비트 정보까지 합산
			toDES |= (uint8_t)tmp;				
		}
		
		if (i == 0) {							// 파일이 비었거나, 위의 반복문에서 바로 eof를 만난 경우(모든 작업이 완료되고 fgetc를 또 호출시에는 eof에서 움직이지 않음)
			break;								// 따라서 암호화 작업 종료.
		}
		//printf("읽어들인 블록: 0x%016llx\n", toDES);
		toDES = DES(toDES, key, 'e');			// DES 진행
		byteCnt++;								// DES를 진행 하면 Cnt 상승
		//printf("암호화된 블록: 0x%016llx\n", toDES);

		for (i = 7; i >= 0; --i) {
			buf[i] = (char)(toDES & 0xff);
			toDES >>= 8;
		}

		fprintf(tofp, "%s", buf);
		fwrite(buf, sizeof(uint64_t), 1, bofp);
	}

	fclose(rfp);
	fclose(bofp);
	fclose(tofp);		// 이용한 스트림들을 닫아준다.

	char* tmpName = (char*)calloc(strlen(boutputName) + 1, sizeof(char));			// outputName 변수는 암/복호화 함수 종료시 사라지므로 동적선언을 이용해 값을 복사하고 구조체에 옮겨준다.

	if (tmpName == NULL) {
		printf("동적 메모리 할당 불가능.\n");
		IsException();
	}

	strcpy_s(tmpName, strlen(boutputName) + 1, boutputName);

	info->fileName = tmpName;					// 이후 복호화 시에 이 구조체의 내용을 지운다면 같은 파일을 한번 더 복호화 할 때 문제가 생길 수 있으므로, 따로 free 해주진 않는다.
	info->key = key;
	info->paddingByte = paddingByte;			// 복호화에 필요한 정보들을 구조체에 넘김
	info->byteCnt = byteCnt;
	arrInfo->currCnt++;

	printf("%s 작성 완료\n",toutputName);
}

void ForDecrpytion(ArrayInfo* arrInfo) {
	char inputName[255];
	char outputName[255];
	char buf[16];		// 64비트 암호화 결과를 파일에 쓰기 위한 임시 버퍼
	int byteCnt = 0;
	int maxCnt;
	DESInfo* info = arrInfo->dataPtr;

	FILE* rfp;
	FILE* ofp;

	uint64_t key;

	int paddingByte;

	while (1) {							// 입력의 오타를 낼 수 있으므로 고려, 다만 이전 메뉴로 돌아 갈 수 있는 선택지도 필요하다 판단하여 추가.
		printf("파일 이름 입력 : ");
		fgets(inputName, sizeof(inputName), stdin);
		inputName[strcspn(inputName, "\n")] = 0;			// 개행문자 제거

		rfp = OpenFile(inputName, "rb");

		if (rfp != NULL) {				// 내부에서 파일명과 현재 이 파일을 복호화 할 수 있는지 검사하는 항목 여부
			int findFlag = 0;

			for (int i = 0; i < arrInfo->currCnt; ++i) {		// 입력된 파일명이 존재할시, 구조체에 해당 파일 명이 존재하는지 검색.
				if (strcmp(info[i].fileName, inputName) == 0) {
					info = &info[i];
					findFlag = 1;
				}
			}													// 이 부분은 함수화 할 수 있을 것 같음, 추후 수정?

			if (findFlag == 1)
				break;
			else {												// flag 가 0 인 경우, 해당 프로그램이 실행됐을 때 생성된 파일이 아니므로 복호화 시 문제가 발생 할 수 있으므로 복호화 종료.
				printf("파일은 존재하지만, 복호화 할 수 있는정보가 없습니다.\n");
				fclose(rfp);
				return;
			}

		}
		else if (strcmp(inputName, "3") == 0)	// 입력이 단일 3 만 들어온다면 함수 자체를 종료.
			return;

		printf("파일이 존재하지 않습니다.\n");
		printf("이전으로 돌아가시려면 3을 입력 해 주세요.\n");
	}

	snprintf(outputName, sizeof(outputName), "복호문_%d.txt", fileCntD++); // 전역변수 filecntE or D를 이용하여 파일 명 관리.
	ofp = OpenFile(outputName, "wt");

	if (ofp == NULL) {							// 만일, 쓰기 작업이 불가능하다면 열려있는 스트림을 닫아주고 예외발생 함수 호출
		printf("파일을 열 수 없었습니다.\n");
		fclose(rfp);
		IsException();
	}

	paddingByte = info->paddingByte;
	maxCnt = info->byteCnt;
	key = info->key;				// 복호화 이므로 따로 구조체에서 key를 받아옴
	//printf("읽은 KEY: 0x%016llx\n", key);
	buf[8] = '\0';

	while (1) {
		uint64_t toDES = 0;
		int i;

		for (i = 0; i < 8; ++i) {				// 8바이트 정보를 받고 DES 함수에 넘길 64비트 정보 생성
			int tmp = fgetc(rfp);				// 각 반복마다 1바이트씩 받으므로, int를 이용해도 상관 x, ++) EOF를 알기 위해선 uint8_t는 부적합하다고 판단.

			if (tmp == EOF && maxCnt == byteCnt) {					// 만일 읽어들이는 도중, EOF를 만난다면 현재 처리하던 바이트의 뒤에 Padding 처리 후, i의 위치를 보고 toDES 변수 뒤를 전부 0으로 처리함
				if (i == 0) {
					break;
				}
			}

			toDES <<= 8;						// 8비트의 공간 확보 후 데이터를 or하여 64비트 정보까지 합산
			toDES |= (uint8_t)tmp;
		}

		if (i == 0) 						// 파일이 비었거나, 바로 EOF를 만난 경우
			break;							// 더 이상 읽을 데이터가 없으므로 종료

		//printf("읽어들인 블록: 0x%016llx\n", toDES);
		toDES = DES(toDES, key, 'd');			// DES 진행
		byteCnt++;
		//printf("복호화된 블록: 0x%016llx\n", toDES);

		uint64_t tmp = toDES;
		for (int i = 7; i >= 0; --i) {
			buf[i] = (char)(tmp & 0xff);
			tmp >>= 8;
		}

		if (byteCnt == maxCnt  && paddingByte > 0) {
			//printf("패딩 된 바이트 : %d\n", paddingByte);
			/*for (int j = 0; j < paddingByte; ++j) {
				toDES >>= 8;
			}*/
			//printf("패딩 제거한 블록: 0x%016llx\n", toDES);
			buf[8 - paddingByte] = '\0';
			//printf("패딩 제거 후 내용 : %s\n", buf);
		}

		fputs(buf, ofp);
	}


	fclose(rfp);
	fclose(ofp);		// 이용한 스트림들을 닫아준다.

	// 복호화는 구조체에 데이터를 추가/삭제 하지 않으므로 암호화시 존재하던 각종 할당, 구조체 수정이 이루어지지 않음
	printf("%s 작성 완료\n", outputName);

}	

void CheckArraySize(ArrayInfo* arrInfo) {
	int currSize = arrInfo->currCnt;
	int maxSize = arrInfo->maxCnt;
	DESInfo* tmp = arrInfo->dataPtr;

	if (currSize > 0) {
		if (arrInfo->dataPtr[currSize - 1].fileName == NULL) {
			printf("배열에 실제로 들어있는 값과 표기가 다릅니다.\n");
			IsException();
		}
	}

	if (currSize >= (maxSize / 2)) {
		tmp = (DESInfo*)calloc(2 * maxSize, sizeof(DESInfo));

		if (tmp == NULL) {
			printf("배열 메모리 재 할당 실패.\n");
			IsException();	// 메모리 재 설정이 불가능하다면 바로 프로그램을 종료. 이는 암호화시 생기는 구조체의 공간이 모자라다는 의미이며 바로 프로그램을 종료해도 된다.
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
	// 배열 생성, 초기화
	DESInfo* DES_info = (DESInfo*)calloc(2,sizeof(DESInfo));
	ArrayInfo* arrayInfo = (ArrayInfo*)malloc(sizeof(ArrayInfo));

	// 초기 값은 2로 초기화
	arrayInfo->currCnt = 0;
	arrayInfo->maxCnt = 2;		
	arrayInfo->dataPtr = DES_info;

	printf("-----------------------------------------------------\n");
	printf("DES 구현 프로그램\n");
	printf("\n");

	do {
		printf("1. 암호화 하기\n");
		printf("2. 복호화 하기\n");
		printf("3. 프로그램 종료\n");

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
			printf("프로그램 종료\n");
			break;
		default:
			printf("허용되지 않은 입력입니다. 다시 입력 해 주세요.\n");
			break;
		}
	} while (input != 3);

	return;
}