#include <genesis.h>

int main() {
	VDP_drawText("Hello, World!", 5, 8);

	while (TRUE) {
		SYS_doVBlankProcess();
	}
	
	return 0;
}