#include "../inc/types.h"

//Index must be 0-7
packedBools setPackedBool(packedBools bools, u16 index, bool state) {
	if (state)
		bools |= (state << index);
	else
		bools &= ~(1 << index);
	return bools;
}

Vect2D_f16 newVector2D_f16(f16 x, f16 y) {
	return (Vect2D_f16) { x, y };
}
Vect2D_f32 newVector2D_f32(f32 x, f32 y) {
	return (Vect2D_f32) { x, y };
}

Vect2D_s8 newVector2D_s8(s8 x, s8 y) {
	return (Vect2D_s8) { x, y };
}
Vect2D_s16 newVector2D_s16(s16 x, s16 y) {
	return (Vect2D_s16) { x, y };
}
Vect2D_s32 newVector2D_s32(s32 x, s32 y) {
	return (Vect2D_s32) { x, y };
}

Vect2D_u8 newVector2D_u8(u8 x, u8 y) {
	return (Vect2D_u8) { x, y };
}
Vect2D_u16 newVector2D_u16(u16 x, u16 y) {
	return (Vect2D_u16) { x, y };
}
Vect2D_u32 newVector2D_u32(u32 x, u32 y) {
	return (Vect2D_u32) { x, y };
}

const u16 custom_palette_white[16] = {
	0x0000,
	0x0EEE,
	0x0EEE,
	0x0EEE,
	0x0EEE,
	0x0EEE,
	0x0EEE,
	0x0EEE,

	0x0EEE,
	0x0EEE,
	0x0EEE,
	0x0EEE,
	0x0EEE,
	0x0EEE,
	0x0EEE,
	0x0EEE
};