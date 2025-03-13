#include "DES_for_user.h"

// 우선 각 함수들이 제대로 동작하는지 테스트, 제일 단순한 초기 - 최종 전치 함수부터 테스트함.
// 기본적으로 DES 함수에 넘겨 줄 사용자 인터페이스 함수에서 입력이 NULL인지 아닌지 판단하겠지만, 혹시 모를 상황과
// 개발자가 이 라이브러리를 사용 하였을 때 DES 함수를 다르게 활용하여 인터페이스를 만든다고 가정 시, DES 함수에 NULL값이 들어가면 곤란하므로
// DES 함수 자체에 예외처리를 해둔다.
void Test_1() {
	uint64_t test_input = 0x9474B8E8C73BCA7D;

	printf("입력값 : 0x%llx \n", test_input);
	test_input = IP(test_input);

	printf("초기 전치 이후 : 0x%llx \n", test_input);
	test_input = PI(test_input);

	printf("최종 전치 이후 : 0x%llx \n", test_input);
}// 문제없이 작동 함, 전치 - 전치 과정에서 데이터 손실이 나지도 않음.

// 제일 단순한 전치가 문제가 없으므로, 다음 과정을 빠르게 넘어감.
// 이 프로그램은 64비트의 랜덤 키값을 만드는 과정에서 패리티 비트까지 만들어 내는 키를 생성함.
// 즉, 8비트가 있다면 제일 앞 비트에 패리티 비트를 넣고(패리티 비트 제거 표에 의해 만들어진 형태.) 진짜 정보 7비트가 들어가며
// 총 64비트가 만들어짐.
// 만들어지는 키와 키의 패리티 비트를 제거하는 과정을 테스트.
void Test_2() {
	uint64_t key = GenerateKey();
	uint64_t tmp = key;
	// 패리티 비트가 맞는가 검증
	for (int i = 0; i < 8; ++i) {
		for (int j = 0; j < 8; ++j) {
			int b = tmp & 1;
			printf("%d",b);
			tmp >>= 1;
		}
		printf(" ");
	}
	printf("\n");
	printf("만들어지는 키 : 0x%llx \n", key);

	// 패리티 비트가 제거되는지 검증
	key = RemoveParity(key);
	tmp = key;
	for (int i = 0; i < 8; ++i) {
		for (int j = 0; j < 8; ++j) {
			int b = tmp & 1;
			printf("%d", b);
			tmp >>= 1;
		}
		printf(" ");
	}

	printf("\n");
	printf("패리티 비트 제거 후 : 0x%llx \n", key);
	// 패리티 비트는 정상적으로 만들어졌으나, 제거 표의 매커니즘이 일반적인 패리티 비트를 제거하는 느낌이 아님, 어찌됐건 key를 만드는 과정에선 문제가 없기에 이어서 진행함.
	// 이어서 RoundKey를 만듬.

	uint64_t roundKey[16];
	RoundKeyGenerater(key, roundKey);

	for (int i = 0; i < 16; ++i) {
		printf("roundKey %d : 0x%llx\n", i + 1, roundKey[i]);
	}
	
} // 각 Round 마다 다른 key를 도출 하는 것 까지 검증완료.

// 키, 전치의 작동이 확인되었으며, 이제 DES 함수에 들어갈 함수들을 테스트 하는 단계.
// SBOX, PBOX의 동작만 균일성있게 나타난다면 DES 함수 자체가 위의 함수의 연속적인 사용을 가정하고 만든 함수이므로
// DES의 작동에는 문제가 없다고 판단.
void Test_3() {
	uint32_t L = 0x9474B8E8;
	uint32_t R = 0xC73BCA7D;
	uint64_t expanR = ExpansionPBox(R);
	printf("확장 전 : 0x%lx\n", R);
	printf("확장 후 : 0x%llx\n", expanR);
	//48비트로 확장되어 나오는 것을 확인 할 수 있다.

	uint32_t afterSbox = SBox(expanR);
	printf("SBOX 결과값 : 0x%lx\n", afterSbox);
	// 다시 sbox를 통해 32비트의 값이 잘 출력 되는 것을 볼 수 있다.
	afterSbox = StraightPBox(afterSbox);
	printf("마지막 PBOX : 0x%lx\n", afterSbox);
	// 인풋을 하드코딩하여 결과가 바뀌지 않는 것을 확인했고, 이는 중간중간 이루어질 roundkey ^ expanR, ^ L 에서 문제 없이 연산됨을 나타낸다.

	Swap(&R, &L, afterSbox);
	printf("R : %lx\nL : %lx\n",R,L);
	// swap 이후 값들이 R은 이전 L, L은 mixer 의 결과값과 이전 L을 xor 한 결과를 제대로 출력해준다.
} //MIXER 함수의 내부 함수들이 문제가 없는 것을 확인하였고, 이를 통해 mixer의 작동도 올바를 것으로 확인되었다.

// 여기서 부턴 DES의 예외를 처리하기 보단, DES 함수에 전달하기 위한 입/출력 함수를 테스트 하는 코드로 전환된다.
// 현재 소스파일의 첫 오류 처리이자 필터 역할을 하는 함수와, 파일을 만들고 없애는 코드이므로 예외상황을 이 쪽에서 대다수 처리하는 것으로 설계했다.
// 제일 단순한 padding 처리를 위한 함수를 테스트
void Test_4() {
	uint64_t toPadding = 0x9474B8E8;
	int paddingByte = Padding(&toPadding, 4);
	printf("패딩된 byte : %d\n패딩된 결과 : 0x%llx\n", paddingByte, toPadding);
}// 만일의 상황에 대비해서 8바이트 이상의 값이 매개변수로 들어갈 때 예외처리를 해두었다. padding 함수 자체에 쓰이진않았지만, padding byte 자체가 -1값이 들어가며, 패딩이 진행되지 않는다.

