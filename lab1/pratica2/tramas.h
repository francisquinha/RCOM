#ifndef TYPEDEF_BOOLEAN_DECLARED_
#define TYPEDEF_BOOLEAN_DECLARED_
typedef char bool; 
#endif /* TYPEDEF_BOOLEAN_DECLARED_*/

// C se for uma trama de positive acknowledgment
// nao tenho a certeza disto, nao sei se o R age desta forma ou de outra
int getRR(int R);

// C se for uma trama de positive acknowledgment, R identifica a trama a que estamos a responder
// nao tenho a certeza disto, nao sei se o R age desta forma ou de outra
int getREJ(int R);

// A depende do sentido da trama original
int getA(bool sender2receiver);

int getC(int set_disc_ua_rr_rej, int R);

// BCC1 codigo de verificacao, depende de A e de C: BCC1 = A ^ C (A ou exclusivo C)
int getBCC1(bool sender2receiver, int set_disc_ua_rr_rej, int R);

// F | A | C | BCC1 | F
int getTrama(bool sender2receiver, int set_disc_ua_rr_rej, int R);