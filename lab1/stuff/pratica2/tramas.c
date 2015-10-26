#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#include <termios.h>
#include <unistd.h>
#include <string.h>

#define FLAG 0b01111110 // F no inicio da trama
#define SET 0b00000111 // C se for uma trama de setup
#define DISC 0b00001011	// C se for uma trama de disconnect
#define UA 0b00000011 // C se for uma trama de unumbered acknowledgement

// C se for uma trama de positive acknowledgment
// nao tenho a certeza disto, nao sei se o R age desta forma ou de outra
int getRR(int R) {
	if (R == 1) return 0b00100001;
	else return 0b00000001;
}

// C se for uma trama de positive acknowledgment, R identifica a trama a que estamos a responder
// nao tenho a certeza disto, nao sei se o R age desta forma ou de outra
int getREJ(int R) {
	if (R == 1) return 0b00100101;
	else return 0b00000101;
}

// A depende do sentido da trama original
int getA(bool sender2receiver) {
	if (sender2receiver) return 0b00000011; // A se for o emissor a enviar a trama e o receptor a responder
	else return 0b00000001; // A se for o receptor a enviar a trama e o emissor a responder
}

int getC(int set_disc_ua_rr_rej, int R) {
	if (set_disc_ua_rr_rej == 1) return SET;
	else if (set_disc_ua_rr_rej == 2) return DISC;
	else if (set_disc_ua_rr_rej == 3) return UA;
	else if (set_disc_ua_rr_rej == 4) return getRR(R);
	else return getREJ(R);
}

// BCC1 codigo de verificacao, depende de A e de C: BCC1 = A ^ C (A ou exclusivo C)
int getBCC1(bool sender2receiver, int set_disc_ua_rr_rej, int R) {
	return getA(sender2receiver) ^ getC();
}

// F | A | C | BCC1 | F
int getTrama(bool sender2receiver, int set_disc_ua_rr_rej, int R) {
	return FLAG | getA(sender2receiver) | getC(set_disc_ua_rr_rej, R) | getBCC1(sender2receiver, set_disc_ua_rr_rej, R) | FLAG;
}
