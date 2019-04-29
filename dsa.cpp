#include "dsa.h"

uint32 divide_2(uint8 x[MAX_STR_LEN])
{
	uint32 temp = 0;

	for (int i = MAX_STR_LEN - 1; i >= 0; i--)
	{
		temp *= 10;
		temp += x[i];
		x[i] = temp / 2;
		temp %= 2;
	}
	return temp;
}

void str_to_32xuint32(string input, uint32 output[32])
{
	uint8 x[MAX_STR_LEN];
	int length = input.length();

	for (int i = 0; i < length; i++) x[i] = input[length - 1 - i] - '0';
	for (int i = length; i < MAX_STR_LEN; i++) x[i] = 0;

	for (int i = 0; i < 32; i++)
	{
		output[i] = 0;
		for (int j = 0; j < 32; j++) output[i] |= (divide_2(x) << j);
	}
}

// return (a >= b)
bool compare_64xuint32(uint32 a[64], uint32 b[64])
{
	for (int i = 63; i >= 0; i--)
	{
		if (a[i] > b[i]) return true;
		if (a[i] < b[i]) return false;
	}
	return true;
}

// rst = (a * b) mod n
void multiply_32xuint32_modn(uint32 rst[32], uint32 a[32], uint32 b[32], uint32 n[32])
{
	uint32 temp1[64];
	uint32 temp2[64];
	uint64 mul;

	for (int i = 0; i < 64; i++) temp1[i] = 0;
	for (int i = 0; i < 32; i++)
	{
		mul = 0;
		for (int j = 0; j < 64; j++) temp2[j] = 0;
		for (int j = 0; j < 32; j++)
		{
			mul += uint64(a[i]) * uint64(b[j]);
			temp2[i + j] = mul % SCALE;
			mul /= SCALE;
		}
		temp2[i + 32] = mul;
		add_64xuint32(temp1, temp1, temp2);
	}

	mod_uint32(rst, temp1, n);
}

// rst = a mod n
void mod_uint32(uint32 rst[32], uint32 a[64], uint32 n[32])
{
	uint32 aa[64];
	uint32 bb[64];

	for (int i = 0; i < 32; i++) bb[i] = n[i];
	for (int i = 32; i < 64; i++) bb[i] = 0;
	for (int i = 0; i < 64; i++) aa[i] = a[i];

	while (compare_64xuint32(aa, bb)) subtract_64xuint32(aa, aa, bb);
	for (int i = 0; i < 32; i++) rst[i] = aa[i];
}

// rst = a + b (a > b)
void add_64xuint32(uint32 rst[64], uint32 a[64], uint32 b[64])
{
	uint64 aa;
	uint64 bb;
	uint64 temp = 0;

	for (int i = 0; i < 64; i++)
	{
		aa = a[i];
		bb = b[i];
		aa += (temp + bb);
		temp = aa / SCALE;
		aa %= SCALE;
		rst[i] = uint32(aa);
	}
}

// rst = a - b (a > b)
void subtract_64xuint32(uint32 rst[64], uint32 a[64], uint32 b[64])
{
	uint64 aa;
	uint64 bb;
	uint64 temp = 0;

	for (int i = 0; i < 64; i++)
	{
		aa = a[i] - temp;
		bb = b[i];
		temp = 0;
		while (aa < bb)
		{
			temp += 1;
			aa += SCALE;
		}
		rst[i] = uint32(aa - bb);
	}
}

void divide_2_32xuint32(uint32 x[32])
{
	uint32 temp1 = 0, temp2 = 0;

	for (int i = 31; i >= 0; i--)
	{
		temp1 = x[i] & 0x00000001;
		x[i] >>= 1;
		x[i] |= (temp2 << 31);
		temp2 = temp1;
	}
}

bool equal_0_32xuint32(uint32 x[32])
{
	for (int i = 0; i < 32; i++)
	{
		if (x[i] != 0) return false;
	}
	return true;
}

bool can_be_divided_by2_32xuint32(uint32 x[32])
{
	if ((x[0] & 0x00000001) > 0) return true;
	else return false;
}

// rst = a - b (a > b)
void subtract_32xuint32(uint32 rst[32], uint32 a[32], uint32 b[32])
{
	uint64 aa;
	uint64 bb;
	uint64 temp = 0;

	for (int i = 0; i < 32; i++)
	{
		aa = a[i] - temp;
		bb = b[i];
		temp = 0;
		while (aa < bb)
		{
			temp += 1;
			aa += SCALE;
		}
		rst[i] = uint32(aa - bb);
	}
}

// rst = (c^e) mod n
void power_mod_32xuint32(uint32 rst[32], uint32 c[32], uint32 e[32], uint32 n[32])
{
	uint32 ee[32];
	uint32 cc[32];
	uint32 temp[32];

	for (int i = 0; i < 32; i++)
	{
		rst[i] = 0;
		ee[i] = e[i];
		cc[i] = c[i];
	}
	rst[0] = 1;

	while (!equal_0_32xuint32(ee))
	{
		if (can_be_divided_by2_32xuint32(ee))
		{
			multiply_32xuint32_modn(temp, cc, cc, n);
			for (int i = 0; i < 32; i++) cc[i] = temp[i];
		}
		else
		{
			multiply_32xuint32_modn(temp, rst, cc, n);
			for (int i = 0; i < 32; i++)
			{
				rst[i] = temp[i];
				temp[i] = 0;
			}
			temp[0] = 1;
			subtract_32xuint32(ee, ee, temp);
		}
	}
}