// 제일 큰 함수 둘인 암/복호화 함수의 경우 string을 다루며, 동적 할당까지 다루기에 설계를 하며 여러 예외를 고려했다
// 이전 전치 암호에서도 사용했던 배열 크기 재할당 함수는 로직을 그대로 이용했지만, 중간에 생길 수 있는 예외를 더 깔끔하게 처리했다
// 이전 로직은 새로 할당하고 복사하는 과정을 거쳤지만, reollc를 이용하고 tmp 변수를 이용해 에러를 탐지할 수 있게 새롭게 설계했다

void Test_5() {
	DESInfo* DES_info = (DESInfo*)calloc( 2, sizeof(DESInfo));
	ArrayInfo* arrayInfo = (ArrayInfo*)malloc(sizeof(ArrayInfo));
	int arrsize;

	// 초기 값은 2로 초기화
	//arrayInfo->currCnt = 2;	// 실제론 배열이 비었지만, 테스트를 위해 2를 넣었다.
	arrayInfo->currCnt = 0;
	arrayInfo->maxCnt = 2;
	arrayInfo->dataPtr = DES_info;

	printf("최대 arrsize : %d\n", arrayInfo->maxCnt);
	CheckArraySize(arrayInfo);
	printf("최대 arrsize : %d\n", arrayInfo->maxCnt);

}
// CheckArrySize 함수는 작동 테스트 이후, currcnt의 위치에 실제 정보가 존재하는지 검사하는 로직을 추가하였다. 주석 처리된 코드를 이용하면 예외를 발생시킨다

void Test_6() {
	DESInfo* DES_info = (DESInfo*)calloc(2, sizeof(DESInfo));
	ArrayInfo* arrayInfo = (ArrayInfo*)malloc(sizeof(ArrayInfo));
	int arrsize;

	// 초기 값은 2로 초기화
	//arrayInfo->currCnt = 2;	// 실제론 배열이 비었지만, 테스트를 위해 2를 넣었다.
	arrayInfo->currCnt = 0;
	arrayInfo->maxCnt = 2;
	arrayInfo->dataPtr = DES_info;

	ForEncrpytion(arrayInfo);
	ForDecrpytion(arrayInfo);
}
// 암/복호화의 파일 명을 검색하는 부분은 공통되는 부분이 존재하지만, 구조체를 검색하느냐, 안 하느냐를 또 검사조건으로 달기에 따로 함수화 하지않고 함수 안에 로직으로 구현했다
// 그리고 유저 interface로 지정한 함수인 만큼, 프로그래머가 DES 자체를 라이브러리로 사용한다고 생각했을 때, 이용할 것 같다는 느낌을 받지 못 하여
// 독자적인 큰 함수 두개로 처리한다는 느낌으로 구현했다
// 동적 메모리들은 프로그램 규모를 보았을 때 딱히 할당 해제하지 않아도 프로그램 종료 시 한꺼번에 해결되도록 일부러 할당 해제를 하지 않았다. 
// 명시적인 free가 필요한 영역을 생각 해 보면 파일 명을 저장하는 DESInfo의 fileName, 그리고 DESInfo* dataPtr 등이 있고, 이를 할당 해제 하려면
// for 문과 currCnt를 이용하면 쉽게 가능하다고 생각된다

// 발생한 예외 상황들을 요약하자면, 예외가 발생 했다면 열려있던 스트림이 닫히지 않았던 문제, 예외를 뱉을 때 열려있던 스트림을 같이 닫게 수정하였다
// 문자열과 동적 할당에 실수가 있어 임시로 동적할 당 한 뒤, 구조체에 그 주소를 넘기는 것으로 문제를 해결했다

// 파일이 비어있을 때 무한 루프를 방지하기 위해 i의 값이 반복 문 내에서 계속 유지되게 하여 이것을 이용해 빈파일 or eof시 루프를 벗어나게 수정했다
// 파일 명이 일치하지 않을 때 메인메뉴로 나가지 않고 선택지를 구현했다

// Final Test중, 암호문/복호문의 길이가 일정 길이로 길어지면 복호화가 중간에 멈추거나 이상하게 복호화되는 현상이 발견되었다
// txt 파일을 읽는 도중 암호문이 00 00이 반복되면 null 값으로 인식하여 더 이상 파일의 내용을 읽지 못 하는 현상을 확인하고
// txt, bin 파일을 같이 생성하여 온전한 binary 파일을 읽고 복호화 하게 설계를 수정하였다
// 이진 형식으로 txt에 저장할 시 ascii 코드를 다시 16진수로 변환하는 문제, txt 파일에서 일부 정보가 누락되는 문제를 txt로 처리하기 보단
// bin 파일을 이용하는게 낫다고 판단했다.


/* --------------------------------------------------------------------------------- */

/*int main() {
	srand((unsigned)time(NULL));

	//Test_1();
	//Test_2();
	//Test_3();
	//Test_4();
	//Test_5();
	//Test_6();
	//StartProgram();
	return 0;
}*/