#pragma once
#include <genesis.h>

typedef unsigned char packedBools;

//Index must be 0-7
#define getPackedBool(bools, index)		((bool)((bools & (1 << index)) >> index))

#define clamp01(X)   (min(max((X), (0)), (1)))

//Index must be 0-7
packedBools setPackedBool(packedBools bools, u16 index, bool state);

typedef struct {
	u16 length;
	u16* array;
}DynamicArray_u16;
typedef struct {
	u16 length;
	u8* array;
}DynamicArray_u8;
typedef struct {
	u16 length;
	bool* array;
}DynamicArray_bool;

typedef struct {
	u8 x;
	u8 y;
}Vect2D_u8;

typedef struct {
	s8 x;
	s8 y;
}Vect2D_s8;

Vect2D_f16 newVector2D_f16(f16 x, f16 y);
Vect2D_f32 newVector2D_f32(f32 x, f32 y);

Vect2D_s8 newVector2D_s8(s8 x, s8 y);
Vect2D_s16 newVector2D_s16(s16 x, s16 y);
Vect2D_s32 newVector2D_s32(s32 x, s32 y);

Vect2D_u8 newVector2D_u8(u8 x, u8 y);
Vect2D_u16 newVector2D_u16(u16 x, u16 y);
Vect2D_u32 newVector2D_u32(u32 x, u32 y);

extern const u16 custom_palette_white[16];