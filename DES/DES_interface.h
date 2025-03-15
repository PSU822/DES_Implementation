#pragma once
#include "DES_values.h"

// 에러 호출 함수
void IsException();

// 초기 치환 함수
uint64_t IP(uint64_t text);

// 최종 치환 함수
uint64_t PI(uint64_t text);

// 각 라운드마다 쓰이는 swap 함수
void Swap(uint32_t* R, uint32_t* L, uint32_t forXOR);

// DES 함수 본체. 플래그, 키, 버퍼의 주소를 받아서 암호화 / 복호화
uint64_t DES(uint64_t text, uint64_t key, char mode_flag);

// 패리티 비트를 만듬
uint8_t MakeParity(uint8_t key);

// 64(56+패리티8)bit의 ciper key를 만듬, 
uint64_t GenerateKey();

// 패리티 비트를 제거하고 반환
uint64_t RemoveParity(uint64_t key);

// 56bit의 키를 받아서 각 Round의 48bit의 Roundkey를 만듬
void RoundKeyGenerater(uint64_t key, uint64_t* roundKey);

// DES 함수 내에서 작동할 함수들
// 32 -> 48bit
uint64_t ExpansionPBox(uint32_t R);
// 48 -> 32bit
uint32_t SBox(uint64_t expanR);
// 32 -> 32bit
uint32_t StraightPBox(uint32_t R);

// Mixer, DES 함수라고 부르기도 함
uint64_t Mixer(uint64_t input, uint64_t* roundKey, char mode_flag);

/* --------------------------------------------------------------------------------- */

void IsException() {
	printf("is excption. program out.\n");
	exit(1);
}

uint64_t IP(uint64_t text) {
	int i;
	uint64_t result = 0;
	
	for (i = 0; i < 64; ++i) {
		result <<= 1;
		result |= (text >> (64 - ip[i])) & MASK_64;
	}

	return result;
}

uint64_t PI(uint64_t text) {
	uint64_t result = 0;

	for (int i = 0; i < 64; ++i) {

		result <<= 1;
		result |= (text >> (64 - pi[i])) & MASK_64;

	}

	return result;
}

void Swap(uint32_t* R, uint32_t* L, uint32_t forXOR) {
	uint32_t tmp;

	tmp = *R;															  // tmp를 이용한 swap, 다만 swap 연산 중 R은 L과 xor이 되고, R의 원래 값이 L이 되므로 swap과 동시에 처리
	*R = *L ^ forXOR;
	*L = tmp;
}

uint64_t DES(uint64_t text, uint64_t key, char mode_flag) {

	if (text == NULL || key == NULL || (mode_flag != 'e' && mode_flag != 'd')) {
		printf("입력값이 NULL이거나 KEY가 NULL이거나, mode 오류입니다.\n");
		IsException();
	}
	uint64_t input = IP(text);									// 초기 전치
	uint64_t shortKey = RemoveParity(key);						// 입력받은 키 패리티 비트 제거


	uint64_t roundKey[16];										// 라운드 키 저장용 배열
	RoundKeyGenerater(key, roundKey);							// 라운드 키 생성

	uint64_t output = Mixer(input, roundKey, mode_flag);		// MIXER(DES 함수)
	output = PI(output);										// 최종 전치

	return output;
}
// 7비트 key를 받고 그 key를 이용해 패리티 비트 1 비트를 만든다
uint8_t MakeParity(uint8_t key) {

	uint8_t count = 0;
	do {
		count += key & 1;
		key >>= 1;
	} while (key);

	if (count > 7) {
		printf("7비트 이상의 key가 생성되거나 입력되었습니다.\n");
		IsException();
	}
	count %= 2;
	

	return count;
}

// 0~127(0xef)까지의 수를 key로 만들어내고 패리티 비트와 합침, 그 연산을 반복해서 64비트 key 생성
uint64_t GenerateKey() {

	uint64_t result = 0;
	uint8_t key;
	uint8_t parity;

	for (int i = 0; i < 8; ++i) {
		key = rand() % 128;

		parity = MakeParity(key);

		key <<= 1;
		key |= parity;

		result <<= 8;
		result |= key;
	}

	return result;
}

// 패리티 비트에 기반하여 key 생성
uint64_t RemoveParity(uint64_t key) {
	uint64_t result = 0;

	for (int i = 0; i < 56; i++) {

		result <<= 1;
		result |= (key >> (64 - removeParityBitTable[i])) & MASK_64;

	}
	return result;
}

