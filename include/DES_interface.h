#pragma once
#include "DES_values.h"

// ���� ȣ�� �Լ�
void IsException();

// �ʱ� ġȯ �Լ�
uint64_t IP(uint64_t text);

// ���� ġȯ �Լ�
uint64_t PI(uint64_t text);

// �� ���帶�� ���̴� swap �Լ�
void Swap(uint32_t* R, uint32_t* L, uint32_t forXOR);

// DES �Լ� ��ü. �÷���, Ű, ������ �ּҸ� �޾Ƽ� ��ȣȭ / ��ȣȭ
uint64_t DES(uint64_t text, uint64_t key, char mode_flag);

// �и�Ƽ ��Ʈ�� ����
uint8_t MakeParity(uint8_t key);

// 64(56+�и�Ƽ8)bit�� ciper key�� ����, 
uint64_t GenerateKey();

// �и�Ƽ ��Ʈ�� �����ϰ� ��ȯ
uint64_t RemoveParity(uint64_t key);

// 56bit�� Ű�� �޾Ƽ� �� Round�� 48bit�� Roundkey�� ����
void RoundKeyGenerater(uint64_t key, uint64_t* roundKey);

// DES �Լ� ������ �۵��� �Լ���
// 32 -> 48bit
uint64_t ExpansionPBox(uint32_t R);
// 48 -> 32bit
uint32_t SBox(uint64_t expanR);
// 32 -> 32bit
uint32_t StraightPBox(uint32_t R);

// Mixer, DES �Լ���� �θ��⵵ ��
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

	tmp = *R;															  // tmp�� �̿��� swap, �ٸ� swap ���� �� R�� L�� xor�� �ǰ�, R�� ���� ���� L�� �ǹǷ� swap�� ���ÿ� ó��
	*R = *L ^ forXOR;
	*L = tmp;
}

uint64_t DES(uint64_t text, uint64_t key, char mode_flag) {

	if (text == NULL || key == NULL || (mode_flag != 'e' && mode_flag != 'd')) {
		printf("�Է°��� NULL�̰ų� KEY�� NULL�̰ų�, mode �����Դϴ�.\n");
		IsException();
	}
	uint64_t input = IP(text);									// �ʱ� ��ġ
	uint64_t shortKey = RemoveParity(key);						// �Է¹��� Ű �и�Ƽ ��Ʈ ����


	uint64_t roundKey[16];										// ���� Ű ����� �迭
	RoundKeyGenerater(key, roundKey);							// ���� Ű ����

	uint64_t output = Mixer(input, roundKey, mode_flag);		// MIXER(DES �Լ�)
	output = PI(output);										// ���� ��ġ

	return output;
}
// 7��Ʈ key�� �ް� �� key�� �̿��� �и�Ƽ ��Ʈ 1 ��Ʈ�� �����
uint8_t MakeParity(uint8_t key) {

	uint8_t count = 0;
	do {
		count += key & 1;
		key >>= 1;
	} while (key);

	if (count > 7) {
		printf("7��Ʈ �̻��� key�� �����ǰų� �ԷµǾ����ϴ�.\n");
		IsException();
	}
	count %= 2;
	

	return count;
}

// 0~127(0xef)������ ���� key�� ������ �и�Ƽ ��Ʈ�� ��ħ, �� ������ �ݺ��ؼ� 64��Ʈ key ����
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

// �и�Ƽ ��Ʈ�� ����Ͽ� key ����
uint64_t RemoveParity(uint64_t key) {
	uint64_t result = 0;

	for (int i = 0; i < 56; i++) {

		result <<= 1;
		result |= (key >> (64 - removeParityBitTable[i])) & MASK_64;

	}
	return result;
}

// ���� Ű ������ �̿��Ͽ� 16������ ���� Ű�� ���� ����� �Լ�
void RoundKeyGenerater(uint64_t key, uint64_t* roundKey) {
	
	uint64_t result;

	uint32_t C = (uint32_t)((key >> 28) & MASK_28F);		// Ű ������ ����Ʈ �� 28��Ʈ ����ũ
	uint32_t D = (uint32_t)(key & MASK_28F);				// Ű ���� ��

	for (int i = 0; i < 16; i++) {

		for (int j = 0; j < iterationShiftTable[i]; j++) {                      //roundkeygenerater�� ���ο��� ����Ʈ�� �� �� Ű�� �����ϱ⵵ �Ѵ�.

			C = MASK_28F & (C << 1) | MASK_32 & (C >> 27);
			D = MASK_28F & (D << 1) | MASK_32 & (D >> 27);

		}

		result = 0;
		result = (((uint64_t)C) << 28) | (uint64_t)D;     // C�� �������� �� �а�, D�� or �����Ͽ� ��ħ

		roundKey[i] = 0;

		for (int j = 0; j < 48; j++) {                                      // 56bit ���� 48bit�� ���̴� ����, ���� �� round ���� roundkey�� �����

			roundKey[i] <<= 1;
			roundKey[i] |= (result >> (56 - keyContractionTable[j])) & MASK_64;

		}

	}
}

uint64_t ExpansionPBox(uint32_t R) {
	uint64_t result = 0;
	for (int i = 0; i < 48; ++i) {                                         // 32��Ʈ -> 48��Ʈ Ȯ�� pbox
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
		result |= (uint32_t)(sBoxTable[i][16 * row + col] & 0x0f);     // 32bit �� ��µǾ�� �ϱ� ������ 4bit �� ����ŷ�Ͽ� or�� ����

	}

	return result;
}

uint32_t StraightPBox(uint32_t R) {
	uint32_t result = 0;

	for (int i = 0; i < 32; ++i) {										   // SBOX ���� ������� ���̺��� �̿��Ͽ� 32��Ʈ ��ġ
		result <<= 1;
		result |= (R >> (32 - straightPBoxTable[i])) & MASK_32;
	}

	return result;
}

uint64_t Mixer(uint64_t input, uint64_t* roundKey, char mode_flag) {

	uint32_t L = (uint32_t)(input >> 32) & MASK_32F;					 // ���� ���� �츮�� ���� 32�� > ����Ʈ �� ����ũ�� ���� ���� ������.
	uint32_t R = (uint32_t)input & MASK_32F;								// ���� ���� �츮�� �����̰�, �������� ���� ����Ʈ �� �ʿ䰡 ���⿡ �̷��� ó���Ѵ�.

	for (int i = 0; i < 16; ++i) {
		uint64_t expanR = ExpansionPBox(R);									// 32-> 48bit. �⺻ R�� ��쿡�� ���߿� swap �������� ���ǹǷ� ���� ����д�.

		if (mode_flag == 'd')												// decode, encrytion ����
			expanR = expanR ^ roundKey[15 - i];								// ������ �������̹Ƿ� 16����(index 15���� ����)

		else
			expanR = expanR ^ roundKey[i];

		uint32_t afterSbox = SBox(expanR);									// 48 -> 32bit
		afterSbox = StraightPBox(afterSbox);

		Swap(&R, &L, afterSbox);
	}

	uint64_t result = (((uint64_t)R) << 32) | (uint64_t)L;

	return result;															// ��� �ݺ��� �����ٸ� �� ���� ��ȣȭ, Ȥ�� ��ȣȭ�� �Ϸ�� ���̹Ƿ� �����ص� �������
}
