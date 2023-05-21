#include "Common.h"

#include "Buf.h"

#include <iostream>

using namespace std;

Buf::Buf() {
	this->b_flags = b_flags_init;
	this->b_wcount = b_wcount_init;
	this->b_addr = new unsigned char[buffer_size];
	if (!this->b_addr) {
		cerr << "bufferÉêÇë¿Õ¼äÊ§°Ü£¡" << endl;
		exit(ERROR_NEW);
	}
	this->b_blkno = b_blkno_init;
	this->b_prev = NULL;
	this->b_next = NULL;
	memset(this->b_paddings, 0, sizeof(b_paddings));
}

Buf::Buf(const Buf& b) {
	this->b_flags = b.b_flags;
	this->b_wcount = b.b_wcount;
	this->b_addr = new unsigned char[buffer_size];
	if (!this->b_addr) {
		cerr << "bufferÉêÇë¿Õ¼äÊ§°Ü£¡" << endl;
		exit(ERROR_NEW);
	}
	memcpy(b_addr, b.b_addr, buffer_size);
	this->b_blkno = b.b_blkno;
	this->b_prev = b.b_prev;
	this->b_next = b.b_next;
	memset(this->b_paddings, 0, sizeof(b_paddings));
}

Buf::~Buf() {
	if (b_addr) {
		delete[] b_addr;
	}
}

bool Buf::checkDELWRI() {
	return this->b_flags & B_DELWRI;
}

bool Buf::checkDONE() {
	return this->b_flags & B_DONE;
}

void Buf::clearDELWRI() {
	this->b_flags &= (~B_DELWRI);
}

void Buf::clearDONE() {
	this->b_flags &= (~B_DONE);
}

void Buf::setDELWRI() {
	this->b_flags |= B_DELWRI;
}

void Buf::setDONE() {
	this->b_flags |= B_DONE;
}