// 받은 키 정보를 이용하여 16라운드의 라운드 키를 각각 만드는 함수
void RoundKeyGenerater(uint64_t key, uint64_t* roundKey) {
	
	uint64_t result;

	uint32_t C = (uint32_t)((key >> 28) & MASK_28F);		// 키 좌측값 시프트 후 28비트 마스크
	uint32_t D = (uint32_t)(key & MASK_28F);				// 키 우측 값

	for (int i = 0; i < 16; i++) {

		for (int j = 0; j < iterationShiftTable[i]; j++) {                      //roundkeygenerater는 내부에서 시프트와 좌 우 키를 조정하기도 한다.

			C = MASK_28F & (C << 1) | MASK_32 & (C >> 27);
			D = MASK_28F & (D << 1) | MASK_32 & (D >> 27);

		}

		result = 0;
		result = (((uint64_t)C) << 28) | (uint64_t)D;     // C를 좌측으로 쫙 밀고, D와 or 연산하여 합침

		roundKey[i] = 0;

		for (int j = 0; j < 48; j++) {                                      // 56bit 에서 48bit로 줄이는 과정, 이후 각 round 마다 roundkey가 저장됨

			roundKey[i] <<= 1;
			roundKey[i] |= (result >> (56 - keyContractionTable[j])) & MASK_64;

		}

	}
}

uint64_t ExpansionPBox(uint32_t R) {
	uint64_t result = 0;
	for (int i = 0; i < 48; ++i) {                                         // 32비트 -> 48비트 확장 pbox
		result <<= 1;
		result |= (uint64_t)((R >> (32 - expansionPBoxTable[i])) & MASK_32);
	}
	return result;
}

uint32_t SBox(uint64_t expanR) {
	char row, col;
	uint32_t result = 0;

	for (int i = 0; i < 8; ++i) {
		// 00 00 RCCC CR00 00 00 00 00 00 expanR
		// 00 00 1000 0100 00 00 00 00 00 row mask
		// 00 00 0111 1000 00 00 00 00 00 column mask
		
		row = (char)((expanR & (SBOX_ROW_MASK >> 6 * i)) >> 42 - 6 * i);
		row = (row >> 4) | row & 1;

		col = (char)((expanR & (SBOX_COL_MASK >> 6 * i)) >> 43 - 6 * i);

		result <<= 4;
		result |= (uint32_t)(sBoxTable[i][16 * row + col] & 0x0f);     // 32bit 씩 출력되어야 하기 때문에 4bit 씩 마스킹하여 or로 저장

	}

	return result;
}

uint32_t StraightPBox(uint32_t R) {
	uint32_t result = 0;

	for (int i = 0; i < 32; ++i) {										   // SBOX 이후 결과물을 테이블을 이용하여 32비트 전치
		result <<= 1;
		result |= (R >> (32 - straightPBoxTable[i])) & MASK_32;
	}

	return result;
}

uint64_t Mixer(uint64_t input, uint64_t* roundKey, char mode_flag) {

	uint32_t L = (uint32_t)(input >> 32) & MASK_32F;					 // 좌측 값을 살리기 위해 32번 > 시프트 후 마스크를 씌워 값을 날린다.
	uint32_t R = (uint32_t)input & MASK_32F;								// 우측 값을 살리기 위함이고, 우측값은 굳이 시프트 할 필요가 없기에 이렇게 처리한다.

	for (int i = 0; i < 16; ++i) {
		uint64_t expanR = ExpansionPBox(R);									// 32-> 48bit. 기본 R의 경우에는 나중에 swap 과정에서 사용되므로 값을 살려둔다.

		if (mode_flag == 'd')												// decode, encrytion 선택
			expanR = expanR ^ roundKey[15 - i];								// 라운드의 역연산이므로 16라운드(index 15부터 시작)

		else
			expanR = expanR ^ roundKey[i];

		uint32_t afterSbox = SBox(expanR);									// 48 -> 32bit
		afterSbox = StraightPBox(afterSbox);

		Swap(&R, &L, afterSbox);
	}

	uint64_t result = (((uint64_t)R) << 32) | (uint64_t)L;

	return result;															// 모든 반복이 끝났다면 이 값은 암호화, 혹은 복호화가 완료된 값이므로 리턴해도 상관없음
}
